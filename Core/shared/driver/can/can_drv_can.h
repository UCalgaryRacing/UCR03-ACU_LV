#ifndef COM_DRV_CAN_H
#define COM_DRV_CAN_H

#include <stdint.h>
#include <stdbool.h>
#include "stm32h7xx_hal.h"
#include "can_config.h"
#include "can_config_pinout.h"
#include "com_typ_common.h"
#include "can_typ_common.h"


/*============================================================================*/
/* Public API                                                                 */
/*============================================================================*/



status_t can_drv_init(void);
status_t can_drv_start(const can_hw_t *hw);
status_t can_drv_transmit(can_msg_t *message);


#endif
