#ifndef DATA_ACU_DATA_H_
#define DATA_ACU_DATA_H_

#include <stdint.h>
#include <stdbool.h>
#include "com_typ_common.h"

/*============================================================================*/
/* Accumulator LV Data Structure                                              */
/*============================================================================*/

typedef struct
{
    float battery_current;
    float battery_voltage;
    float battery_power;
    float ts_voltage;
    bool ts_active;
} acu_aculv_data_t;

typedef struct
{
    bool bms_fault;
    bool imd_fault;
} acu_fault_t;

/**
 * Voltage statistics for the entire battery pack.
 */
typedef struct
{
    float   cell_min_v;         /**< Minimum cell voltage in V */
    float   cell_max_v;         /**< Maximum cell voltage in V */
    float   cell_avg_v;         /**< Average cell voltage in V */
    float   pack_v;             /**< Total pack voltage in V */
    uint8_t cell_min_slave;     /**< Slave index containing min cell */
    uint8_t cell_min_idx;       /**< Cell index within slave for min */
    uint8_t cell_max_slave;     /**< Slave index containing max cell */
    uint8_t cell_max_idx;       /**< Cell index within slave for max */
} bms_voltage_stats_t;

/**
 * Temperature statistics for the entire battery pack.
 */
typedef struct
{
    float   temp_min_c;         /**< Minimum temperature in °C */
    float   temp_max_c;         /**< Maximum temperature in °C */
    float   temp_avg_c;         /**< Average temperature in °C */
    uint8_t temp_min_slave;     /**< Slave index containing min temp */
    uint8_t temp_min_idx;       /**< Thermistor index within slave for min */
    uint8_t temp_max_slave;     /**< Slave index containing max temp */
    uint8_t temp_max_idx;       /**< Thermistor index within slave for max */
} bms_temp_stats_t;

/*============================================================================*/
/* Initialization                                                             */
/*============================================================================*/

status_t acu_data_init(void);
status_t acu_fault_init(void);

/*============================================================================*/
/* Setters                                                                    */
/*============================================================================*/

status_t acu_data_set_aculv(const acu_aculv_data_t *aculv_data);
status_t acu_data_set_fault_status(const acu_fault_t *acu_fault_status);

void acu_data_set_aculv_battery_current(float current);
void acu_data_set_aculv_battery_power(float power);
void acu_data_set_aculv_battery_voltage(float voltage);
void acu_data_set_aculv_ts_active(bool ts_active);
void acu_data_set_aculv_ts_voltage(float ts_voltage);

void acu_data_set_bms_fault_status(bool fault_status);
void acu_data_set_imd_fault_status(bool fault_status);

/*============================================================================*/
/* Getters                                                                    */
/*============================================================================*/

float acu_data_get_aculv_battery_voltage(void);
float acu_data_get_aculv_battery_current(void);
float acu_data_get_aculv_battery_power(void);
float acu_data_get_aculv_ts_voltage(void);
bool acu_data_get_aculv_ts_active(void);

bool acu_data_get_bms_fault_status(void);
bool acu_data_get_imd_fault_status(void);


status_t acu_data_get_fault_status(acu_fault_t *acu_fault_status);


#endif /* DATA_ACU_DATA_H_ */
