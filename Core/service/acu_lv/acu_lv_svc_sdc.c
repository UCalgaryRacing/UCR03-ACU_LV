/*
 * acu_lv_sdc.c
 *
 *  Created on: Apr 10, 2026
 *      Author: clayd
 */
#include "cmsis_os2.h"
#include "acu_lv_drv_sdc.h"
#include "acu_lv_svc_sdc.h"

extern acu_lv_sdc_t sdc;

void acu_lv_svc_sdc_init()
{
    acu_lv_drv_set_ams_latch(false);
    acu_lv_drv_set_imd_latch(false);
    acu_lv_drv_set_ams_ok(false);
}

void acu_lv_svc_reset_imd_latch()
{   
    acu_lv_drv_set_imd_latch(true);
    osDelay(100);
    acu_lv_drv_set_imd_latch(false);
}

void acu_lv_svc_reset_ams_latch()
{
    acu_lv_drv_set_ams_latch(true);
    osDelay(100);
    acu_lv_drv_set_ams_latch(false);
}

void acu_lv_svc_set_ams_ok()
{
    acu_lv_drv_set_ams_ok(true);

    acu_lv_svc_reset_ams_latch();
}

