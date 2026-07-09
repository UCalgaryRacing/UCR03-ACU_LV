#include "can_drv_can.h"
#include "cmsis_os2.h"

static bool g_driver_initialized = false;

static uint8_t fdcan_dlc_to_bytes(uint32_t dlc);
static uint32_t dlc_to_fdcan_dlc(uint8_t dlc);
static const can_hw_t *can_drv_get_hw_from_handle(FDCAN_HandleTypeDef *hfdcan);
static void can_drv_isr_handler(FDCAN_HandleTypeDef *hfdcan);
static status_t can_drv_receive(const can_hw_t *hw, can_msg_t *message);


/*============================================================================*/
/* Private Initialization Functions                                           */
/*============================================================================*/

status_t can_drv_init()
{

    //TODO add guards similar to analog to avoid reinitialization

    if (g_driver_initialized)
    {
        return OK;
    }

    g_driver_initialized = true;

    return OK;
}

/*============================================================================*/
/* Private Start/Stop Functions                                               */
/*============================================================================*/

status_t can_drv_start(const can_hw_t *hw)
{
    if (hw == NULL)
    {
        return ERROR_INVALID_PARAM;
    }
    
    if (hw->context->started == false)
    {
        if (HAL_FDCAN_Start(hw->can_handle) != HAL_OK)
        {
            return ERROR_GENERAL;
        }

        if (HAL_FDCAN_ActivateNotification(hw->can_handle, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0) != HAL_OK)
        {
            return ERROR_GENERAL;
        }

        hw->context->started = true;
    }

    return OK;
}

/*============================================================================*/
/* Filter                                                                     */
/*============================================================================*/

//TODO add filter stuff to config
status_t can_drv_configure_filter(const can_hw_t *hw)
{
    FDCAN_FilterTypeDef filter;

    filter.IdType = FDCAN_STANDARD_ID;
    filter.FilterIndex = 0;
    filter.FilterType = FDCAN_FILTER_MASK;
    filter.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
    //TODO get this filter from somewhere
    filter.FilterID1 =  0x201;
    filter.FilterID2 = 0x7FF;

    if (HAL_FDCAN_ConfigFilter(hw->can_handle, &filter) != HAL_OK)
    {
        return ERROR_GENERAL;
    }

    return OK;
}


/*============================================================================*/
/* Transmit                                                                   */
/*============================================================================*/


status_t can_drv_transmit(can_msg_t *message)
{
    if (message == NULL)
    {
        return ERROR_INVALID_PARAM;
    }

    const can_hw_t *hw = &can_config[message->channel].hw;

    FDCAN_TxHeaderTypeDef tx_header;

    tx_header.Identifier = message->id;
    tx_header.IdType = FDCAN_STANDARD_ID;
    tx_header.TxFrameType = FDCAN_DATA_FRAME;
    tx_header.DataLength = dlc_to_fdcan_dlc(message->dlc);
    tx_header.ErrorStateIndicator = FDCAN_ESI_ACTIVE;

    if (hw->can_handle == CAN_1_HANDLE)
    {
        tx_header.BitRateSwitch = FDCAN_BRS_ON;
        tx_header.FDFormat =  FDCAN_FD_CAN;
    }
    else if(hw->can_handle == CAN_2_HANDLE)
    {
        tx_header.BitRateSwitch = FDCAN_BRS_ON;
        tx_header.FDFormat =  FDCAN_FD_CAN;
    }

    tx_header.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    tx_header.MessageMarker = 0;

    if (HAL_FDCAN_AddMessageToTxFifoQ(hw->can_handle, &tx_header, (uint8_t *)message->data) != HAL_OK)
    {
        return ERROR_BUSY;
    }

    return OK;
}


/*============================================================================*/
/* Receive                                                                    */
/*============================================================================*/



void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
    if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) == 0)
    {
        return;
    }

    can_drv_isr_handler(hfdcan);
}

void HAL_FDCAN_RxFifo1Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo1ITs)
{
    if ((RxFifo1ITs & FDCAN_IT_RX_FIFO1_NEW_MESSAGE) == 0)
    {
        return;
    }

    can_drv_isr_handler(hfdcan);
}




