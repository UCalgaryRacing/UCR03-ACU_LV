#include "can_config.h"
#include "can_config_pinout.h"
#include "can_typ_common.h"


static can_context_t can_1_context = {
    .started = false
};

static can_context_t can_2_context = {
    .started = false
};

//TODO maybe split hw and context / dma stuff into seperate structs
//also maybe just statically define g_started flags in source file, no sharing context like analog.c
const can_config_t can_config[can_CHANNEL_COUNT] =
{
    [CAN1] =
    {
        .enabled = true,
        .hw =
        {
            .can_handle = CAN_1_HANDLE,
            .context = &can_1_context,
        },

    },

    [CAN2] =
    {
        .enabled = true,
        .hw =
        {
            .can_handle = CAN_2_HANDLE,
            .context = &can_2_context,
        },

    }
};