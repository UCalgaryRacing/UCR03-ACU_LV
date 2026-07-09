/*
 * acu_lv_svc_ts.h
 *
 *  Created on: Apr 7, 2026
 *      Author: clayd
 */

#ifndef SERVICE_ACU_LV_SVC_TS_H_
#define SERVICE_ACU_LV_SVC_TS_H_

#include "acu_lv_config.h"
#include "com_typ_common.h"

acu_lv_status_t acu_lv_svc_update_ts_voltage();
float acu_lv_svc_get_ts_voltage();
acu_lv_status_t acu_lv_svc_start_ts_filter();
acu_lv_status_t acu_lv_svc_stop_ts_filter();
status_t acu_lv_svc_check_ts_voltage();

#endif /* SERVICE_ACU_LV_SVC_TS_H_ */
