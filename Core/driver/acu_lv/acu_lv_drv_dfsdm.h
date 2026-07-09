/*
 * acu_lv_drv_dfsdm.h
 *
 *  Created on: Apr 7, 2026
 *      Author: clayd
 */

#ifndef DRIVER_ACU_LV_DRV_DFSDM_H_
#define DRIVER_ACU_LV_DRV_DFSDM_H_

#include "acu_lv_config.h"
#include "stm32h7xx_hal.h"

acu_lv_status_t acu_lv_start_DFSDM_filter(DFSDM_Filter_HandleTypeDef * filter, int32_t * buffer);
acu_lv_status_t acu_lv_stop_DFSDM_filter(DFSDM_Filter_HandleTypeDef * filter);

#endif /*DRIVER_ACU_LV_DRV_DFSDM_H_*/