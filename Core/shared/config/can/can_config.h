/*
 * can_config.h
 *
 *  Created on: Apr 10, 2026
 *      Author: clayd
 */

#ifndef CONFIG_CAN_CONFIG_H_
#define CONFIG_CAN_CONFIG_H_

#include <stdint.h>
#include <stdbool.h>
#include "can_config_pinout.h"

/*============================================================================*/
/* Types                                                                      */
/*============================================================================*/

typedef struct {
    bool started;
} can_context_t;


/*============================================================================*/
/* Channel Hardware Mapping                                                   */
/*============================================================================*/

typedef struct
{
    FDCAN_HandleTypeDef *can_handle;
    can_context_t *context;
} can_hw_t;


typedef struct
{
    bool enabled;
    can_hw_t hw;
} can_config_t;

//Accumulator
#define ACCUMULATOR_DATA_CAN_ID                 199
#define ACCUMULATOR_FAULT_CAN_ID                300



//Front Controller 
#define FRONT_CONTROLLER_STATE_CAN_ID           200
#define APPS_CAN_ID                             201
#define FRONT_BSE_CAN_ID                        202     
#define MOTOR_DATA_CAN_ID                       203
#define MOTOR_REQUEST_CAN_ID                    204  
#define FRONT_PMD_CURRENT_CAN_ID                205
#define FRONT_WHEEL_SPEED_CAN_ID                206
#define FRONT_SUSPENSION_CAN_ID                 207
#define FRONT_INSTRUMENTATION_CAN_ID            208

//Rear Controller
#define REAR_CONTROLLER_STATE_CAN_ID            400
#define GLV_BATTERY_CAN_ID                      401
#define REAR_PDM_CURRENT_CAN_ID                 402
#define REAR_WHEEL_SPEED_CAN_ID                 403
#define REAR_COOLING_CONTROL_CAN_ID             404
#define REAR_CONTROL_CAN_ID                     405
#define REAR_BSE_CAN_ID                         406
#define REAR_COOLANT_TEMP_CAN_ID                407
#define REAR_SUSPENSION_CAN_ID                  408
#define BSPD_CAN_ID                             409
#define REAR_INSTRUMENTATION_CAN_ID             410

//Wheel
#define STEERING_WHEEL_CAN_ID                   500

//Bamocar
#define BAMOCAR_CAN_TX_ID                       513 //to bamocar
#define BAMOCAR_CAN_RX_ID                       385 //from bamocar

//Brusa
#define CHARGER_COMMAND_CAN_ID                  1560
#define CHARGER_STATUS_CAN_ID                   1552
#define CHARGER_ACTUAL_1_CAN_ID                 1553
#define CHARGER_ACTUAL_2_CAN_ID                 1554
#define CHARGER_TEMP_CAN_ID                     1555
#define CHARGER_ERROR_CAN_ID                    1556

#endif /* CONFIG_CAN_CONFIG_H_ */


/*============================================================================*/
/* Extern Configuration                                                       */
/*============================================================================*/

extern const can_config_t can_config[];