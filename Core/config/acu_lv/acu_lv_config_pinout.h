/*
 * acu_lv_config_pinout.h
 *
 * Pinout definitions for the accumulator voltage measurement system.
 * This file maps the microcontroller's GPIO pins to their functions
 * in the system (e.g., SPI CS lines, ADC inputs, GPIOs for ADBMS6830).
 * All pin assignments should be defined here for easy reference and modification.
 * 
 *  Created on: Mar 23, 2026
 *      Author: f-dunnwolbaum
 */

#ifndef CONFIG_ACU_LV_CONFIG_PINOUT_H_
#define CONFIG_ACU_LV_CONFIG_PINOUT_H_

#include "stm32h7xx_hal.h"


/*============================================================================*/
/* BMS1 ISO SPI                                                               */
/*============================================================================*/

extern SPI_HandleTypeDef hspi5;
#define ADBMS_1_SPI_HANDLE (&hspi5)

#define ADBMS_1_CSN_PORT GPIOD
#define ADBMS_1_CSN_PIN GPIO_PIN_9

#define ADBMS_1_WAKE_PORT GPIOD
#define ADBMS_1_WAKE_PIN GPIO_PIN_14

#define ADMBS_1_INTR_PORT GPIOD
#define ADBMS_1_INTR_PIN GPIO_PIN_13


/*============================================================================*/
/* BMS2 ISO SPI                                                               */
/*============================================================================*/

extern SPI_HandleTypeDef hspi2;
//debug
// #define ADBMS_2_SPI_HANDLE (&hspi5)

// #define ADBMS_2_CSN_PORT GPIOD
// #define ADBMS_2_CSN_PIN GPIO_PIN_9 

// #define ADBMS_2_WAKE_PORT GPIOD
// #define ADBMS_2_WAKE_PIN GPIO_PIN_14

// #define ADMBS_2_INTR_PORT GPIOD
// #define ADBMS_2_INTR_PIN GPIO_PIN_13

// normal
// extern SPI_HandleTypeDef hspi2;
// #define ADBMS_2_SPI_HANDLE (&hspi2)

// #define ADBMS_2_CSN_PORT GPIOD
// #define ADBMS_2_CSN_PIN GPIO_PIN_11

// #define ADBMS_2_WAKE_PORT GPIOD
// #define ADBMS_2_WAKE_PIN GPIO_PIN_8

// #define ADMBS_2_INTR_PORT GPIOD
// #define ADBMS_2_INTR_PIN GPIO_PIN_10

/*============================================================================*/
/* ADC                                                                        */
/*============================================================================*/

// TODO determine how to define the dma buffers for the ADCs
// could copy TCU and have a large array to support all ADC channels
// or change it to be one array per ADC and use the defines for index
// that way would have to have define to map the name to the ADC array

#define STM_ADC_SCALING_FACTOR 0.1240079365079365f

// Adcs handles SDC monitering and power for AIR coils
extern ADC_HandleTypeDef hadc2;
extern ADC_HandleTypeDef hadc3;

#define ACU_LV_ADC_2_HANDLE (&hadc2)
#define ACU_LV_ADC_3_HANDLE (&hadc3)

//rank 1 on adc3, adc3 input 3
#define SDC_MID_1_ADC_HANDLE (&hadc3)
#define SDC_MID_1_BUFFER_INDEX  0U
#define SDC_MID_1_BUFFER_LENGTH 4U

// rank 2 on adc3, adc3 input 5
#define SDC_MID_2_ADC_HANDLE (&hadc3)
#define SDC_MID_2_BUFFER_INDEX 1U
#define SDC_MID_2_BUFFER_LENGTH 4U

// rank 3 on adc3, adc3 input 2
#define SDC_8_ADC_HANDLE (&hadc3)
#define SDC_8_BUFFER_INDEX 2U
#define SDC_8_BUFFER_LENGTH 4U

