/*
 * acu_lv_app_state_manager.h
 *
 *  Created on: Mar 23, 2026
 *      Author: f-dunnwolbaum
 */

#ifndef APPLICATION_ACU_LV_APP_STATE_MANAGER_H_
#define APPLICATION_ACU_LV_APP_STATE_MANAGER_H_

typedef enum
{
    ACU_LV_APP_STATE_STARTUP = 0U,
    ACU_LV_APP_STATE_IDLE,
    ACU_LV_APP_STATE_PRECHARGE,
    ACU_LV_APP_STATE_ACTIVE,
    ACU_LV_APP_STATE_FAULT,
    ACU_LV_APP_STATE_CHARGING,
    ACU_LV_APP_STATE_BALANCING
} acu_lv_app_state_t;

// public function prototype
void acu_lv_app_state_machine_init();
void acu_lv_app_state_machine_step();




#endif /* APPLICATION_ACU_LV_APP_STATE_MANAGER_H_ */
