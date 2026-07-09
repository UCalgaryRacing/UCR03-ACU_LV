/*
 * acu_lv_drv_adbms6830.h
 *
 * ADBMS6830B Battery Stack Monitor Driver - Public API
 *
 * This driver provides a high-level interface for the ADBMS6830B
 * 16-channel battery monitor IC. It handles:
 * - SPI communication with daisy-chained devices
 * - Cell voltage and GPIO measurements
 * - Cell balancing control
 * - Configuration management with shadow registers
 * - Diagnostic and fault detection
 *
 * Thread Safety:
 * - All SPI operations are protected by a mutex with priority inheritance
 * - Safe to call from multiple RTOS tasks
 *
 *  Created on: Nov 12, 2025
 *      Author: esall
 */

#ifndef DRIVERS_ADBMS6830_H_
#define DRIVERS_ADBMS6830_H_

#include <stdint.h>
#include <stdbool.h>
#include "acu_lv_drv_adbms6830_regs.h"
#include "acu_lv_config.h"


/*============================================================================*/
/* Driver Initialization                                                      */
/*============================================================================*/

/**
 * @brief Initialize the ADBMS6830 driver.
 *
 * Creates the SPI bus mutex for thread-safe operation.
 * Must be called once before any other driver functions.
 *
 * @return 0 on success, -1 on failure (mutex creation failed).
 */
int adbms6830_init(void);

/*============================================================================*/
/* Wake-Up and Power Management                                               */
/*============================================================================*/

/**
 * @brief Wake up all devices in the daisy chain.
 *
 * Sends a series of CS pulses with appropriate timing to wake devices
 * from SLEEP or STANDBY state. Uses the robust method from datasheet:
 * toggle CS for each device with 1ms waits between pulses.
 *
 * Call this before first communication or after extended idle periods.
 *
 * @return 0 on success.
 */
int adbms6830_wakeup(void);

/**
 * @brief Send a single wake-up pulse (CS low then high).
 *
 * For advanced use - use adbms6830_wakeup() for standard wake-up.
 * This sends a minimal CS toggle to generate a wake-up signal.
 */
void adbms6830_wakeup_pulse(void);

/*============================================================================*/
/* Shadow Register Management                                                 */
/*============================================================================*/

/**
 * @brief Initialize the CFGA shadow buffer and clear validity.
 *
 * @param shadow Pointer to shadow object to initialize.
 */
void adbms6830_shadow_init(adbms6830_shadow_t *shadow);

/**
 * @brief Update the CFGA shadow with a new payload.
 *
 *
 * @param shadow Pointer to shadow object.
 * @param data   Pointer to CFGA payload (6 bytes).
 */
void adbms6830_shadow_set_cfga(adbms6830_shadow_t *shadow,
                               const uint8_t data[ADBMS_REG_GROUP_SIZE]);

/**
 * @brief Get a pointer to the CFGA shadow payload.
 *
 * @param shadow Pointer to shadow object.
 * @return Pointer to 6-byte CFGA payload, or NULL on bad input.
 */
const uint8_t *adbms6830_shadow_get_cfga(const adbms6830_shadow_t *shadow);

/**
 * @brief Write Configuration Register Group A (CFGA) from the shadow to the IC.
 *
 * Builds the WRCFGA SPI command frame with PEC and transmits it.
 *
 * @param shadow Pointer to CFGA shadow (source payload).
 * @return ADBMS_STATUS_OK on success, error code on failure.
 */
int adbms6830_write_cfga(const adbms6830_shadow_t *shadow);

/**
 * @brief Read Configuration Register Group A (CFGA) from the IC into the shadow.
 *
 * Issues the RDCFGA command, validates the data PEC, and updates @p shadow.
 *
 * @param shadow Pointer to CFGA shadow (destination payload).
 * @return ADBMS_STATUS_OK on success, error code on failure.
 */
int adbms6830_read_cfga(adbms6830_shadow_t *shadow);

/*============================================================================*/
/* Cell Voltage Measurement                                                   */
/*============================================================================*/

