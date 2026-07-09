/*
 * acu_lv_app_stat_manager.c
 *
 *  Created on: Apr 8, 2026
 *      Author: clayd
 */

#include "acu_lv_app_state_manager.h"

#include "cmsis_os2.h"

#include "acu_lv_svc_imd.h"
#include "acu_lv_svc_air.h"
#include "acu_lv_svc_precharge.h"
#include "acu_lv_svc_sdc.h"
#include "acu_lv_svc_cell_voltage.h"
#include "acu_lv_svc_ts.h"
#include "acu_lv_svc_accu.h"
#include "acu_lv_svc_shunt.h"
#include "bms_svc_thermistor.h"

#include "acu_lv_drv_air.h"
#include "acu_lv_drv_imd.h"
#include "acu_lv_drv_sdc.h"

#include "acu_data.h"
#include "rco_data.h"

/* Private Prototypes*/
static acu_lv_app_state_t handle_startup_state();
static acu_lv_app_state_t handle_idle_state();
static acu_lv_app_state_t handle_precharge_state();
static acu_lv_app_state_t handle_active_state();
static acu_lv_app_state_t handle_fault_state();
static acu_lv_app_state_t handle_charging_state();
static acu_lv_app_state_t handle_balencing_state();

static void state_entry(acu_lv_app_state_t state);
static void state_exit(acu_lv_app_state_t state);

static acu_lv_app_state_t g_current_state = ACU_LV_APP_STATE_STARTUP;
static acu_lv_app_state_t g_previous_state = ACU_LV_APP_STATE_STARTUP;


void acu_lv_app_state_machine_init()
{
    g_current_state = ACU_LV_APP_STATE_STARTUP;
    g_previous_state = ACU_LV_APP_STATE_STARTUP;
}

void acu_lv_app_state_machine_step()
{   
    acu_lv_app_state_t next_state;
    switch (g_current_state)
    {
    case ACU_LV_APP_STATE_STARTUP:
        next_state = handle_startup_state();
        break;
    
    case ACU_LV_APP_STATE_IDLE:
        next_state = handle_idle_state();
        break;
    
    case ACU_LV_APP_STATE_PRECHARGE:
        next_state = handle_precharge_state();
        break;

    case ACU_LV_APP_STATE_ACTIVE:
        next_state = handle_active_state();
        break;

    case ACU_LV_APP_STATE_FAULT:
        next_state = handle_fault_state();
        break;

    case ACU_LV_APP_STATE_CHARGING:
        next_state = handle_charging_state();
        break;

    case ACU_LV_APP_STATE_BALANCING:
        next_state = handle_balencing_state();
        break;

    default:
        next_state = ACU_LV_APP_STATE_STARTUP;
        break;
    }

    // state transition
    if(next_state != g_current_state)
    {   
        // do anything that needs to be done when leaving the current state
        state_exit(g_current_state);

        // actually do the state change
        g_previous_state = g_current_state;
        g_current_state = next_state;

        // do anything that needs to be done when entering new state
        state_entry(g_current_state);
    }
}

static acu_lv_app_state_t handle_startup_state()
{
    //delay before checking IMD because it takes forever, might need to change to allow for it to check before getting
    // datasheet says 2sec, delay slightly longer than that
    osDelay(2500);

    // check imd and that cell voltages are in range
    // add temperature checking here
    if(acu_data_get_imd_fault_status())
    {
        return ACU_LV_APP_STATE_FAULT;
    }

    return ACU_LV_APP_STATE_IDLE;
}

static acu_lv_app_state_t handle_idle_state()
{
    // start measuring ts and accu voltage, add logic to fast task
    // write the getter function for accu and ts voltage

    // check cell voltage and temps out of range

    // check state transition, start with error case
    // then move through to check for errors based on safety/priority
    // if((!acu_lv_svc_imd_ok()) || (acu_lv_svc_check_cell_voltage() != OK))
    // {
    //    return ACU_LV_APP_STATE_FAULT;
    // }
    // if sdc_reserve is less than 9V then stay in idle state
    //else 
     if((acu_lv_svc_sdc_reserve_good()))
     {
         return ACU_LV_APP_STATE_PRECHARGE;
     }

    // tssi enabled

    // transition to precharge once sdc reserve is 9V
    // fault transition if imd fault, cell voltage or temp out of range
    return ACU_LV_APP_STATE_IDLE;
}

