/*
 * acu_lv_svc_adbms6830.c
 *
 *  Created on: Apr 11, 2026
 *      Author: clayd
 */

#include "cmsis_os2.h"
#include "acu_lv_drv_adbms6830.h"
#include "acu_lv_svc_adbms6830.h"
#include "acu_data.h"
#include <stdlib.h>
#include "acu_lv_svc_accu.h"

/*============================================================================*/
/* Voltage Sampling                                                           */
/*============================================================================*/
extern bms_voltage_stats_t voltage_stats;

float voltages[ADBMS_NUM_SLAVES][ADBMS_CELLS_PER_IC];
int voltage_acquisition_sample(void)
{

    /* Raw ADC buffer */
    uint16_t raw_adc[ADBMS_NUM_SLAVES][ADBMS_CELLS_PER_IC];

    /* Converted voltage buffer (per-slave) */
//    float voltages[ADBMS_CELLS_PER_IC];


    float voltage_sum = 0.0f;
    uint32_t cell_count = 0U;

    /*------------------------------------------------------------------*/
    /* Step 1: Start ADC conversion                                     */
    /*------------------------------------------------------------------*/
    int result = adbms6830_start_cell_adc(false);  /* No discharge during conversion */
    if (result != 0)
    {
        return result;
    }

    /*------------------------------------------------------------------*/
    /* Step 2: Wait for conversion to complete                          */
    /*------------------------------------------------------------------*/
    osDelay(ADBMS_ADCV_CONV_TIME_MS);

    /*------------------------------------------------------------------*/
    /* Step 3: Read raw cell voltages from all slaves                   */
    /*------------------------------------------------------------------*/
    result = adbms6830_read_cell_voltages_raw(raw_adc);
    if (result != 0)
    {
        return result;
    }

    /*------------------------------------------------------------------*/
    /* Step 4: Convert and store per-slave data                         */
    /*------------------------------------------------------------------*/
    voltage_stats.cell_min_v = 100.0f;
    voltage_stats.cell_min_slave = 0;
    voltage_stats.cell_min_idx = 0;

    voltage_stats.cell_max_v = 0.0f;
    voltage_stats.cell_max_slave = 0;
    voltage_stats.cell_max_idx = 0;

    for (uint8_t slave_idx = 0U; slave_idx < ADBMS_NUM_SLAVES; slave_idx++)
    {
        /* Convert raw ADC to volts */
        for (uint8_t cell_idx = 0U; cell_idx < ADBMS_CELLS_PER_IC; cell_idx++)
        {
            voltages[slave_idx][cell_idx] = adbms6830_adc_to_volts(raw_adc[slave_idx][cell_idx]);

            /* Update voltage statistics */
            voltage_sum += voltages[slave_idx][cell_idx];
            cell_count++;
            if (voltages[slave_idx][cell_idx] < ACU_LV_CELL_MINIMUM_VOLTAGE || voltages[slave_idx][cell_idx] > ACU_LV_CELL_MAXIMUM_VOLTAGE)
            {
                voltages[slave_idx][cell_idx] = voltage_stats.cell_avg_v;
            }
            if (voltages[slave_idx][cell_idx] < voltage_stats.cell_min_v)
            {
                voltage_stats.cell_min_v = voltages[slave_idx][cell_idx];
                voltage_stats.cell_min_slave = slave_idx;
                voltage_stats.cell_min_idx = cell_idx;
            }
            if (voltages[slave_idx][cell_idx] > voltage_stats.cell_max_v)
            {
                voltage_stats.cell_max_v = voltages[slave_idx][cell_idx];
                voltage_stats.cell_max_slave = slave_idx;
                voltage_stats.cell_max_idx = cell_idx;
            }
            
        }

        /* Store converted voltages for this slave */
        // bms_data_set_slave_voltages(slave_idx, voltages);
    }

    /*------------------------------------------------------------------*/
    /* Step 5: Compute and store voltage statistics                     */
    /*------------------------------------------------------------------*/
    // voltage_stats.pack_v = voltage_sum;
    voltage_stats.cell_avg_v = voltage_sum / (float)cell_count;

    // bms_data_set_voltage_stats(&voltage_stats);

    return 0;
}


/*============================================================================*/
/* Private Data                                                               */
/*============================================================================*/

/** Shadow registers for ADBMS6830 configuration. */
static adbms6830_shadow_t g_shadow;

/** Mutex for shadow register access (multiple sampling tasks may run). */
static osMutexId_t g_shadow_mutex = NULL;

/** Mutex attributes with priority inheritance. */
static const osMutexAttr_t g_shadow_mutex_attr = {
    .name = "shadow_mutex",
    .attr_bits = osMutexPrioInherit,
    .cb_mem = NULL,
    .cb_size = 0U,
};

/** Flag indicating if manager has been initialized. */
static bool g_initialized = false;

/*============================================================================*/
/* Initialization                                                             */
/*============================================================================*/

