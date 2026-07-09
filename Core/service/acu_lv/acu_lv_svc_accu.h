/*
 * acu_lv_svc_acu.h
 *
 *  Created on: Apr 7, 2026
 *      Author: clayd
 */

#ifndef SERVICE_ACU_LV_SVC_ACCU_H_
#define SERVICE_ACU_LV_SVC_ACCU_H_

#include "acu_lv_config.h"
#include "com_typ_common.h"

acu_lv_status_t acu_lv_svc_update_accu_voltage();
float acu_lv_svc_get_accu_voltage();
float acu_lv_svc_get_accu_min();
float acu_lv_svc_get_accu_max();
acu_lv_status_t acu_lv_svc_start_accu_filter();
acu_lv_status_t acu_lv_svc_stop_accu_filter();
status_t acu_lv_svc_check_accu_voltage();

#endif /* SERVICE_ACU_LV_SVC_ACCU_H_ */