static acu_lv_app_state_t handle_precharge_state()
{   
    //check IMD and cell voltages, add temp measurement
    // if((!acu_lv_svc_imd_ok()) || (acu_lv_svc_check_cell_voltage() != OK) || (acu_lv_svc_check_precharge_timeout() != OK))
    // {
    //    return ACU_LV_APP_STATE_FAULT;
    // }
    // monitor accu and ts voltage
    //else 
    if(acu_lv_svc_check_precharge_done()) 
    {
        return ACU_LV_APP_STATE_ACTIVE;
    }
    if(rco_data_get_reset_button() == 1)
    {
        return ACU_LV_APP_STATE_IDLE;
    }
    // monitor pack current
    // monitor sdc
    // tssi enabled

    //transition to active when ts and accu reach 90%
    //transition to fault if watchdog timeout
    return ACU_LV_APP_STATE_PRECHARGE;
}

static acu_lv_app_state_t handle_active_state()
{
    // monitor accu and ts voltage
    // monitor pack current
    // monitor cell voltage and temp
    // monitor sdc
    // monitor imd
    // tssi enabled
    if(!acu_lv_svc_sdc_reserve_good())
    {
        return ACU_LV_APP_STATE_IDLE;
    }
    // potentially get rid of this, if reset button is held it should drop sdc reserve and transition to idle without needing to check the button state here
    // if(rco_data_get_reset_button() == 1)
    // {
    //     return ACU_LV_APP_STATE_IDLE;
    // }

    //transition to charge if charging message recieved
    //fault transition if imd fault, cell voltage or temp out of range
    return ACU_LV_APP_STATE_ACTIVE;
}

static acu_lv_app_state_t handle_fault_state()
{   
    // monitor ts and accu voltage
    // monitor temps and voltage
    // monitor imd
    // disable logging

    // check for imd reset button to reset the latches and exit fault state
    // tssi enabled/ flash red
    return ACU_LV_APP_STATE_FAULT;
}

static acu_lv_app_state_t handle_charging_state()
{
    // monitoring cell temp and voltage
    // monitor accu and ts
    // tssi disabled
    // imd monitoring
    // current measurement disabled

    //transition to fault if voltage or temp out of range, charger comm lost,charger fault
    // transition to balance if balance command rx or any cell reaches max voltage
    return ACU_LV_APP_STATE_FAULT;
}

static acu_lv_app_state_t handle_balencing_state()
{
    //voltage and temp monitor
    // accu and ts voltage monitor
    //disabled ts current monitor
    // imd monitor
    //charging disabled

    //transition to fault if imd fault, voltage or temp out of range, charger fault
    //transition to charge once cells are within 10mV
    return ACU_LV_APP_STATE_FAULT;
}

static void state_entry(acu_lv_app_state_t state)
{   
    // no startup entry because the start up state should not be entered
    switch (state)
    {
    case ACU_LV_APP_STATE_IDLE:
        acu_lv_drv_open_air();
        break;
    case ACU_LV_APP_STATE_PRECHARGE:
        //close negative air when leaving idle
        acu_lv_drv_close_air_neg();
        acu_lv_svc_start_precharge_timer();
        break;
    case ACU_LV_APP_STATE_ACTIVE:
        acu_data_set_aculv_ts_active(true);
        break;
    case ACU_LV_APP_STATE_FAULT:
        // check what faulted and send tssi red signal
        acu_lv_drv_open_air();
        break;
    case ACU_LV_APP_STATE_CHARGING:
        /* code */
        break; 
    case ACU_LV_APP_STATE_BALANCING:
        /* code */
        break;    
    default:
        break;
    }
}

static void state_exit(acu_lv_app_state_t state)
{
    switch (state)
    {
    case ACU_LV_APP_STATE_STARTUP:
        acu_lv_svc_set_ams_ok();
        acu_lv_svc_reset_ams_latch();
        acu_lv_svc_reset_imd_latch(); // new, try also resetting imd latch
        break;
    case ACU_LV_APP_STATE_IDLE:
        // acu_lv_svc_set_ams_ok(); // try doing in both active and idle?
        // acu_lv_svc_reset_ams_latch();
        break;
    case ACU_LV_APP_STATE_PRECHARGE:
        //close positive air when leaving precharge
        acu_lv_svc_stop_precharge_timer();
        acu_lv_drv_close_air_pos();
        break;
    case ACU_LV_APP_STATE_ACTIVE:
        acu_data_set_aculv_ts_active(false);
        acu_lv_drv_open_air(); // may need to change once balancing and charging is added
        break;
    case ACU_LV_APP_STATE_FAULT:
        break;
    case ACU_LV_APP_STATE_CHARGING:
        /* code */
        break; 
    case ACU_LV_APP_STATE_BALANCING:
        /* code */
        break;    
    default:
        break;
    }
}
