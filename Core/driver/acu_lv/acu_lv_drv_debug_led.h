/*
 * acu_lv_drv_debug_led.h
 *
 *  Created on: Apr 8, 2026
 *      Author: clayd
 */

#ifndef DRIVER_ACU_LV_DRV_DEBUG_LED_H_
#define DRIVER_ACU_LV_DRV_DEBUG_LED_H_

#include "acu_lv_config.h"

static inline void acu_lv_drv_turn_on_led(debug_led_t *led)
{
    led->state = GPIO_PIN_SET;
    HAL_GPIO_WritePin(led->port,led->pin,led->state);
}

static inline void acu_lv_drv_turn_off_led(debug_led_t * led)
{
    led->state = GPIO_PIN_RESET;
    HAL_GPIO_WritePin(led->port,led->pin,led->state);
}

static inline void acu_lv_drv_toggle_led(debug_led_t * led)
{
    (led->state == GPIO_PIN_RESET) ? (led->state = GPIO_PIN_SET) : (led->state = GPIO_PIN_RESET);
    HAL_GPIO_WritePin(led->port,led->pin,led->state);
}

#endif /* DRIVER_ACU_LV_DRV_DEBUG_LED_H_ */
