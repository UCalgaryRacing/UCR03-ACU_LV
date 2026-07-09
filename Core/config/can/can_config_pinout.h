/*
 * can_config_pinout.h
 *
 *  Created on: Apr 10, 2026
 *      Author: clayd
 */

#ifndef CONFIG_CAN_CAN_CONFIG_PINOUT_H_
#define CONFIG_CAN_CAN_CONFIG_PINOUT_H_

#include "stm32h7xx_hal.h"

extern FDCAN_HandleTypeDef hfdcan2;


#define CAN_1_HANDLE (&hfdcan2)
#define CAN_2_HANDLE (&hfdcan2)

#endif /* CONFIG_CAN_CAN_CONFIG_PINOUT_H_ */
