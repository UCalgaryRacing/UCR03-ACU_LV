/*
 * rco_data.c
 *
 *  Created on: Apr 21, 2026
 *      Author: clayd
 */


#include "rco_data.h"

static uint8_t g_reset_button = 0;

void rco_data_set_reset_button(uint8_t status)
{
    g_reset_button = status;
}

uint8_t rco_data_get_reset_button()
{
    return g_reset_button;
}
