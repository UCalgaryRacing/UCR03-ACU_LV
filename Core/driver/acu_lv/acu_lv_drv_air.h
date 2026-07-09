/*
 * acu_lv_drv_air.h
 *
 *  Created on: Apr 9, 2026
 *      Author: clayd
 */

#ifndef DRIVER_ACU_LV_DRV_AIR_H_
#define DRIVER_ACU_LV_DRV_AIR_H_

#include "acu_lv_config.h"

void acu_lv_drv_air_init();
acu_lv_status_t acu_lv_drv_update_sdc_reserve();
void acu_lv_drv_open_air();
void acu_lv_drv_close_air_pos();
void acu_lv_drv_close_air_neg();

#endif /* DRIVER_ACU_LV_DRV_AIR_H_ */
