/*
 * acu_lv_drv_analog.h
 *
 *  Created on: Apr 9, 2026
 *      Author: clayd
 */

#ifndef DRIVER_ACU_LV_DRV_ANALOG_H_
#define DRIVER_ACU_LV_DRV_ANALOG_H_

#include <stdint.h>
#include "acu_lv_config.h"
#include "acu_lv_config.h"

acu_lv_status_t acu_lv_drv_adc_init(analog_adc_context_t * adc_context);
acu_lv_status_t acu_lv_drv_adc_start_dma(analog_hw_t * hw, uint32_t * adc_buffer);

#endif /* DRIVER_ACU_LV_DRV_ANALOG_H_ */
