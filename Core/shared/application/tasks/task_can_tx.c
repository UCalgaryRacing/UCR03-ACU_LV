#include "can_config.h"
#include "com_typ_common.h"
#include "can_typ_common.h"
#include "cmsis_os.h"
#include "can_drv_can.h"

#include "acu_lv_drv_debug_led.h"

extern debug_led_t red_led;

extern osMessageQueueId_t Can1TxQueueHandle;
extern osMessageQueueId_t Can2TxQueueHandle;

const static uint32_t period = 10;
static uint32_t next_wake;

void task_can_tx_init()
{
    can_config_t can_1_config = can_config[CAN1];
	can_drv_start(&can_1_config.hw);


    next_wake = osKernelGetTickCount();
}


void task_can_tx_loop()
{
    next_wake += period;
    osDelayUntil(next_wake);
    can_msg_t msg;


    if (osMessageQueueGet(Can1TxQueueHandle, &msg, NULL, osWaitForever) == osOK)
    {
        can_drv_transmit(&msg);
        acu_lv_drv_toggle_led(&red_led);
    }
//
//    if (osMessageQueueGet(Can2TxQueueHandle, &msg, NULL, 0) == osOK)
//    {
//        can_drv_transmit(&msg);
//    }
}
