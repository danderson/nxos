
#include "at91sam7s256.h"

#include "mytypes.h"
#include "interrupts.h"
#include "aic.h"
#include "systick.h"
#include "avr.h"
#include "twi.h"
#include "lcd.h"
#include "display.h"
#include "sound.h"
#include "usb.h"
#include "sensors.h"
#include "motors.h"
#include "i2c.h"
#include "radar.h"

#include "tests.h"

#define RADAR_SENSOR_SLOT 0

static void core_init() {
  aic_init();
  interrupts_enable();
  systick_init();
  sound_init();
  avr_init();
  motors_init();
  lcd_init();
  display_init();
  sensors_init();
  usb_init();
  i2c_init();

  /* Delay a little post-init, to let all the drivers settle down. */
  systick_wait_ms(100);
}

static void core_shutdown() {
  lcd_shutdown();
  usb_disable();
  avr_power_down();
}

void main() {
  core_init();

  //tests_usb_hardcore();
  //tests_usb();
  //tests_all();

  radar_init(RADAR_SENSOR_SLOT);
  systick_wait_ms(100);

  radar_test(RADAR_SENSOR_SLOT);
  systick_wait_ms(5000);

  /*
  U8 i;
  for (i=0 ; i<10 ; i++) {
    radar_test(RADAR_SENSOR_SLOT);
    display_string("Test ");
    display_uint(i);
    display_string("/10");
    systick_wait_ms(1000);
  }
  */

  core_shutdown();
}
