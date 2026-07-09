/*
 * acu_lv_drv_air.c
 *
 *  Created on: Apr 9, 2026
 *      Author: clayd
 */

#include "acu_lv_drv_air.h"

extern acu_lv_air_t air;
extern uint16_t g_adc_2_dma_buffer[ACU_LV_ADC_2_MAX_NUMBER_CHANNELS];
extern uint16_t g_adc_3_dma_buffer[ACU_LV_ADC_3_MAX_NUMBER_CHANNELS];

void acu_lv_drv_air_init()
{
    air.air_hw.neg_state = GPIO_PIN_RESET;
    air.air_hw.pos_state = GPIO_PIN_RESET;

    HAL_GPIO_WritePin(air.air_hw.air_neg_port, air.air_hw.air_neg_pin,air.air_hw.neg_state);

    HAL_GPIO_WritePin(air.air_hw.air_pos_port, air.air_hw.air_pos_pin,air.air_hw.pos_state);

    air.air_closed = false;
}

acu_lv_status_t acu_lv_drv_update_sdc_reserve()
{
   if(air.analog_hw->adc_context->dma_started == false)
   {
       return ACU_LV_ERROR;
   }

    air.sdc_reserve = ((float)air.analog_hw->adc_buffer[air.adc_buffer_index] * air.analog_hw->adc_context->adc_vref * 
                        air.analog_hw->adc_context->adc_scaling) / air.analog_hw->adc_context->adc_max;

    return ACU_LV_OK;
}

void acu_lv_drv_open_air()
{
    air.air_hw.neg_state = GPIO_PIN_RESET;
    air.air_hw.pos_state = GPIO_PIN_RESET;

    HAL_GPIO_WritePin(air.air_hw.air_neg_port, air.air_hw.air_neg_pin,air.air_hw.neg_state);

    HAL_GPIO_WritePin(air.air_hw.air_pos_port, air.air_hw.air_pos_pin,air.air_hw.pos_state);

    air.air_closed = false;
}

void acu_lv_drv_close_air_pos()
{
    air.air_hw.pos_state = GPIO_PIN_SET;
    HAL_GPIO_WritePin(air.air_hw.air_pos_port, air.air_hw.air_pos_pin,air.air_hw.pos_state);

//    // after closing positive, check if negative is also closed and update the closed flag
//    if(air.air_hw.neg_state == GPIO_PIN_SET)
//    {
//        air.air_closed = true;
//    }
//    else
//    {
//        air.air_closed = false;
//    }
}

void acu_lv_drv_close_air_neg()
{
    air.air_hw.neg_state = GPIO_PIN_SET;
    HAL_GPIO_WritePin(air.air_hw.air_neg_port, air.air_hw.air_neg_pin,air.air_hw.neg_state);

//    // after closing negative, check if pos is also closed and update the closed flag
//    if(air.air_hw.pos_state == GPIO_PIN_SET)
//    {
//        air.air_closed = true;
//    }
//    else
//    {
//        air.air_closed = false;
//    }
}
