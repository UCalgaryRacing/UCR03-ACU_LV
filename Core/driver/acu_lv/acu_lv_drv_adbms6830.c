/*
 * adbms6830.c
 *
 * ADBMS6830B Battery Stack Monitor Driver Implementation.
 *
 *  Created on: Nov 12, 2025
 *      Author: esall
 */

#include "acu_lv_drv_adbms6830.h"
#include "acu_lv_config.h"
#include "acu_lv_config_pinout.h"
#include "cmsis_os2.h"
#include <string.h>

/*============================================================================*/
/* SPI Bus Mutex                                                              */
/*============================================================================*/

/** Mutex for exclusive SPI bus access. */
static osMutexId_t g_spi_mutex;

/** Mutex attributes with priority inheritance to prevent inversion. */
static const osMutexAttr_t g_spi_mutex_attr =
{
    .name = "adbms_spi_mutex",
    .attr_bits = osMutexPrioInherit,  /* Prevent priority inversion */
    .cb_mem = NULL,
    .cb_size = 0U,
};

/** Flag indicating if driver has been initialized. */
static bool g_driver_initialized = false;

/**
 * @brief Acquire SPI bus mutex.
 */
static inline void spi_lock(void)
{
    osMutexAcquire(g_spi_mutex, osWaitForever);
}

/**
 * @brief Release SPI bus mutex.
 */
static inline void spi_unlock(void)
{
    osMutexRelease(g_spi_mutex);
}

/*============================================================================*/
/* Local helpers                                                              */
/*============================================================================*/

/*----------------------------------------------------------------------------*/
/* Low-level SPI helpers                                                      */
/*----------------------------------------------------------------------------*/

/** Maximum combined frame size: 4-byte command + 8-byte data (per slave). */
#define ADBMS_MAX_FRAME_SIZE    (4U + ADBMS_REG_GROUP_SIZE + 2U)

/** Maximum read transaction size: 4-byte command + data from all slaves. */
#define ADBMS_MAX_READ_SIZE     (4U + (ADBMS_NUM_SLAVES * (ADBMS_REG_GROUP_SIZE + 2U)))

/**
 * @brief Send a command-only frame (no data payload).
 *
 * @param cmd_frame  4-byte command frame (cmd[2] + PEC[2]).
 * @return HAL_OK on success, HAL error code on failure.
 */
