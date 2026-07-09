/*
 * acu_lv_svc_acu.c
 *
 *  Created on: Apr 7, 2026
 *      Author: clayd
 */

#include "acu_lv_config.h"
#include "acu_lv_svc_accu.h"
#include "acu_lv_drv_dfsdm.h"
#include "acu_data.h"

extern acu_lv_pack_measurement_t accu_voltage;

acu_lv_status_t acu_lv_svc_update_accu_voltage()
{
    if(accu_voltage.hw.dma_started == true)
    {
        //*accu_voltage.data = ((float)(*accu_voltage.raw_data >> 8))* accu_voltage.settings.scaling_factor;
    	*accu_voltage.data = ((*accu_voltage.raw_data))* accu_voltage.settings.scaling_factor;
    	acu_data_set_aculv_battery_voltage(*accu_voltage.data);
    }
    else
    {
        return ACU_LV_ERROR;
    }
    return ACU_LV_OK;
}

float acu_lv_svc_get_accu_voltage()
{
    return *accu_voltage.data;
}

float acu_lv_svc_get_accu_min()
{
    return accu_voltage.settings.valid_min;
}

float acu_lv_svc_get_accu_max()
{
    return accu_voltage.settings.valid_max;
} 

acu_lv_status_t acu_lv_svc_start_accu_filter()
{
    acu_lv_status_t status;
    status = acu_lv_start_DFSDM_filter(accu_voltage.hw.filter_handle, accu_voltage.raw_data);
    if(status == ACU_LV_OK)
    {
    	accu_voltage.hw.dma_started = true;
    }
    return status;
}

acu_lv_status_t acu_lv_svc_stop_accu_filter()
{
    acu_lv_status_t status;
    status = acu_lv_stop_DFSDM_filter(accu_voltage.hw.filter_handle);

    if(status == ACU_LV_OK)
    {
    	accu_voltage.hw.dma_started = false;
    }
    return status;
}

status_t acu_lv_svc_check_accu_voltage()
{
    // check if accumulator voltage is within allowed range
    if(acu_lv_svc_get_accu_voltage() > acu_lv_svc_get_accu_max())
    {
        return ERROR_GENERAL;
    }
    else if(acu_lv_svc_get_accu_voltage() < acu_lv_svc_get_accu_min())
    {
        return ERROR_GENERAL;
    }
    return OK;
}
