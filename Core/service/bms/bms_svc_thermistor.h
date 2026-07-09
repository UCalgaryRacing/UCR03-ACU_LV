#ifndef BMS_SVC_THERMISTOR_H
#define BMS_SVC_THERMISTOR_H

#include "acu_lv_drv_adbms6830_regs.h"
#include "com_typ_common.h"

void bms_svc_acquire_thermistor_temps(uint8_t mux_state);
void bms_svc_admbs_toggle_mux(adbms_gpo_pin_t pin);
bool bms_svc_check_temps();

#endif