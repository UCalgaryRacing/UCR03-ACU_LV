/*
 * bms_types.h
 *
 * Core data structures for the BMS system.
 * Defines slave-level and pack-level data types.
 *
 *  Created on: Dec 8, 2025
 *      Author: esall
 */

#ifndef TYPES_BMS_TYPES_H_
#define TYPES_BMS_TYPES_H_

#include <stdint.h>
#include <stdbool.h>
#include "acu_lv_config.h"

/*============================================================================*/
/* Slave-Level Data (one ADBMS6830B IC)                                       */
/*============================================================================*/

/**
 * Cell voltage and temperature data from a single BMS slave (ADBMS6830B).
 *
 * Voltages are stored in volts (V).
 * Temperatures are stored in degrees Celsius (°C).
 */
typedef struct
{
    float cell_v[ADBMS_CELLS_PER_IC];       /**< Cell voltages in V */
    float temp_c[ADBMS_THERMS_PER_IC];      /**< Temperatures in °C */
} bms_slave_data_t;

/*============================================================================*/
/* Pack-Level Aggregates                                                      */
/*============================================================================*/



#endif /* TYPES_BMS_TYPES_H_ */
