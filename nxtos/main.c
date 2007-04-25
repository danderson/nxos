
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
#include "motors.h"

#include "tests.h"

static void core_init() {
  aic_init();
  interrupts_enable();
  systick_init();
  sound_init();
  avr_init();
  motors_init();
  lcd_init();
  display_init();
  //usb_init();

  /* Delay a little post-init, to let all the drivers settle down. */
  systick_wait_ms(100);
}

static void core_shutdown() {
  lcd_shutdown();
  avr_power_down();
}

void
main()
{
  core_init();

  core_shutdown();
}
