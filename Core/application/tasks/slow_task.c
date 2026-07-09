/*
 * slow_task.c
 *
 *  Created on: Apr 7, 2026
 *      Author: clayd
 */

#include "slow_task.h"
#include "cmsis_os2.h"
#include "acu_lv_drv_debug_led.h"
#include "acu_data.h"

const static uint32_t period = 5000;
static uint32_t next_wake;

extern debug_led_t red_led;

void slow_task_init()
{
    next_wake = osKernelGetTickCount();
    acu_lv_drv_turn_off_led(&red_led);
}
void slow_task_loop()
{
    next_wake += period;
    osDelayUntil(next_wake);
//    acu_lv_drv_toggle_led(&red_led);
}

