/**
 * acu_lv_drv_amc3306m05.h
 * 
 * Driver layer file for AMC3306M05 and AMC3336
 * 
 * AMC3306M05 --> +/-50mV adc used on shunt board
 * 
 * AMC3336 --> +/-1V adc used on hv for measuring ACCU and TS
 */

#ifndef DRIVERS_AMC33X6XX_H_
#define DRIVERS_AMC33X6XX_H_

#include "acu_lv_config.h"
#include "acu_lv_config_pinout.h"


static inline void acu_lv_start_DFSDM_filter(DFSDM_Filter_HandleTypeDef * filter, int32_t * buffer)
{   
    //length of all filter conversions should be one because we only need one reading per filter
    HAL_DFSDM_FilterRegularStart_DMA(filter,buffer,1);
}

static inline void acu_lv_stop_DFSDM_filter(DFSDM_Filter_HandleTypeDef * filter)
{
    HAL_DFSDM_FilterRegularStop_DMA(filter);
}

#endif // DRIVERS_AMC33X6XX_H_