#include "cmsis_os2.h"
#include "acu_lv_drv_adbms6830.h"
#include "acu_lv_svc_adbms6830.h"
#include "acu_lv_drv_adbms6830_regs.h"
#include "bms_svc_thermistor.h"
#include "acu_data.h"
#include "acu_lv_config.h"
// void bms_svc_admbs_toggle_mux(uint16_t gpio);

/* Private Functions */
static float get_highest_temp();
static float get_lowest_temp();

/*============================================================================*/
/* Temperature Sampling                                                       */
/*============================================================================*/

static uint16_t raw_temps[ADBMS_NUM_SLAVES][ADBMS_THERMS_PER_IC];
static float processed_temps[ADBMS_NUM_SLAVES][ADBMS_THERMS_PER_IC];

extern bms_temp_stats_t temp_stats;

static float calculate_thermistor_temperature(float adc_voltage)
{
  //stole this shit from arduino forum!!!
  float steinhart;
  float resistance = 10000 * adc_voltage / (3 - adc_voltage);
  steinhart = resistance / 10000;     // (R/Ro)
  steinhart = log(steinhart);                  // ln(R/Ro)
  steinhart /= 3950;                   // 1/B * ln(R/Ro)
  steinhart += 1.0 / (25 + 273.15); // + (1/To)
  steinhart = 1.0 / steinhart;                 // Invert
  steinhart -= 273.15;

  return steinhart;

  
}

//TODO add this to adbms drivers
void bms_svc_admbs_toggle_mux(adbms_gpo_pin_t pin)
{
    adbms6830_shadow_t *shadow = bms_manager_get_shadow();

    //mux control connected to gpio1 which is bit 0 of byte 3
    shadow->cfga[3] ^= ADBMS_GPO1_MASK;

    adbms6830_write_cfga(shadow);

    adbms6830_read_cfga(shadow);
}



void bms_svc_acquire_thermistor_temps(uint8_t mux_state)
{   
    uint8_t mux_index = mux_state * 9;
    uint16_t count = 0;
    int result = adbms6830_start_gpio_adc();
    float therm_sum = 0.0f;
    // Wait for ADC conversion to complete (~4ms)
    osDelay(5);

    result = adbms6830_read_gpio_voltages_raw(raw_temps,mux_state);

    for (int slave = 0; slave < ADBMS_NUM_SLAVES; slave++)
    {
        for (int thermistor = mux_index; thermistor < mux_index + 9; thermistor++)
        {   
            float thermistor_voltage = adbms6830_adc_to_volts(raw_temps[slave][thermistor]);
            processed_temps[slave][thermistor] = calculate_thermistor_temperature(thermistor_voltage);

            if(processed_temps[slave][thermistor] > temp_stats.temp_max_c)
            {
                temp_stats.temp_max_c = processed_temps[slave][thermistor];
                temp_stats.temp_max_idx = thermistor;
                temp_stats.temp_max_slave = slave;
            }
            else if (processed_temps[slave][thermistor] < -50.0f)
            {
               processed_temps[slave][thermistor] = temp_stats.temp_avg_c;
            }
            else if (processed_temps[slave][thermistor] > 1000.0f)
            {
                processed_temps[slave][thermistor] = temp_stats.temp_avg_c;
            }
            else if(processed_temps[slave][thermistor] < temp_stats.temp_min_c)
            {
                temp_stats.temp_min_c = processed_temps[slave][thermistor];
                temp_stats.temp_min_idx = thermistor;
                temp_stats.temp_max_slave = slave;
            }else
            {
                therm_sum += processed_temps[slave][thermistor];
                count++;
            }
            
        }

    }
    temp_stats.temp_avg_c = therm_sum / (float)count;
}

bool bms_svc_check_temps()
{
    if((get_lowest_temp() < ACULV_CELL_MIN_TEMPERATURE) || (get_highest_temp() > ACULV_CELL_MAX_TEMPERATURE))
    {
        return false;
    }
    return true;
}

static float get_highest_temp()
{   
    return temp_stats.temp_max_c;
}

static float get_lowest_temp()
{
    return temp_stats.temp_min_c;
}
