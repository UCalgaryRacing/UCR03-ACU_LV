/*
 * acu_lv_svc_shunt.c
 *
 *  Created on: Apr 7, 2026
 *      Author: clayd
 */

#include "acu_lv_svc_shunt.h"
#include "acu_lv_config.h"
#include "acu_lv_drv_dfsdm.h"

extern acu_lv_pack_measurement_t pack_current;

acu_lv_status_t acu_lv_svc_update_shunt()
{
    if(pack_current.hw.dma_started == true)
    {
        *pack_current.data = ((float)(*pack_current.raw_data >> 8)) * pack_current.settings.scaling_factor;
    }
    else
    {
        return ACU_LV_ERROR;
    }
    return ACU_LV_OK;
}

float acu_lv_svc_get_shunt_current()
{
    return *pack_current.data;
}

acu_lv_status_t acu_lv_svc_start_shunt_filter()
{
    acu_lv_status_t status;
    status = acu_lv_start_DFSDM_filter(pack_current.hw.filter_handle, pack_current.raw_data);
    if(status == ACU_LV_OK)
    {
    	pack_current.hw.dma_started = true;
    }
    return status;
}

acu_lv_status_t acu_lv_svc_stop_shunt_filter()
{
    acu_lv_status_t status;
    status = acu_lv_stop_DFSDM_filter(pack_current.hw.filter_handle);
    if(status == ACU_LV_OK)
    {
    	pack_current.hw.dma_started = true;
    }
    return status;
}

status_t acu_lv_svc_check_shunt_current()
{
    if(acu_lv_svc_get_shunt_current() > CURRENT_MAX_DISCHARGE_A)
    {
        return ERROR_GENERAL;
    }
    return OK;
}
