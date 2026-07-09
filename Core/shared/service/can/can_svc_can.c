#include "can_svc_can.h"


extern osMessageQueueId_t Can1TxQueueHandle;
extern osMessageQueueId_t Can2TxQueueHandle;

status_t com_svc_can_init(void)
{
    //TODO start can here
    return OK;
}


status_t com_svc_can_transmit(const can_msg_t *msg)
{
    if (msg == NULL)
        return ERROR_INVALID_PARAM;

    if (msg->channel == CAN1)
    {
        if (osMessageQueuePut(Can1TxQueueHandle, msg, 0, 0) != osOK)
        {
            return ERROR_BUSY;
        }
    }
    
    if (msg->channel == CAN2)
    {
        if (osMessageQueuePut(Can2TxQueueHandle, msg, 0, 0) != osOK)
        {
            return ERROR_BUSY;
        }
    }


    return OK;
}



#define MAX_ROUTES 32

static can_route_entry_t route_table[MAX_ROUTES];
static uint32_t route_count = 0;

status_t com_svc_can_register(uint32_t id, can_rx_handler_t handler)
{
    if (route_count >= MAX_ROUTES)
        return ERROR;

    route_table[route_count++] = (can_route_entry_t){id, handler};

    return OK;
}

//ROUTING TABLE

void com_svc_can_route(can_msg_t *msg)
{
    for (uint32_t i = 0; i < route_count; i++)
    {
        if (route_table[i].id == msg->id)
        {
            route_table[i].handler(msg);
            return;
        }
    }
}








