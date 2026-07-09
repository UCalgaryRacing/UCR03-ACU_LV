/*
 * acu_lv_svc_precharge.h
 *
 *  Created on: Apr 10, 2026
 *      Author: clayd
 */

#ifndef SERVICE_ACU_LV_SVC_PRECHARGE_H_
#define SERVICE_ACU_LV_SVC_PRECHARGE_H_

#include <stdbool.h>
#include "com_typ_common.h"

bool acu_lv_svc_check_precharge_done();
bool acu_lv_svc_check_precharge_timeout();
void acu_lv_svc_start_precharge_timer();
void acu_lv_svc_stop_precharge_timer();

#endif /* SERVICE_ACU_LV_SVC_PRECHARGE_H_ */
