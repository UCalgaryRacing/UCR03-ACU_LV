/*
 * acu_lv_svc_ts.c
 *
 *  Created on: Apr 7, 2026
 *      Author: clayd
 */

#include "acu_lv_config.h" 
#include "acu_lv_svc_ts.h"
#include "acu_lv_drv_dfsdm.h"
#include "acu_lv_svc_accu.h"
#include "acu_data.h"

extern acu_lv_pack_measurement_t ts_voltage;

acu_lv_status_t acu_lv_svc_update_ts_voltage()
{
    if(ts_voltage.hw.dma_started == true)
    {
        *ts_voltage.data = ((float)(*ts_voltage.raw_data >> 8))* ts_voltage.settings.scaling_factor;
        acu_data_set_aculv_ts_voltage(*ts_voltage.data);
    }
    else
    {
        return ACU_LV_ERROR;
    }
    return ACU_LV_OK;
}

float acu_lv_svc_get_ts_voltage()
{
    return * ts_voltage.data;
}

acu_lv_status_t acu_lv_svc_start_ts_filter()
{
    acu_lv_status_t status;
    status = acu_lv_start_DFSDM_filter(ts_voltage.hw.filter_handle, ts_voltage.raw_data);
    if(status == ACU_LV_OK)
    {
    	ts_voltage.hw.dma_started = true;
    }
    return status;
}

acu_lv_status_t acu_lv_svc_stop_ts_filter()
{   
    acu_lv_status_t status;
    status = acu_lv_stop_DFSDM_filter(ts_voltage.hw.filter_handle);
    if(status == ACU_LV_OK)
    {
    	ts_voltage.hw.dma_started = false;
    }
    return status;
}

status_t acu_lv_svc_check_ts_voltage()
{
    // check if ts voltage is within 10% of accumulator
    // should tractive be allowed to be greater that accu?
    if(acu_lv_svc_get_ts_voltage() > acu_lv_svc_get_accu_voltage() * 1.1)
    {
        return ERROR_GENERAL;
    } 
    else if(acu_lv_svc_get_ts_voltage() < acu_lv_svc_get_accu_voltage() * 0.9)
    {
        return ERROR_GENERAL;
    }

    return OK;
}