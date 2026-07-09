/*
 * acu_lv_svc_adbms6830.h
 *
 *  Created on: Apr 11, 2026
 *      Author: clayd
 */

#ifndef SERVICE_ACU_LV_ACU_LV_SVC_ADBMS6830_H_
#define SERVICE_ACU_LV_ACU_LV_SVC_ADBMS6830_H_

#include <stdint.h>


int voltage_acquisition_sample(void);
int temperature_acquisition_sample(void);

/*============================================================================*/
/* Initialization                                                             */
/*============================================================================*/

/**
 * @brief Initialize the BMS manager and all subsystems.
 *
 * Configures the ADBMS6830 slaves with application-specific settings
 * (CTH threshold, reference, GPO, etc.) and initializes the BMS data store.
 *
 * Must be called once before any sampling functions.
 *
 * @return 0 on success, negative error code on failure.
 */
int bms_manager_init(void);

/*============================================================================*/
/* Status and Diagnostics                                                     */
/*============================================================================*/

/**
 * @brief Check if the BMS manager is initialized.
 *
 * @return true if initialized and ready, false otherwise.
 */
bool bms_manager_is_ready(void);

/**
 * @brief Verify communication with all BMS slaves.
 *
 * Performs a write/read-back test of the configuration registers
 * to confirm all slaves in the daisy chain are responding.
 *
 * @return 0 if all slaves respond correctly, negative error code on failure.
 */
int bms_manager_verify_communication(void);

/**
 * @brief Reconfigure BMS slaves with current shadow register values.
 *
 * Useful after changing configuration parameters or recovering from
 * a communication error.
 *
 * @return 0 on success, negative error code on failure.
 */
int bms_manager_reconfigure(void);

/*============================================================================*/
/* Shadow Register Access (for voltage/temperature acquisition modules)       */
/*============================================================================*/

/**
 * @brief Acquire exclusive access to shadow registers.
 *
 * Must be called before modifying shadow registers.
 * Call bms_manager_shadow_unlock() when done.
 */
void bms_manager_shadow_lock(void);

/**
 * @brief Release shadow register access.
 */
void bms_manager_shadow_unlock(void);

/**
 * @brief Get pointer to shadow registers.
 *
 * Only valid while holding the shadow lock.
 *
 * @return Pointer to shadow register structure.
 */
adbms6830_shadow_t *bms_manager_get_shadow(void);


void vw_voltages();
#endif /* SERVICE_ACU_LV_ACU_LV_SVC_ADBMS6830_H_ */
