
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

#define RADAR_SENSOR_SLOT 1

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

  display_clear();
  display_cursor_set_pos(0, 0);
  display_string(".start");
  display_end_line();

  i2c_init();
  radar_init(RADAR_SENSOR_SLOT);

  radar_test(RADAR_SENSOR_SLOT);

  display_string(".end");
  display_end_line();
  while(1);

  core_shutdown();
}