static HAL_StatusTypeDef adbms_spi_send_cmd(const uint8_t cmd_frame[4])
{
    uint8_t temp_buffer[4];
    for(uint8_t i = 0; i < 4; i++)
    {
        temp_buffer[i] = cmd_frame[i];
    }
    HAL_GPIO_WritePin(ADBMS_1_CSN_PORT, ADBMS_1_CSN_PIN, GPIO_PIN_RESET);
    HAL_StatusTypeDef hal_status = HAL_SPI_Transmit(ADBMS_1_SPI_HANDLE, temp_buffer, 4U, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(ADBMS_1_CSN_PORT, ADBMS_1_CSN_PIN, GPIO_PIN_SET);
    return hal_status;
}

/**
 * @brief Send command and read response data using full-duplex transceive.
 *
 * Uses HAL_SPI_TransmitReceive for continuous clocking with no gaps.
 * TX buffer contains command + zeros (dummy bytes during receive phase).
 * RX buffer receives garbage during command phase, then actual data.
 *
 * @param cmd_frame  4-byte command frame.
 * @param rx_data    Buffer for received data (only the data portion, not command echo).
 * @param rx_len     Number of data bytes to receive.
 * @return HAL_OK on success, HAL error code on failure.
 */
static HAL_StatusTypeDef adbms_spi_read(const uint8_t cmd_frame[4], uint8_t *rx_data, uint16_t rx_len)
{
    uint8_t tx_buf[ADBMS_MAX_READ_SIZE];
    uint8_t rx_buf[ADBMS_MAX_READ_SIZE];
    uint16_t total_len = 4U + rx_len;

    /* Build TX buffer: command + zeros for receive clocking */
    memcpy(tx_buf, cmd_frame, 4U);
    memset(&tx_buf[4], 0x00, rx_len);

    HAL_GPIO_WritePin(ADBMS_1_CSN_PORT, ADBMS_1_CSN_PIN, GPIO_PIN_RESET);
    HAL_StatusTypeDef hal_status = HAL_SPI_TransmitReceive(ADBMS_1_SPI_HANDLE, tx_buf, rx_buf, total_len, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(ADBMS_1_CSN_PORT, ADBMS_1_CSN_PIN, GPIO_PIN_SET);

    /* Copy received data (skip first 4 bytes which are garbage during TX) */
    if (hal_status == HAL_OK)
    {
        memcpy(rx_data, &rx_buf[4], rx_len);
    }

    return hal_status;
}

/**
 * @brief Send command and write data in a single SPI transaction.
 *
 * Combines command and data into one buffer for a single HAL_SPI_Transmit call,
 * ensuring no timing gaps between command and data bytes.
 *
 * @param cmd_frame  4-byte command frame.
 * @param data       Data bytes to write.
 * @param data_len   Number of data bytes (typically 8: 6 payload + 2 PEC).
 * @return HAL_OK on success, HAL error code on failure.
 */
static HAL_StatusTypeDef adbms_spi_write(const uint8_t cmd_frame[4], const uint8_t *data, uint16_t data_len)
{
    /* Combine command and data into single buffer */
    uint8_t tx_buf[ADBMS_MAX_FRAME_SIZE];
    memcpy(tx_buf, cmd_frame, 4U);
    memcpy(&tx_buf[4], data, data_len);

    HAL_GPIO_WritePin(ADBMS_1_CSN_PORT, ADBMS_1_CSN_PIN, GPIO_PIN_RESET);
    osDelay(1);
    HAL_GPIO_WritePin(ADBMS_1_CSN_PORT, ADBMS_1_CSN_PIN, GPIO_PIN_SET);
    osDelay(1);

    HAL_GPIO_WritePin(ADBMS_1_CSN_PORT, ADBMS_1_CSN_PIN, GPIO_PIN_RESET);
    HAL_StatusTypeDef hal_status = HAL_SPI_Transmit(ADBMS_1_SPI_HANDLE,tx_buf, 4U + data_len, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(ADBMS_1_CSN_PORT, ADBMS_1_CSN_PIN, GPIO_PIN_SET);
    return hal_status;
}

/*----------------------------------------------------------------------------*/
/* PEC calculation                                                            */
/*----------------------------------------------------------------------------*/

/**
 * @brief Calculate PEC15 over command frame bytes.
 *
 * Polynomial: 0x4599, initial remainder 0x0010, as specified for ADBMS/LTC
 * battery monitor devices (see datasheet "Command PEC").
 *
 * @param data Pointer to input bytes.
 * @param len  Number of bytes.
 * @return 15-bit PEC value left-aligned in a 16-bit word.
 */
static uint16_t adbms6830_pec15_calc(const uint8_t *data, uint8_t len)
{
    uint16_t remainder = 16U; /* 0x0010 */

    for (uint8_t byte_idx = 0U; byte_idx < len; byte_idx++)
    {
        remainder ^= (uint16_t)data[byte_idx] << 7;
        for (uint8_t bit_idx = 0U; bit_idx < 8U; bit_idx++)
        {
            if (remainder & 0x4000U)
            {
                remainder = (uint16_t)((remainder << 1) ^ 0x4599U);
            }
            else
            {
                remainder <<= 1;
            }
        }
    }

    return (uint16_t)(remainder << 1);
}











/**
 * @brief Calculate PEC10 over data bytes and 6-bit command counter.
 *
 * The ADBMS6830B data PEC is always computed over the 6 register bytes
 * followed by 6 command-counter bits.  For write commands the counter
 * value is 0; for read-back verification it is the CCNT returned by
 * the device.
 *
 * @param data        Pointer to register data bytes.
 * @param len         Number of data bytes (typically 6).
 * @param cmd_counter 6-bit command counter (0 for write PEC).
 * @return 10-bit PEC value in bits [9:0].
 */
static uint16_t adbms6830_pec10_calc(const uint8_t *data, uint8_t len,
                                     uint8_t cmd_counter)
{
    uint16_t remainder = 16U; /* 0x0010 */

    for (uint8_t byte_idx = 0U; byte_idx < len; byte_idx++)
    {
        remainder ^= (uint16_t)data[byte_idx] << 2;
        for (uint8_t bit_idx = 0U; bit_idx < 8U; bit_idx++)
        {
            if (remainder & 0x0200U)
            {
                remainder = (uint16_t)((remainder << 1) ^ 0x008FU);
            }
            else
            {
                remainder <<= 1;
            }
        }
        remainder &= 0x03FFU;
    }

    /* XOR the 6-bit command counter into bits [9:4] of the remainder,
     * then clock 6 more bits through the CRC register. */
    remainder ^= (uint16_t)((uint16_t)(cmd_counter & 0x3FU) << 4);
    for (uint8_t bit_idx = 0U; bit_idx < 6U; bit_idx++)
    {
        if (remainder & 0x0200U)
        {
            remainder = (uint16_t)((remainder << 1) ^ 0x008FU);
        }
        else
        {
            remainder <<= 1;
        }
    }

    return remainder & 0x03FFU;
}











/*============================================================================*/
/* Driver Initialization                                                      */
/*============================================================================*/

int adbms6830_init(void)
{
    if (g_driver_initialized)
    {
        return 0;  /* Already initialized */
    }

    /* Create SPI bus mutex */
    g_spi_mutex = osMutexNew(&g_spi_mutex_attr);
    if (g_spi_mutex == NULL)
    {
        return -1;  /* Mutex creation failed */
    }

    g_driver_initialized = true;
    return 0;
}

/*============================================================================*/
/* Wake-Up and Power Management                                               */
/*============================================================================*/

void adbms6830_wakeup_pulse(void)
{
    /* Toggle CS low then high - minimum tDWELL (240ns) is satisfied
     * by GPIO switching time on most MCUs */
    HAL_GPIO_WritePin(ADBMS_1_CSN_PORT, ADBMS_1_CSN_PIN, GPIO_PIN_RESET);
    /* Brief delay - GPIO switching + instruction time exceeds 240ns */
    osDelay(1U);
    // __NOP(); __NOP(); __NOP(); __NOP(); // need to measure wakeup pulse. probably different for H7
    HAL_GPIO_WritePin(ADBMS_1_CSN_PORT, ADBMS_1_CSN_PIN, GPIO_PIN_SET);
}

int adbms6830_wakeup(void)
{

    /* Send one wake-up pulse per device in the daisy chain.
     * Each pulse wakes a device and allows it to propagate the next pulse.
     * Wait 1ms between pulses (> tREADY/tWAKE, < tIDLE). */
    for (uint8_t device_idx = 0U; device_idx < (ADBMS_NUM_SLAVES); device_idx++)
    {
        adbms6830_wakeup_pulse();

        /* Wait between pulses - use osDelay for millisecond delay */
        osDelay(1U);
    }

    return 0;
}

/*============================================================================*/
/* Shadow Register Management                                                 */
/*============================================================================*/

void adbms6830_shadow_init(adbms6830_shadow_t *shadow)
{
    if (shadow == NULL)
    {
        return;
    }

    /* Start with datasheet defaults */
    adbms_cfga_set_defaults(shadow->cfga);

    /* Apply application-specific configuration from adbms6830_config.h */
    adbms_cfga_set_cth(shadow->cfga, ADBMS_DEFAULT_CTH);
    adbms_cfga_set_fc(shadow->cfga, ADBMS_DEFAULT_FC);
    adbms_cfga_set_owa_mode(shadow->cfga, ADBMS_DEFAULT_OWA);
    adbms_cfga_set_gpo(shadow->cfga, ADBMS_DEFAULT_GPO);
}

void adbms6830_shadow_set_cfga(adbms6830_shadow_t *shadow,
                               const uint8_t data[ADBMS_REG_GROUP_SIZE])
{
    if (shadow == NULL || data == NULL)
    {
        return;
    }
    memcpy(shadow->cfga, data, ADBMS_REG_GROUP_SIZE);
}

const uint8_t *adbms6830_shadow_get_cfga(const adbms6830_shadow_t *shadow)
{
    if (shadow == NULL)
    {
        return NULL;
    }
    return shadow->cfga;
}

int adbms6830_write_cfga(const adbms6830_shadow_t *shadow)
{
    if (shadow == NULL)
    {
        return -1;
    }

    const uint8_t *cfga = shadow->cfga;

    /* Command frame: 2 command bytes + 2 PEC bytes */
    uint8_t cmd_buf[4];
    cmd_buf[0] = (uint8_t)(CMD_WRCFGA >> 8);
    cmd_buf[1] = (uint8_t)CMD_WRCFGA;

    uint16_t cmd_pec = adbms6830_pec15_calc(cmd_buf, 2U);
    cmd_buf[2] = (uint8_t)(cmd_pec >> 8);
    cmd_buf[3] = (uint8_t)cmd_pec;

    /* Data frame: 6 CFGA bytes + 2 PEC bytes (PEC10 for data)
     * Per Table 42 (Write Data PEC Format):
     *   PEC0: bits [7:2]=0, bits [1:0]=PEC[9:8]
     *   PEC1: bits [7:0]=PEC[7:0]
     */
    uint8_t data_buf[ADBMS_REG_GROUP_SIZE + 2U];
    memcpy(data_buf, cfga, ADBMS_REG_GROUP_SIZE);

    uint16_t data_pec = adbms6830_pec10_calc(data_buf, ADBMS_REG_GROUP_SIZE, 0U);
    data_buf[6] = (uint8_t)((data_pec >> 8) & 0x03U);  /* PEC[9:8] in bits [1:0] */
    data_buf[7] = (uint8_t)data_pec;                   /* PEC[7:0] */

    HAL_StatusTypeDef hal_status = adbms_spi_write(cmd_buf, data_buf, sizeof(data_buf));

    return (hal_status == HAL_OK) ? 0 : -1;
}

int adbms6830_read_cfga(adbms6830_shadow_t *shadow)
{
    if (shadow == NULL)
    {
        return -1;
    }

    /* Command frame: 2 command bytes + 2 PEC bytes */
    uint8_t cmd_buf[4];
    cmd_buf[0] = (uint8_t)(CMD_RDCFGA >> 8);
    cmd_buf[1] = (uint8_t)CMD_RDCFGA;

    uint16_t cmd_pec = adbms6830_pec15_calc(cmd_buf, 2U);
    cmd_buf[2] = (uint8_t)(cmd_pec >> 8);
    cmd_buf[3] = (uint8_t)cmd_pec;

    /* Read 6 data bytes + 2 PEC bytes
     * Per Table 43 (Read Data PEC Format):
     *   PEC0: bits [7:2]=CCNT[5:0] (command counter), bits [1:0]=PEC[9:8]
     *   PEC1: bits [7:0]=PEC[7:0]
     * The data PEC is calculated over the 6 data bytes + command counter.
     */
    uint8_t rx_buf[ADBMS_REG_GROUP_SIZE + 2U] = {0};

    HAL_StatusTypeDef hal_status = adbms_spi_read(cmd_buf, rx_buf, sizeof(rx_buf));

    if (hal_status != HAL_OK)
    {
        return -1;
    }

    /* Extract received PEC[9:0] from PEC0/PEC1 bytes */
    uint16_t received_pec = (uint16_t)(((uint16_t)(rx_buf[6] & 0x03U) << 8) | rx_buf[7]);

    /* Extract command counter from PEC0[7:2] */
    uint8_t cmd_counter = (uint8_t)((rx_buf[6] >> 2) & 0x3FU);

    uint16_t calculated_pec = adbms6830_pec10_calc(rx_buf, ADBMS_REG_GROUP_SIZE, cmd_counter);

    if (received_pec != calculated_pec)
    {
        return -2;
    }

    memcpy(shadow->cfga, rx_buf, ADBMS_REG_GROUP_SIZE);

    return 0;
}

/*============================================================================*/
/* Cell Voltage Measurement                                                   */
/*============================================================================*/

/**
 * @brief Send a command-only frame (no data payload).
 *
 * @param cmd  16-bit command code.
 * @return 0 on success, -1 on SPI error.
 */
static int adbms6830_send_command(uint16_t cmd)
{
    uint8_t cmd_buf[4];
    cmd_buf[0] = (uint8_t)(cmd >> 8);
    cmd_buf[1] = (uint8_t)cmd;

    adbms6830_wakeup();

    uint16_t cmd_pec = adbms6830_pec15_calc(cmd_buf, 2U);
    cmd_buf[2] = (uint8_t)(cmd_pec >> 8);
    cmd_buf[3] = (uint8_t)cmd_pec;

    HAL_StatusTypeDef hal_status = adbms_spi_send_cmd(cmd_buf);
    return (hal_status == HAL_OK) ? 0 : -1;
}

/**
 * @brief Read a register group from all slaves in the daisy chain.
 *
 * @param cmd       16-bit read command code.
 * @param rx_data   Output buffer [ADBMS_NUM_SLAVES][ADBMS_REG_GROUP_SIZE].
 * @return 0 on success, -1 on SPI error, -2 on PEC error.
 */
static int adbms6830_read_register_group(uint16_t cmd, uint8_t rx_data[ADBMS_NUM_SLAVES][ADBMS_REG_GROUP_SIZE])
{
    /* Command frame */
    uint8_t cmd_buf[4];
    cmd_buf[0] = (uint8_t)(cmd >> 8);
    cmd_buf[1] = (uint8_t)cmd;

    uint16_t cmd_pec = adbms6830_pec15_calc(cmd_buf, 2U);
    cmd_buf[2] = (uint8_t)(cmd_pec >> 8);
    cmd_buf[3] = (uint8_t)cmd_pec;

    /* Each slave returns 6 data bytes + 2 PEC bytes */
    const uint8_t bytes_per_slave = ADBMS_REG_GROUP_SIZE + 2U;
    uint8_t rx_buf[ADBMS_NUM_SLAVES * (ADBMS_REG_GROUP_SIZE + 2U)];

    HAL_StatusTypeDef hal_status = adbms_spi_read(cmd_buf, rx_buf, sizeof(rx_buf));
    if (hal_status != HAL_OK)
    {
        return -1;
    }

    /* Validate PEC for each slave and extract data */
    for (uint8_t slave_idx = 0U; slave_idx < ADBMS_NUM_SLAVES; slave_idx++)
    {
        uint8_t *slave_data = &rx_buf[slave_idx * bytes_per_slave];

        /* Extract PEC from received data */
        uint16_t received_pec = (uint16_t)(((uint16_t)(slave_data[6] & 0x03U) << 8) | slave_data[7]);
        uint8_t cmd_counter = (uint8_t)((slave_data[6] >> 2) & 0x3FU);

        uint16_t calculated_pec = adbms6830_pec10_calc(slave_data, ADBMS_REG_GROUP_SIZE, cmd_counter);

        // if (received_pec != calculated_pec)
        // {
        //     return -2; /* PEC error on slave */
        // }

        /* Copy validated data */
        memcpy(rx_data[slave_idx], slave_data, ADBMS_REG_GROUP_SIZE);
    }

    return 0;
}

int adbms6830_start_cell_adc(bool discharge_permitted)
{
    uint16_t cmd = CMD_ADCV_BASE;
    if (discharge_permitted)
    {
        cmd |= CMD_ADCV_DCP;
    }

    int result = adbms6830_send_command(cmd);

    return result;
}

static uint16_t avg_voltage_raw = 1000;

int adbms6830_read_cell_voltages_raw(uint16_t raw_adc[ADBMS_NUM_SLAVES][ADBMS_CELLS_PER_IC])
{
    /* Cell voltage register group commands (each group contains 3 cells) */
    static const uint16_t cell_voltage_cmds[6] = 
    {
        CMD_RDCVA, CMD_RDCVB, CMD_RDCVC, CMD_RDCVD, CMD_RDCVE, CMD_RDCVF
    };

    uint8_t reg_data[ADBMS_NUM_SLAVES][ADBMS_REG_GROUP_SIZE];
    int result;


    /* Read all 6 cell voltage register groups */
    for (uint8_t reg_group = 0U; reg_group < 6U; reg_group++)
    {
        result = adbms6830_read_register_group(cell_voltage_cmds[reg_group], reg_data);
        if (result != 0)
        {
            return result;
        }

        /* Extract 3 cell voltages per group (16-bit little-endian) */
        for (uint8_t slave_idx = 0U; slave_idx < ADBMS_NUM_SLAVES; slave_idx++)
        {
            uint8_t first_cell_in_group = reg_group * 3U;

            /* Each register group contains 3 cells (bytes 0-5) */
            for (uint8_t cell_in_group = 0U; cell_in_group < 3U; cell_in_group++)
            {
                uint8_t cell_idx = first_cell_in_group + cell_in_group;
                if (cell_idx < ADBMS_CELLS_PER_IC)
                {
                    uint8_t byte_offset = cell_in_group * 2U;
                    raw_adc[slave_idx][cell_idx] = (uint16_t)reg_data[slave_idx][byte_offset] |
                                                   ((uint16_t)reg_data[slave_idx][byte_offset + 1U] << 8);
                }
            }
        }
    }

    return 0;
}

/*============================================================================*/
/* GPIO (Auxiliary) Measurement                                               */
/*============================================================================*/

int adbms6830_start_gpio_adc(void)
{
    int result = adbms6830_send_command(CMD_ADAX_BASE);

    return result;
}
static uint16_t avg_temp_raw = 300;

int adbms6830_read_gpio_voltages_raw(uint16_t raw_adc[ADBMS_NUM_SLAVES][ADBMS_THERMS_PER_IC],uint8_t mux_state)
{
    /* Auxiliary register group commands:
     * RDAUXA: GPIO1, GPIO2, GPIO3
     * RDAUXB: GPIO4, GPIO5, GPIO6
     * RDAUXC: GPIO7, GPIO8, GPIO9
     * RDAUXD: GPIO10, VM, VP (we only need GPIO10)
     */
    static const uint16_t gpio_cmds[4] = {
        CMD_RDAUXA, CMD_RDAUXB, CMD_RDAUXC, CMD_RDAUXD
    };

    uint8_t reg_data[ADBMS_NUM_SLAVES][ADBMS_REG_GROUP_SIZE];
    int result;

    uint8_t mux_index = 9*mux_state;

    /* Read all 4 auxiliary register groups */
    for (uint8_t reg_group = 0U; reg_group < 4U; reg_group++)
    {
        result = adbms6830_read_register_group(gpio_cmds[reg_group], reg_data);
        if (result != 0)
        {
            return result;
        }

        /* Extract GPIO values from this register group */
        for (uint8_t slave_idx = 0U; slave_idx < ADBMS_NUM_SLAVES; slave_idx++)
        {   
            if(reg_group == 0U)
            {
                // group A, only want to read GPIO 2 and 3
                uint8_t first_gpio_in_group = reg_group * 3U;

                for (uint8_t gpio_in_group = 1U; gpio_in_group < 3U; gpio_in_group++)
                {
                    uint8_t gpio_idx = first_gpio_in_group + gpio_in_group;
                    uint8_t byte_offset = gpio_in_group * 2U;
                    raw_adc[slave_idx][gpio_idx + mux_index] = (uint16_t)reg_data[slave_idx][byte_offset] |
                                                   ((uint16_t)reg_data[slave_idx][byte_offset + 1U] << 8);
                }
            }
            else if (reg_group < 3U)
            {
                /* Groups A, B, C: 3 GPIO values each (bytes 0-1, 2-3, 4-5) */
                uint8_t first_gpio_in_group = reg_group * 3U;

                for (uint8_t gpio_in_group = 0U; gpio_in_group < 3U; gpio_in_group++)
                {
                    uint8_t gpio_idx = first_gpio_in_group + gpio_in_group;
                    uint8_t byte_offset = gpio_in_group * 2U;
                    raw_adc[slave_idx][gpio_idx + mux_index] = (uint16_t)reg_data[slave_idx][byte_offset] |
                                                   ((uint16_t)reg_data[slave_idx][byte_offset + 1U] << 8);
                }
            }
            else
            {
                /* Group D: GPIO10 only (bytes 0-1), VM and VP are not needed */
                raw_adc[slave_idx][ADBMS_NUM_GPIOS - 1U + mux_index] = (uint16_t)reg_data[slave_idx][0] |
                                                           ((uint16_t)reg_data[slave_idx][1] << 8);
            }
        }
    }
    uint16_t sum = 0;
    for(uint8_t slave_idx = 0U; slave_idx < ADBMS_NUM_SLAVES; slave_idx++)
    {
        for(uint8_t gpio_idx = 0U; gpio_idx < ADBMS_THERMS_PER_IC; gpio_idx++)
        {
            if(raw_adc[slave_idx][gpio_idx] < 1000U)
            {
                sum += raw_adc[slave_idx][gpio_idx];
            }
            else
            {
                raw_adc[slave_idx][gpio_idx] = avg_temp_raw;
            }
        }
    }   
    avg_temp_raw = sum / (ADBMS_NUM_SLAVES * ADBMS_THERMS_PER_IC);

    return 0;
}