// rank 4 on adc3, adc3 input 4
#define SDC_END_ADC_HANDLE (&hadc3)
#define SDC_END_BUFFER_INDEX 3U
#define SDC_END_END_BUFFER_LENGTH

// rank 1 on adc2, adc2 input 5
#define SDC_7_ADC_HANDLE (&hadc2)
#define SDC_7_BUFFER_INDEX 0U
#define SDC_7_BUFFER_LENGTH 2U

// rank 2 on adc2, adc2 input 3
#define SDC_RESERVE_ADC_HANDLE (&hadc2)
#define SDC_RESERVE_BUFFER_INDEX 1U
#define SDC_RESERVE_BUFFER_LENGTH 2U

/*============================================================================*/
/* Current Shunt (AMC33X6XX)                                                  */
/*============================================================================*/

extern DFSDM_Filter_HandleTypeDef hdfsdm1_filter2;
#define CURRENT_SHUNT_FILTER_HANDLE (&hdfsdm1_filter2)

#define SHUNT_STATUS_PORT GPIOG
#define SHUNT_STATUS_PIN GPIO_PIN_0

/*============================================================================*/
/* TS Voltage Measurement  (AMC33X6XX)                                        */
/*============================================================================*/

extern DFSDM_Filter_HandleTypeDef hdfsdm1_filter0;
#define TS_VOLTAGE_FILTER    (&hdfsdm1_filter0)

#define TS_VOLTAGE_STATUS_PORT GPIOG
#define TS_VOLTAGE_STATUS_PIN GPIO_PIN_1

/*============================================================================*/
/* Pack Voltage Measurement (AMC33X6XX)                                       */
/*============================================================================*/

extern DFSDM_Filter_HandleTypeDef hdfsdm1_filter1;
#define ACCU_VOLTAGE_FILTER  (&hdfsdm1_filter1)

#define ACCU_VOLTAGE_STATUS_PORT GPIOF
#define ACCU_VOLTAGE_STATUS_PIN GPIO_PIN_14

/*============================================================================*/
/* AIR                                                                        */
/*============================================================================*/

#define AIR_NEG_EN_PORT GPIOJ
#define AIR_NEG_EN_PIN GPIO_PIN_3

#define AIR_POS_EN_PORT GPIOB
#define AIR_POS_EN_PIN GPIO_PIN_0

/*============================================================================*/
/* AMS                                                                        */
/*============================================================================*/

#define AMS_OK_PORT GPIOA
#define AMS_OK_PIN GPIO_PIN_5

#define AMS_LATCH_RESET_PORT GPIOH
#define AMS_LATCH_RESET_PIN GPIO_PIN_4


/*============================================================================*/
/* IMD                                                                        */
/*============================================================================*/

#define IMD_OK_PORT GPIOJ
#define IMD_OK_PIN GPIO_PIN_2

#define IMD_M_PORT GPIOA
#define IMD_M_PIN GPIO_PIN_3

#define IMD_LATCH_RESET_PORT GPIOH
#define IMD_LATCH_RESET_PIN GPIO_PIN_3

/*============================================================================*/
/* DS18B20                                                                    */
/*============================================================================*/
extern UART_HandleTypeDef huart4;
#define DS18B20_UART_HANDLE (&huart4)

#define DS18B20_VDD_EN_PORT GPIOD
#define DS18B20_VDD_EN_PIN GPIO_PIN_12

#define DS18B20_WEAK_PULL_UP_PORT GPIOI
#define DS18B20_WEAK_PULL_UP_PIN GPIO_PIN_0


/*============================================================================*/
/* Debug LEDs                                                                 */
/*============================================================================*/

#define LED_1_PORT GPIOB
#define LED_1_PIN GPIO_PIN_8

#define LED_2_PORT GPIOB
#define LED_2_PIN GPIO_PIN_9

#define LED_3_PORT GPIOE
#define LED_3_PIN GPIO_PIN_3

#endif /* CONFIG_ACU_LV_CONFIG_PINOUT_H_ */
