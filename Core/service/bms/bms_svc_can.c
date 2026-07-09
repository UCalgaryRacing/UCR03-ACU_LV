#include "can_svc_can.h"
#include "acu_data.h"


void bms_svc_can_tx_acu_fault_data()
{
    can_msg_t msg;
    msg.channel = CAN1;
	msg.id = ACCUMULATOR_FAULT_CAN_ID;
	msg.dlc = 1;

    acu_fault_t acu_fault;
    acu_data_get_fault_status(&acu_fault);

    struct ucr_03_accumulator_fault_t accumulator_fault_struct;
    accumulator_fault_struct.bms_fault = acu_fault.bms_fault;
    accumulator_fault_struct.imd_fault = acu_fault.imd_fault;
    ucr_03_accumulator_fault_pack(msg.data, &accumulator_fault_struct, msg.dlc);

    com_svc_can_transmit(&msg);
}

void bms_svc_can_tx_acu_data()
{
    can_msg_t msg;
    msg.channel = CAN1;
	msg.id = ACCUMULATOR_DATA_CAN_ID;
	msg.dlc = 20;

    acu_aculv_data_t acu_data;
    acu_data.battery_current = acu_data_get_aculv_battery_current();
    acu_data.battery_power = acu_data_get_aculv_battery_power();
    acu_data.battery_voltage = acu_data_get_aculv_battery_voltage();
    acu_data.ts_active = acu_data_get_aculv_ts_active();
    acu_data.ts_voltage = acu_data_get_aculv_ts_voltage();

    struct ucr_03_accumulator_data_t accumulator_data_struct;
    accumulator_data_struct.battery_current = acu_data.battery_current;
    accumulator_data_struct.battery_power = acu_data.battery_power;
    accumulator_data_struct.battery_voltage = acu_data.battery_voltage;
    accumulator_data_struct.ts_active = acu_data.ts_active;
    accumulator_data_struct.ts_voltage = acu_data.ts_voltage;

    ucr_03_accumulator_data_pack(msg.data, &accumulator_data_struct, msg.dlc);

    com_svc_can_transmit(&msg);
}