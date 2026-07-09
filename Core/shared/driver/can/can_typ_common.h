#ifndef CAN_TYPE_COMMON_H
#define CAN_TYPE_COMMON_H

#include <stdint.h>

typedef enum
{
    CAN1 = 0,           
    CAN2,               
    can_CHANNEL_COUNT
} can_channel_id_t;


typedef struct
{
    can_channel_id_t channel;
    uint32_t id;
    uint8_t  dlc;
    uint8_t  data[64];

} can_msg_t;

#endif
