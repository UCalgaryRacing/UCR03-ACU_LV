#include "can_config.h"
#include "com_typ_common.h"
#include "ucr_03.h"
#include "can_typ_common.h"
#include "can_svc_can.h"
#include "rco_data.h"
#include "rco_svc_can_route.h"

static bool g_initialized = false;

void rco_svc_can_rx_rear_controller_data(can_msg_t *msg);

status_t rco_svc_can_route_init(void)
{
    //TODO start CAN here
    com_svc_can_register(REAR_CONTROL_CAN_ID, rco_svc_can_rx_rear_controller_data);
    g_initialized = true;
    return OK;
}

void rco_svc_can_rx_rear_controller_data(can_msg_t *msg)
{
    //received by front controller
    struct ucr_03_rear_control_t rear_control_can_msg;
    ucr_03_rear_control_unpack(&rear_control_can_msg, msg->data, msg->dlc);

    //We only care about reset button
    rco_data_set_reset_button(rear_control_can_msg.reset_button);
}
