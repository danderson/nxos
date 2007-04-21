#ifndef __NXTOS_AVR_H__
#define __NXTOS_AVR_H__

#include "mytypes.h"

void avr_init();
void avr_1kHz_update();
void avr_set_motor(U32 n, int power_percent, int brake);
void avr_power_down();
U32 avr_buttons_get();
U32 avr_battery_voltage();
U32 avr_sensor_adc(U32 n);
void avr_set_input_power(U32 n, U32 power_type);

#endif
