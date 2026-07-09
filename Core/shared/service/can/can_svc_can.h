#ifndef CAN_SVC_CAN_H_
#define CAN_SVC_CAN_H_


#include "can_typ_common.h" 
#include "com_typ_common.h" 
#include "can_config.h" 
#include "ucr_03.h"

#include "cmsis_os.h"

typedef void (*can_rx_handler_t)(can_msg_t *msg);

typedef struct
{
    uint32_t id;
    can_rx_handler_t handler;
} can_route_entry_t;

status_t com_svc_can_init(void);
status_t com_svc_can_start(uint8_t channel);

status_t com_svc_can_transmit(const can_msg_t *msg);
void com_svc_can_route(can_msg_t *msg);
status_t com_svc_can_register(uint32_t id, can_rx_handler_t handler);




#endif