static void can_drv_isr_handler(FDCAN_HandleTypeDef *hfdcan)
{
    const can_hw_t *hw = can_drv_get_hw_from_handle(hfdcan);

    if (hw == NULL)
    {
        return;
    }

    can_msg_t message;

    if (can_drv_receive(hw, &message) == OK)
    {

        if (hw->can_handle == CAN_1_HANDLE)
        {
            extern osMessageQueueId_t Can1RxQueueHandle;
            
            osMessageQueuePut(Can1RxQueueHandle, &message, 0, 0);
        }

        // if (hw->can_handle == CAN_2_HANDLE)
        // {
        //     {
        //     extern osMessageQueueId_t Can2RxQueueHandle;
            
        //     osMessageQueuePut(Can2RxQueueHandle, &message, 0, 0);
        //     }
        // }
    }
}

static status_t can_drv_receive(const can_hw_t *hw, can_msg_t *message)
{
    FDCAN_RxHeaderTypeDef rx_header;

    if (hw == NULL || message == NULL)
    {
        return ERROR_INVALID_PARAM;
    }

    if (HAL_FDCAN_GetRxMessage(hw->can_handle, FDCAN_RX_FIFO0, &rx_header, message->data) != HAL_OK)
    {
        return ERROR_GENERAL;
    }

    message->dlc = fdcan_dlc_to_bytes(rx_header.DataLength);

    message->id = rx_header.Identifier;

    return OK;
}


/*============================================================================*/
/* Utilities                                                                  */
/*============================================================================*/


static uint8_t fdcan_dlc_to_bytes(uint32_t dlc)
{
    switch (dlc)
    {
        case FDCAN_DLC_BYTES_0: return 0;
        case FDCAN_DLC_BYTES_1: return 1;
        case FDCAN_DLC_BYTES_2: return 2;
        case FDCAN_DLC_BYTES_3: return 3;
        case FDCAN_DLC_BYTES_4: return 4;
        case FDCAN_DLC_BYTES_5: return 5;
        case FDCAN_DLC_BYTES_6: return 6;
        case FDCAN_DLC_BYTES_7: return 7;
        case FDCAN_DLC_BYTES_12: return 12;
        case FDCAN_DLC_BYTES_16: return 16;
        case FDCAN_DLC_BYTES_20: return 20;
        case FDCAN_DLC_BYTES_24: return 24;
        case FDCAN_DLC_BYTES_32: return 32;
        case FDCAN_DLC_BYTES_48: return 48;
        case FDCAN_DLC_BYTES_64: return 64;
        default: return 8;
    }
}


static uint32_t dlc_to_fdcan_dlc(uint8_t dlc)
{
    switch (dlc)
    {
        case 0:  return FDCAN_DLC_BYTES_0;
        case 1:  return FDCAN_DLC_BYTES_1;
        case 2:  return FDCAN_DLC_BYTES_2;
        case 3:  return FDCAN_DLC_BYTES_3;
        case 4:  return FDCAN_DLC_BYTES_4;
        case 5:  return FDCAN_DLC_BYTES_5;
        case 6:  return FDCAN_DLC_BYTES_6;
        case 7:  return FDCAN_DLC_BYTES_7;
        case 8:  return FDCAN_DLC_BYTES_8;
        case 12:  return FDCAN_DLC_BYTES_12;
        case 16: return FDCAN_DLC_BYTES_16;
        case 20: return FDCAN_DLC_BYTES_20;
        case 24: return FDCAN_DLC_BYTES_24;
        case 32: return FDCAN_DLC_BYTES_32;
        case 48: return FDCAN_DLC_BYTES_48;
        case 64: return FDCAN_DLC_BYTES_64;
        default: return FDCAN_DLC_BYTES_8;
    }
}


static const can_hw_t *can_drv_get_hw_from_handle(FDCAN_HandleTypeDef *hfdcan)
{
    for (uint8_t i = 0; i < can_CHANNEL_COUNT; i++)
    {
        if (can_config[i].hw.can_handle == hfdcan)
        {
            return &can_config[i].hw;
        }
    }

    return NULL;
}
