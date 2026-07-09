/*
 * acu_lv_svc_air.c
 *
 *  Created on: Apr 10, 2026
 *      Author: clayd
 */

#include "acu_lv_svc_air.h"
#include "acu_lv_drv_air.h"

extern acu_lv_air_t air;

float acu_lv_svc_get_sdc_reserve()
{
    return air.sdc_reserve;
}

bool acu_lv_svc_sdc_reserve_good()
{
    if(acu_lv_svc_get_sdc_reserve() >= 9.0f)
    {
        return true;
    }
    else
    {
        return false;
    }
}
