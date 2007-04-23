
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

#include "tests.h"

static void core_init() {
  aic_init();
  interrupts_enable();
  systick_init();
  sound_init();
  avr_init();
  lcd_init();
  display_init();
}

static void core_shutdown() {
  lcd_shutdown();
  avr_power_down();
}

void
main()
{
  core_init();
  tests_motor();
  tests_sound();
  tests_display();
  tests_time();
  core_shutdown();
}
