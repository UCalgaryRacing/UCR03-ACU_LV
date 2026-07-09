#include "acu_data.h"
#include <string.h>
#include "acu_lv_config.h"

static bool g_initialized;

static acu_aculv_data_t g_aculv_data;
static acu_fault_t g_aculv_fault;

/*============================================================================*/
/* Initialization                                                             */
/*============================================================================*/
//TODO configure default values as invaldi
status_t acu_data_init(void)
{
    memset(&g_aculv_data, 0, sizeof(g_aculv_data));
    g_aculv_data.battery_current = 0.0f;
    g_aculv_data.battery_power = 0.0f;
    g_aculv_data.battery_voltage = 0.0f;
    g_aculv_data.ts_voltage = 0.0f;
    g_aculv_data.ts_active = false;

    memset(&g_aculv_fault, 0, sizeof(g_aculv_fault));
    g_aculv_fault.bms_fault = false;
    g_aculv_fault.imd_fault = false;

    g_initialized = true;

    return OK;
}

bms_voltage_stats_t voltage_stats = 
{
    .cell_min_v     = ACU_LV_CELL_MAXIMUM_VOLTAGE,  
    .cell_max_v     = ACU_LV_CELL_MINIMUM_VOLTAGE,   
    .cell_avg_v     = 3.6f,
    .pack_v         = 0.0f,
    .cell_min_slave = 0U,
    .cell_min_idx   = 0U,
    .cell_max_slave = 0U,
    .cell_max_idx   = 0U,
};

bms_temp_stats_t temp_stats =
{
    .temp_min_c = ACULV_CELL_MAX_TEMPERATURE, 
    .temp_max_c = ACULV_CELL_MAX_TEMPERATURE,    
    .temp_avg_c = ADBMS_AVERGAE_THERM_VOLTAGE,
    .temp_min_slave = 0,
    .temp_min_idx = 0,
    .temp_max_slave = 0,
    .temp_max_idx = 0,
};

/*============================================================================*/
/* ACULV Data Access                                                          */
/*============================================================================*/

//TODO: add mutex protection for these data accesses
status_t acu_data_set_aculv(const acu_aculv_data_t *aculv_data)
{
    if (aculv_data == NULL)
    {
        return ERROR_INVALID_PARAM;
    }

    g_aculv_data = *aculv_data;

    return OK;
}
void acu_data_set_aculv_battery_current(float current)
{
    g_aculv_data.battery_current = current;
}

void acu_data_set_aculv_battery_power(float power)
{
    g_aculv_data.battery_power = power;
}

void acu_data_set_aculv_battery_voltage(float voltage)
{
    g_aculv_data.battery_voltage = voltage;
}

void acu_data_set_aculv_ts_active(bool ts_active)
{
    g_aculv_data.ts_active = ts_active;
}

void acu_data_set_aculv_ts_voltage(float ts_voltage)
{
    g_aculv_data.ts_voltage = ts_voltage;
}

float acu_data_get_aculv_battery_voltage(void)
{
    return g_aculv_data.battery_voltage;
}

float acu_data_get_aculv_battery_current(void)
{
    return g_aculv_data.battery_current;
}

float acu_data_get_aculv_battery_power(void)
{
    return g_aculv_data.battery_power;
}

float acu_data_get_aculv_ts_voltage(void)
{
    return g_aculv_data.ts_voltage;
}

bool acu_data_get_aculv_ts_active(void)
{
    return g_aculv_data.ts_active;
}

/*============================================================================*/
/* ACULV Fault Status Access                                                  */
/*============================================================================*/
status_t acu_data_set_fault_status(const acu_fault_t *acu_fault_status)
{
    if(acu_fault_status == NULL)
    {
        return ERROR_INVALID_PARAM;
    }

    g_aculv_fault = *acu_fault_status;

    return OK;
}

status_t acu_data_get_fault_status(acu_fault_t *acu_fault_status)
{
	*acu_fault_status = g_aculv_fault;
	return OK;
}

void acu_data_set_bms_fault_status(bool fault_status)
{
    g_aculv_fault.bms_fault = fault_status;
}

void acu_data_set_imd_fault_status(bool fault_status)
{
    g_aculv_fault.imd_fault = fault_status;
}

bool acu_data_get_bms_fault_status(void)
{
    return g_aculv_fault.bms_fault;
}

bool acu_data_get_imd_fault_status(void)
{
    return g_aculv_fault.imd_fault;
}
