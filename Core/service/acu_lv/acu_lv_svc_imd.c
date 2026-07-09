/*
 * acu_lv_svc_imd.c
 *
 *  Created on: Apr 10, 2026
 *      Author: clayd
 */

#include "acu_lv_svc_imd.h"

bool acu_lv_svc_imd_ok()
{
    if(acu_lv_drv_get_imd_state() == ACU_LV_IMD_OK)
    {
        return true;
    }
    else
    {
        return false;
    }
}
