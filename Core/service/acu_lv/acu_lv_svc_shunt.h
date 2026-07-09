/*
 * acu_lv_svc_shunt.h
 *
 *  Created on: Apr 7, 2026
 *      Author: clayd
 */

#ifndef SERVICE_ACU_LV_SVC_SHUNT_H_
#define SERVICE_ACU_LV_SVC_SHUNT_H_

#include "acu_lv_config.h"
#include "com_typ_common.h"

acu_lv_status_t acu_lv_svc_update_shunt();
float acu_lv_svc_get_shunt_current();
acu_lv_status_t acu_lv_svc_start_shunt_filter();
acu_lv_status_t acu_lv_svc_stop_shunt_filter();
status_t acu_lv_svc_check_shunt_current();

#endif /* SERVICE_ACU_LV_SVC_SHUNT_H_ */
