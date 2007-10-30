#ifndef __NXOS_TESTS_H__
#define __NXOS_TESTS_H__

#define RADAR_SENSOR_SLOT 0

void tests_motor();
void tests_sound();
void tests_display();
void tests_sysinfo();
void tests_sensors();
void tests_tachy();
void tests_usb();
void tests_usb_hardcore();
void tests_radar(U8 sensor);

void tests_all();

#endif