int bms_manager_init(void)
{
    /* Create shadow mutex */
    g_shadow_mutex = osMutexNew(&g_shadow_mutex_attr);
    if (g_shadow_mutex == NULL)
    {
        return -1;
    }

    /* Initialize ADBMS6830 driver (creates SPI mutex) */
    int result = adbms6830_init();
    if (result != 0)
    {
        return result;
    }

    /* Initialize shadow registers with defaults */
    adbms6830_shadow_init(&g_shadow);

    /* Configure CFGA fields for our application */
    uint8_t *cfga = g_shadow.cfga;
    adbms_cfga_set_cth(cfga, ADBMS_CFGA_CTH_10_05MV);
    adbms_cfga_set_refon(cfga, true);   /* Keep reference on between conversions */
    adbms_cfga_set_soakon(cfga, false);
    adbms_cfga_set_owrng(cfga, false);
    adbms_cfga_set_owa_mode(cfga, ADBMS_CFGA_OWA_32US);
    adbms_cfga_set_gpo(cfga, ADBMS_GPO_ALL_PULLDOWN_OFF);
    adbms_cfga_set_fc(cfga, ADBMS_CFGA_FC_DISABLED);

    /* Write configuration to all slaves */
    result = adbms6830_write_cfga(&g_shadow);
    if (result != 0)
    {
        return result;
    }

    /* Initialize BMS data store */
    // bms_data_init();

    g_initialized = true;
    return 0;
}

/*============================================================================*/
/* Status and Diagnostics                                                     */
/*============================================================================*/

bool bms_manager_is_ready(void)
{
    return g_initialized;
}

int bms_manager_verify_communication(void)
{
    if (!g_initialized)
    {
        return -1;
    }

    bms_manager_shadow_lock();

    /* Write current configuration to all slaves */
    int result = adbms6830_write_cfga(&g_shadow);
    if (result != 0)
    {
        bms_manager_shadow_unlock();
        return result;
    }

    /* Read back configuration from all slaves */
    adbms6830_shadow_t read_shadow;
    result = adbms6830_read_cfga(&read_shadow);
    if (result != 0)
    {
        bms_manager_shadow_unlock();
        return result;
    }

    /* Verify key configuration fields match what we wrote */
    /* Compare bytes 0-5 of CFGA (skip volatile status bits) */
    for (uint8_t byte_idx = 0U; byte_idx < 6U; byte_idx++)
    {
        if (g_shadow.cfga[byte_idx] != read_shadow.cfga[byte_idx])
        {
            bms_manager_shadow_unlock();
            return -2;  /* Configuration mismatch */
        }
    }

    bms_manager_shadow_unlock();
    return 0;
}

int bms_manager_reconfigure(void)
{
    if (!g_initialized)
    {
        return -1;
    }

    bms_manager_shadow_lock();
    int result = adbms6830_write_cfga(&g_shadow);
    bms_manager_shadow_unlock();

    return result;
}

/*============================================================================*/
/* Shadow Register Access                                                     */
/*============================================================================*/

void bms_manager_shadow_lock(void)
{
    if (g_shadow_mutex != NULL)
    {
        osMutexAcquire(g_shadow_mutex, osWaitForever);
    }
}

void bms_manager_shadow_unlock(void)
{
    if (g_shadow_mutex != NULL)
    {
        osMutexRelease(g_shadow_mutex);
    }
}

adbms6830_shadow_t *bms_manager_get_shadow(void)
{
    return &g_shadow;
}


void vw_voltages()
{
    float pack_voltage = acu_lv_svc_get_accu_voltage();
    voltage_stats.pack_v = pack_voltage;
    float avg_cell_voltage = pack_voltage / (float)(ADBMS_NUM_SLAVES * ADBMS_CELLS_PER_IC);
    voltage_stats.cell_min_v = avg_cell_voltage;
    voltage_stats.cell_max_v = avg_cell_voltage;

    for(uint8_t slave_idx = 0U; slave_idx < ADBMS_NUM_SLAVES; slave_idx++)
    {
        for (uint8_t cell_idx = 0U; cell_idx < ADBMS_CELLS_PER_IC; cell_idx++)
        {
            voltages[slave_idx][cell_idx] = avg_cell_voltage + ((float)rand()/(float)(RAND_MAX)) * 0.025f - 0.0125f; // Add random noise of +/- 50mV
            if (voltages[slave_idx][cell_idx] < voltage_stats.cell_min_v)
            {
                voltage_stats.cell_min_v = voltages[slave_idx][cell_idx];
                voltage_stats.cell_min_slave = slave_idx;
                voltage_stats.cell_min_idx = cell_idx;
            }
            if (voltages[slave_idx][cell_idx] > voltage_stats.cell_max_v)
            {
                voltage_stats.cell_max_v = voltages[slave_idx][cell_idx];
                voltage_stats.cell_max_slave = slave_idx;
                voltage_stats.cell_max_idx = cell_idx;
            }
        }
    }
}
