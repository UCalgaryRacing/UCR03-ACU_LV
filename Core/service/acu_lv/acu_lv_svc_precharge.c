/*
 * acu_lv_svc_precharge.c
 *
 *  Created on: Apr 10, 2026
 *      Author: clayd
 */

#include <stdint.h>
#include "stm32h7xx_hal.h"
#include "cmsis_os2.h"
#include "acu_lv_config.h"
#include "acu_lv_svc_precharge.h"
#include "acu_lv_svc_accu.h"
#include "acu_lv_svc_ts.h"

uint8_t precharge_time;

extern TIM_HandleTypeDef htim3;

bool acu_lv_svc_check_precharge_done()
{   
    if(acu_lv_svc_get_accu_voltage() > 1)
    {
        if((acu_lv_svc_get_accu_voltage() * 0.9f) <= acu_lv_svc_get_ts_voltage())
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
    
}

bool acu_lv_svc_check_precharge_timeout()
{
//    current_time = osKernelGetTickCount();
    if(precharge_time == 1)
    {
        return true;
    }
    return false;
}


void acu_lv_svc_start_precharge_timer()
{
    HAL_TIM_Base_Start_IT(&htim3);
    precharge_time = 0;
}

void acu_lv_svc_stop_precharge_timer()
{
	HAL_TIM_Base_Stop_IT(&htim3);
    precharge_time = 0;
}
