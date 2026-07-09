/*
 * medium_task.c
 *
 *  Created on: Apr 7, 2026
 *      Author: clayd
 */

#include "medium_task.h"
#include "cmsis_os2.h"
#include "acu_lv_drv_debug_led.h"
#include "acu_lv_drv_imd.h"
#include "acu_lv_drv_analog.h"
#include "acu_lv_config.h"
#include "acu_lv_drv_air.h"

const static uint32_t period = 50;
static uint32_t next_wake;

extern debug_led_t green_led;

void medium_task_init()
{
    next_wake = osKernelGetTickCount();

    acu_lv_drv_turn_off_led(&green_led);
}
void medium_task_loop()
{
    next_wake += period;
    osDelayUntil(next_wake);
    // acu_lv_drv_toggle_led(&green_led);

    // check imd state
    // acu_lv_drv_update_imd_state();

    // // update sdc reserve
    // acu_lv_drv_update_sdc_reserve();

//    acu_lv_drv_turn_off_led(&green_led);
}

