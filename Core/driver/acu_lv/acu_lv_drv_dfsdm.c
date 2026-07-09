/*
 * acu_lv_drv_dfsdm.c
 *
 *  Created on: Apr 7, 2026
 *      Author: clayd
 */

#include "acu_lv_drv_dfsdm.h"

acu_lv_status_t acu_lv_start_DFSDM_filter(DFSDM_Filter_HandleTypeDef * filter, int32_t * buffer)
{   
    if((filter== NULL) || (buffer == NULL))
    {
        return ACU_LV_ERROR;
    }
    else
    {
        //length of all filter conversions should be one because we only need one reading per filter
        //becuase filter does the oversampling and averaging
        if(HAL_DFSDM_FilterRegularStart_DMA(filter,buffer,1) != HAL_OK)
        {
            return ACU_LV_ERROR;
        }
    }
    return ACU_LV_OK;
}

acu_lv_status_t acu_lv_stop_DFSDM_filter(DFSDM_Filter_HandleTypeDef * filter)
{   
    if(filter == NULL)
    {
        return ACU_LV_ERROR;
    }
    else
    {
        if(HAL_DFSDM_FilterRegularStop_DMA(filter) != HAL_OK)
        {
            return ACU_LV_ERROR;
        }
    }
    return ACU_LV_OK;
    
}