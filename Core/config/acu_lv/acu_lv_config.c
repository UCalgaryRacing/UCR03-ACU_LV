/**
 * Configuration for the entire ACU LV
 * 
 */

#include "acu_lv_config.h"
#include "acu_lv_config_pinout.h"
#include "stm32h7xx_hal.h"

/*============================================================================*/
/* Shunt, TS and ACCU Measurement Configuration */
/*============================================================================*/

float g_shunt_data_float;
int32_t g_shunt_data_raw;

acu_lv_pack_measurement_t pack_current= {
    .hw = {
        .status_port = SHUNT_STATUS_PORT,
        .status_pin = SHUNT_STATUS_PIN,
        .dma_started = false,
        .filter_handle = CURRENT_SHUNT_FILTER_HANDLE
    },
    .settings = {
        .measurement_type = ACU_LV_MEASUREMENT_PACK_CURRENT,
        .scaling_factor = ACU_LV_SHUNT_SCALING_FACTOR,
        .valid_max = 10,
        .valid_min = 0
    },
    .data = &g_shunt_data_float,
    .raw_data = &g_shunt_data_raw
};

float g_accu_voltage_float;
int32_t g_accu_voltage_raw;

acu_lv_pack_measurement_t accu_voltage = {
    .hw = {
        .status_port = ACCU_VOLTAGE_STATUS_PORT,
        .status_pin = ACCU_VOLTAGE_STATUS_PIN,
        .dma_started = false,
        .filter_handle = ACCU_VOLTAGE_FILTER
    },
    .settings = {
        .measurement_type = ACU_LV_MEASUREMENT_ACCU_VOLTAGE,
        .scaling_factor = ACU_LV_ACCU_SCALING_FACTOR,
        .valid_max = ACCU_VOLTAGE_MAX_V,
        .valid_min = ACCU_VOLTAGE_MIN_V
    },
    .data = &g_accu_voltage_float,
    .raw_data = &g_accu_voltage_raw
};

float g_ts_data_float;
int32_t g_ts_data_raw;

acu_lv_pack_measurement_t ts_voltage = {
    .hw = {
        .status_port = TS_VOLTAGE_STATUS_PORT,
        .status_pin = TS_VOLTAGE_STATUS_PIN,
        .dma_started = false,
        .filter_handle = TS_VOLTAGE_FILTER
    },
    .settings = {
        .measurement_type = ACU_LV_MEASUREMENT_TS_VOLTAGE,
        .scaling_factor = ACU_LV_TS_SCALING_FACTOR,
        .valid_min = 0,
        .valid_max = ACCU_VOLTAGE_MAX_V //should this be 90% of acu?
    },
    .data = &g_ts_data_float,
    .raw_data = &g_ts_data_raw
};

/*============================================================================*/
/* ADC Configuration*/
/*============================================================================*/

uint16_t g_adc_2_dma_buffer[ACU_LV_ADC_2_MAX_NUMBER_CHANNELS];
uint16_t g_adc_3_dma_buffer[ACU_LV_ADC_3_MAX_NUMBER_CHANNELS];

analog_adc_context_t adc_2_context = {
    .adc_handle = ACU_LV_ADC_2_HANDLE,
    .adc_channels = ACU_LV_ADC_2_MAX_NUMBER_CHANNELS,
    .adc_max = ACU_LV_ADC_2_MAX_COUNTS,
    .adc_vref = ACU_LV_ADC_VREF,
    .adc_scaling = ACU_LV_ADC_SCALING_FACTOR,
    .calibrated = false,
    .dma_started = false
};

analog_hw_t adc_2_hw = {
    .adc_context = &adc_2_context,
    .adc_buffer = g_adc_2_dma_buffer
};

analog_adc_context_t adc_3_context = {
    .adc_handle = ACU_LV_ADC_3_HANDLE,
    .adc_channels = ACU_LV_ADC_3_MAX_NUMBER_CHANNELS,
    .adc_max = ACU_LV_ADC_3_MAX_COUNTS,
    .adc_vref = ACU_LV_ADC_VREF,
    .adc_scaling = ACU_LV_ADC_SCALING_FACTOR,
    .calibrated = false,
    .dma_started = false
};

analog_hw_t adc_3_hw = {
    .adc_context = &adc_3_context,
    .adc_buffer = g_adc_3_dma_buffer
};

/*============================================================================*/
/* AIR Configuration*/
/*============================================================================*/


acu_lv_air_t air = {
    .analog_hw = &adc_2_hw,
    .air_hw = {
        .air_neg_port = AIR_NEG_EN_PORT,
        .air_neg_pin = AIR_NEG_EN_PIN,
        .neg_state = GPIO_PIN_RESET,
        .air_pos_port = AIR_POS_EN_PORT,
        .air_pos_pin = AIR_POS_EN_PIN,
        .pos_state = GPIO_PIN_RESET
    },
    .adc_buffer_index = SDC_RESERVE_BUFFER_INDEX,
    .sdc_reserve = 0.0f,
    .air_closed = false
};

/*============================================================================*/
/* IMD*/
/*============================================================================*/

acu_lv_imd_t imd = {
    .hw = {
        .imd_m_port = IMD_M_PORT,
        .imd_m_pin = IMD_M_PIN,
        .imd_ok_port = IMD_OK_PORT,
        .imd_ok_pin = IMD_OK_PIN
    },
    .state = ACU_LV_IMD_FAULT
};

/*============================================================================*/
/* SDC*/
/*============================================================================*/

acu_lv_sdc_t sdc = {
    .hw = {
        .imd_latch_reset_port = IMD_LATCH_RESET_PORT,
        .imd_latch_reset_pin = IMD_LATCH_RESET_PIN,
        .imd_latch_state = GPIO_PIN_RESET,

        .ams_latch_reset_port = AMS_LATCH_RESET_PORT,
        .ams_latch_reset_pin = AMS_LATCH_RESET_PIN,
        .ams_latch_state = GPIO_PIN_RESET,

        .ams_ok_port = AMS_OK_PORT,
        .ams_ok_pin = AMS_OK_PIN,
        .ams_ok_state = GPIO_PIN_RESET
    },
    .shutdown_closed = false
};

/*============================================================================*/
/* Debug LEDs*/
/*============================================================================*/

debug_led_t blue_led = {
    .port = LED_1_PORT,
    .pin = LED_1_PIN,
    .state = GPIO_PIN_RESET
};

debug_led_t green_led = {
    .port = LED_2_PORT,
    .pin = LED_2_PIN,
    .state = GPIO_PIN_RESET
};

debug_led_t red_led = {
    .port = LED_3_PORT,
    .pin = LED_3_PIN,
    .state = GPIO_PIN_RESET
};
