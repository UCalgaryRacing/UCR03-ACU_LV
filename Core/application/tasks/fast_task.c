/*
 * fast_task.c
 *
 *  Created on: Apr 7, 2026
 *      Author: clayd
 */
// test commit
#include <stdint.h>
#include "cmsis_os2.h"
#include "acu_lv_config.h"

#include "fast_task.h"
#include "acu_lv_svc_accu.h"
#include "acu_lv_svc_ts.h"
#include "acu_lv_svc_shunt.h"
#include "acu_lv_svc_adbms6830.h"

#include "acu_lv_drv_debug_led.h"
#include "acu_lv_drv_imd.h"
#include "acu_lv_drv_analog.h"
#include "acu_lv_drv_air.h"
#include "acu_lv_drv_sdc.h"
#include "acu_lv_drv_adbms6830.h"
#include "acu_lv_drv_adbms6830_types.h"
#include "acu_lv_drv_adbms6830_regs.h"
#include "aculv_drv_ds18b20.h"

#include "acu_lv_svc_adbms6830.h"
#include "bms_svc_thermistor.h"
#include "bms_svc_fault.h"
#include "bms_svc_can.h"

#include "rco_data.h"
#include "acu_data.h"
#include "acu_lv_svc_sdc.h"

#include "stm32h7xx_hal.h"
#include "acu_lv_config_pinout.h"

#include "acu_svc_can_route.h"

extern debug_led_t red_led;
const static uint32_t period = 10;
static uint32_t next_wake;

extern debug_led_t blue_led;
extern analog_hw_t adc_2_hw, adc_3_hw;

extern uint16_t g_adc_2_dma_buffer[ACU_LV_ADC_2_MAX_NUMBER_CHANNELS];
extern uint16_t g_adc_3_dma_buffer[ACU_LV_ADC_3_MAX_NUMBER_CHANNELS];

extern analog_adc_context_t adc_2_context;
extern ADC_HandleTypeDef hadc2;

int g_result;
uint32_t i = 0;

bool g_bms_valid;

GPIO_PinState g_wake_pin_state;
// initialization functions for the fast task
void fast_task_init()
{   
    // add code to turn on one of the three leds
    next_wake = osKernelGetTickCount();

    // start the conversion for measuring accumulator and tractive voltage
    acu_lv_svc_start_ts_filter();
    acu_lv_svc_start_accu_filter();
    acu_lv_svc_start_shunt_filter();
    
    // calibrate both the ADCs
    acu_lv_drv_adc_init(adc_2_hw.adc_context);
    acu_lv_drv_adc_init(adc_3_hw.adc_context);

    //start the DMA for ADCs
    acu_lv_drv_adc_start_dma(&adc_2_hw, (uint32_t*)g_adc_2_dma_buffer);
    acu_lv_drv_adc_start_dma(&adc_3_hw,(uint32_t*)g_adc_3_dma_buffer);
    // HAL_ADCEx_Calibration_Start(&hadc2, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED);
    // HAL_ADC_Start_DMA(&hadc2,&g_test_buffer,1);

    // initialize airs, should make svc layer do it eventually
    acu_lv_drv_air_init();

    // initialize sdc
    acu_lv_svc_sdc_init();

    bms_manager_init();

    acu_lv_drv_turn_on_led(&blue_led);

    acu_svc_can_route_init();

    // uses russian init code, victor says it works but that was on G4
    // some parts were changes to work for H7 and uses UART instead of USART, hopefully it works
    aculv_drv_ds18b20_init();

    osDelay(5000);
}

// code that runs in the infinite loop for the fast task
void fast_task_loop()
{
     next_wake += period;
     osDelayUntil(next_wake);
    
    g_wake_pin_state = HAL_GPIO_ReadPin(ADBMS_1_WAKE_PORT, ADBMS_1_WAKE_PIN);
    // update the values for tractive and accumulator voltage
    acu_lv_svc_update_ts_voltage();
    acu_lv_svc_update_accu_voltage();
    acu_lv_svc_update_shunt();

    acu_lv_drv_update_imd_state();

    // update sdc reserve
    acu_lv_drv_update_sdc_reserve();


//     while(1)
//     {
//
//         acu_lv_drv_toggle_led(&red_led);
//         acu_lv_svc_close_sdc();
//         acu_lv_drv_close_air_neg();
//         for (i = 0; i < 5000000; i++)
//         {
//        	    acu_lv_svc_update_ts_voltage();
//        	    acu_lv_svc_update_accu_voltage();
//         }
//         osDelay(5000);
//         acu_lv_drv_toggle_led(&red_led);
//         acu_lv_drv_close_air_pos();
//         acu_lv_drv_toggle_led(&red_led);
//         for (i = 0; i < 5000000; i++)
//         {
//        	    acu_lv_svc_update_ts_voltage();
//        	    acu_lv_svc_update_accu_voltage();
//         }
//         osDelay(50000);
//         acu_lv_svc_open_sdc();
//         osDelay(20000);
//     }


//    while(1)
//    {
//        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
//        acu_lv_drv_toggle_led(&red_led);
//        HAL_Delay(2000);
//        HAL_GPIO_WritePin(GPIOH, GPIO_PIN_4, GPIO_PIN_SET);
//        HAL_Delay(2000);
//        HAL_GPIO_WritePin(GPIOH, GPIO_PIN_4, GPIO_PIN_RESET);
//        HAL_Delay(2000);
//    }

    //6830 cell voltage acquisition
    // voltage_acquisition_sample();
    vw_voltages();

    // adbms 6830 thermistor
    // bms_svc_acquire_thermistor_temps(0U);
    // bms_svc_admbs_toggle_mux(ADBMS_GPO_PIN_1);
    // bms_svc_acquire_thermistor_temps(1U);
    // bms_svc_admbs_toggle_mux(ADBMS_GPO_PIN_1);
    get_Temperature(); // russian get temperature code from victor, if it works then we can start VWing

    bms_svc_check_faults();

    bms_svc_can_tx_acu_fault_data();

    bms_svc_can_tx_acu_data();

    // code to reset imd and ams latch based on message from rear controller (rco), shouldn't need because it is handled in hardware
    // if(rco_data_get_reset_button() == 1)
    // {
    //     acu_lv_svc_reset_ams_latch();
    //     acu_lv_svc_reset_imd_latch();
    // }

    acu_lv_drv_toggle_led(&blue_led);    
}
