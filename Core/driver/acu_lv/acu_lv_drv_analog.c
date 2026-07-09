/*
 * acu_lv_drv_analog.c
 *
 *  Created on: Apr 9, 2026
 *      Author: clayd
 */

#include "acu_lv_drv_analog.h"

acu_lv_status_t acu_lv_drv_adc_init(analog_adc_context_t *adc_context)
{
    if(adc_context == NULL)
    {
        return ACU_LV_ERROR;
    }

    if(adc_context->calibrated == false)
    {
        if(HAL_ADCEx_Calibration_Start(adc_context->adc_handle,ADC_CALIB_OFFSET,ADC_SINGLE_ENDED) != HAL_OK)
        {
            return ACU_LV_ERROR;
        }
        adc_context->calibrated = true;
    }
    return ACU_LV_OK;
}

acu_lv_status_t acu_lv_drv_adc_start_dma(analog_hw_t * hw, uint32_t * adc_buffer)
{
    if(HAL_ADC_Start_DMA(hw->adc_context->adc_handle,adc_buffer,hw->adc_context->adc_channels) != HAL_OK)
    {
        return ACU_LV_ERROR;
    }

    hw->adc_context->dma_started = true;

    return ACU_LV_OK;
}

