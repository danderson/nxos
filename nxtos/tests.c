/* Various test routines for components of the NXT. */

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

static void hello() {
  sound_freq(1000, 100);
  systick_wait_ms(50);
  sound_freq(2000, 100);
  systick_wait_ms(900);
}

static void goodbye() {
  sound_freq(2000, 100);
  systick_wait_ms(50);
  sound_freq(1000, 100);
  systick_wait_ms(50);
}


void
tests_motor() {
  hello();

  avr_set_motor(0, 80, 0);
  systick_wait_ms(1000);

  avr_set_motor(0, -80, 0);
  systick_wait_ms(1000);

  avr_set_motor(0, 80, 0);
  systick_wait_ms(1000);

  avr_set_motor(0, 0, 1);
  systick_wait_ms(200);

  goodbye();
}

void tests_display() {
  char buf[2] = { 0, 0 };
  int i;

  hello();

  display_clear();
  display_cursor_set_pos(0, 0);

  for (i=32; i<128; i++) {
    buf[0] = i;
    display_string(buf);
    if ((i % 16) == 15)
      display_string("\n");
  }
  display_refresh();

  systick_wait_ms(5000);
  goodbye();
}

void tests_time() {
  int i;
  int t;

  hello();

  for (i=0; i<20; i++) {
    t = systick_get_ms();
    display_clear();
    display_cursor_set_pos(0, 0);
    display_string("  TX52 - NxtOS\n\n"
                   "\n"
                   "T(ms) : ");
    display_uint(t);
    display_string("\nT(hex): ");
    display_hex(t);
    display_refresh();
    systick_wait_ms(883);
    sound_freq(1500, 100);
  }

  goodbye();
}