/**
 * @brief Start cell voltage ADC conversion on all slaves.
 *
 * Sends the ADCV command to trigger conversion. After calling this,
 * wait at least ADBMS_ADCV_CONV_TIME_MS before reading results,
 * or use adbms6830_poll_cell_adc() to check completion.
 *
 * @param discharge_permitted  If true, allows cell balancing during conversion.
 * @return 0 on success, negative error code on failure.
 */
int adbms6830_start_cell_adc(bool discharge_permitted);

/**
 * @brief Poll cell voltage ADC conversion status.
 *
 * Checks if the C-ADC conversion is complete on all devices in the stack.
 * This is an alternative to using a fixed delay after starting conversion.
 *
 * @return true if conversion is complete, false if still in progress.
 */
bool adbms6830_poll_cell_adc(void);

/**
 * @brief Poll all ADC conversion status.
 *
 * Checks if all ADC conversions (C-ADC, S-ADC, AUX, AUX2) are complete.
 *
 * @return true if all conversions are complete, false if any is in progress.
 */
bool adbms6830_poll_adc(void);

/**
 * @brief Read cell voltages from all slaves.
 *
 * Reads all 6 cell voltage register groups and extracts 16 cell voltages
 * per slave. Results are returned as raw ADC counts (16-bit).
 *
 * @param raw_adc  Output buffer [ADBMS_NUM_SLAVES][ADBMS_CELLS_PER_IC].
 *                 Each value is a 16-bit ADC count.
 * @return 0 on success, negative error code on failure (e.g., PEC error).
 */
int adbms6830_read_cell_voltages_raw(uint16_t raw_adc[ADBMS_NUM_SLAVES][ADBMS_CELLS_PER_IC]);

/**
 * @brief Convert raw ADC count to voltage in volts.
 *
 * @param raw_adc  Raw 16-bit ADC count from cell voltage register.
 * @return Voltage in volts (V).
 */
static inline float adbms6830_adc_to_volts(uint16_t raw_adc)
{
    return (float)raw_adc * ADBMS_CELL_ADC_LSB_V + 1.5f;
}

/*============================================================================*/
/* GPIO (Auxiliary) Measurement                                               */
/*============================================================================*/

/**
 * @brief Start GPIO ADC conversion on all slaves.
 *
 * Sends the ADAX command to trigger conversion on all GPIO inputs.
 * After calling this, wait at least ADBMS_ADAX_CONV_TIME_MS before reading,
 * or use adbms6830_poll_gpio_adc() to check completion.
 *
 * @return 0 on success, negative error code on failure.
 */
int adbms6830_start_gpio_adc(void);

/**
 * @brief Poll GPIO ADC conversion status.
 *
 * Checks if the AUX ADC conversion is complete on all devices in the stack.
 *
 * @return true if conversion is complete, false if still in progress.
 */
bool adbms6830_poll_gpio_adc(void);

/**
 * @brief Read GPIO voltages from all slaves.
 *
 * Reads all 4 auxiliary register groups and extracts GPIO1-10 voltages
 * per slave. Results are returned as raw ADC counts (16-bit).
 *
 * @param raw_adc  Output buffer [ADBMS_NUM_SLAVES][ADBMS_NUM_GPIOS].
 *                 Index 0 = GPIO1, Index 9 = GPIO10.
 *                 Each value is a 16-bit ADC count.
 * @return 0 on success, negative error code on failure (e.g., PEC error).
 */
int adbms6830_read_gpio_voltages_raw(uint16_t raw_adc[ADBMS_NUM_SLAVES][ADBMS_THERMS_PER_IC],uint8_t mux_state);

/**
 * @brief Convert raw GPIO ADC count to voltage in volts.
 *
 * @param raw_adc  Raw 16-bit ADC count from GPIO register.
 * @return Voltage in volts (V).
 */
static inline float adbms6830_gpio_adc_to_volts(uint16_t raw_adc)
{
    return (float)raw_adc * ADBMS_GPIO_ADC_LSB_V;
}

#endif /* DRIVERS_ADBMS6830_H_ */
