#include "acu_data.h"

#include "com_typ_common.h"

#include "bms_svc_thermistor.h"
#include "acu_lv_svc_cell_voltage.h"
#include "acu_lv_svc_precharge.h"
#include "acu_lv_svc_accu.h"
#include "acu_lv_svc_imd.h"

void bms_svc_check_faults()
{
    acu_fault_t acu_fault;

    acu_fault.bms_fault = false;
    acu_fault.imd_fault = false;

    // if(acu_lv_svc_check_cell_voltage() || bms_svc_check_temps() || acu_lv_svc_check_precharge_timeout() || (acu_lv_svc_check_accu_voltage() != OK))
    if(acu_lv_svc_check_cell_voltage() != OK)
     {
         acu_fault.bms_fault = true;
     } 

    if (!acu_lv_svc_imd_ok())
    {
        acu_fault.imd_fault = true;
    } 

    acu_data_set_fault_status(&acu_fault);

    //TODO send CAN here?
}
