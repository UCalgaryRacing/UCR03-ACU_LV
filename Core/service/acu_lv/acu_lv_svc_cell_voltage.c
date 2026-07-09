/*
 * acu_lv_svc_cell_voltage.c
 *
 *  Created on: Apr 14, 2026
 *      Author: clayd
 */

#include "acu_lv_svc_cell_voltage.h"
#include "acu_lv_config.h"
#include "acu_data.h"

extern bms_voltage_stats_t voltage_stats;

/* Private functions*/
static float get_lowest_cell_voltage()
{
    return voltage_stats.cell_min_v;
}
static float get_highest_cell_voltage()
{
   return voltage_stats.cell_max_v;
}

bool acu_lv_svc_check_cell_voltage()
{
    if((get_highest_cell_voltage() > ACU_LV_CELL_MAXIMUM_VOLTAGE) || (get_lowest_cell_voltage() < ACU_LV_CELL_MINIMUM_VOLTAGE))
    {
        return ERROR_GENERAL;
    }

    return OK;
}
