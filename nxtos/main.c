
#include "mytypes.h"
#include "interrupts.h"
#include "aic.h"
#include "at91sam7s256.h"
#include "systick.h"
#include "avr.h"
#include "twi.h"
#include "lcd.h"
#include "display.h"

#include "sound.h"

#include "inutil.h"

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

void alive() {
  systick_wait_ms(883);
  sound_freq(1500, 100);
}

void more_test() {
  int i;
  int t;

  sound_freq(1000, 100);
  systick_wait_ms(50);
  sound_freq(2000, 100);

  for (i=0; i<20; i++) {
    t = systick_get_ms();
    display_clear();
    display_cursor_set_pos(0, 0);
    display_string("  TX52 - NxtOS\n\n"
                   "0123456789ABCDEF\n"
                   "\n"
                   "Time (ms) : ");
    display_uint(t);
    display_string("\nTime (hex): ");
    display_hex(t);
    display_refresh();
    alive();
  }

  sound_freq(2000, 100);
  systick_wait_ms(50);
  sound_freq(1000, 100);
  systick_wait_ms(500);
}

void
core_test() {
  avr_set_motor(0, 80, 0);
  sound_freq_async(440, 1000);
  systick_wait_ms(1000);

  avr_set_motor(0, -80, 0);
  sound_freq_async(880, 1000);
  systick_wait_ms(1000);

  avr_set_motor(0, 80, 0);
  sound_freq_async(1320, 1000);
  systick_wait_ms(1000);

  avr_set_motor(0, 0, 1);
  systick_wait_ms(200);

  sound_freq(2000, 100);
  systick_wait_ms(50);
  sound_freq(2000, 100);
  systick_wait_ms(50);
}

void
main()
{
  core_init();
  more_test();

  core_shutdown();
}
