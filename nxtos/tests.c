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
#include "memmap.h"

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
  systick_wait_ms(900);
}

void beep_word(U32 value) {
  U32 i=32;

  hello();

  while (i > 0 && !(value & 0x80000000)) {
    value <<= 1;
    i--;
  }
  while (i > 0) {
    if (value & 0x80000000)
      sound_freq(2000, 300);
    else
      sound_freq(1000, 300);
    systick_wait_ms(700);
    value <<= 1;
    i--;
  }

  goodbye();
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

void tests_sound() {
  enum {
    end = 0, sleep500 = 1, si = 990, dod = 1122,
    re = 1188, mi = 1320, fad = 1496, sol = 1584,
  } pain[] = {
    si, sleep500,
    fad, si, sol, sleep500,
    fad, mi, fad, sleep500,
    mi, fad, sol, sol, fad, mi, si, sleep500,
    fad, si, sol, sleep500,
    fad, mi, re,  sleep500,
    mi, re,  dod, dod, re,  dod, si, end
  };
  int i = 0;

  hello();

  while (pain[i] != end) {
    if (pain[i] == sleep500)
      systick_wait_ms(150);
    else
      sound_freq(pain[i], 150);
    systick_wait_ms(150);
    i++;
  }

  systick_wait_ms(1000);
  goodbye();
}

void tests_display() {
  char buf[2] = { 0, 0 };
  int i;

  hello();

  display_clear();
  display_cursor_set_pos(0, 0);

  display_string("***** NxtOS ****\n"
                 "----------------\n");
  for (i=32; i<128; i++) {
    buf[0] = i;
    display_string(buf);
    if ((i % 16) == 15)
      display_string("\n");
  }

  systick_wait_ms(5000);
  goodbye();
}

void tests_time() {
  int i;
  int t;

  hello();

  for (i=0; i<20; i++) {
    t = systick_get_ms();

    /* Manual screen refresh on even seconds, automatic on odd
     * seconds.
     */
    display_auto_refresh((i % 2) ? TRUE : FALSE);

    display_clear();
    display_cursor_set_pos(0, 0);
    display_string("  TX52 - NxtOS\n\n"
                   "\n"
                   "T(ms) : ");
    display_uint(t);
    display_string("\nT(hex): ");
    display_hex(t);

    /* If this is an even iteration, do the manual refresh. */
    if (!(i % 2))
      display_refresh();

    systick_wait_ms(900);
    sound_freq(1500, 100);
  }

  goodbye();
}

void tests_sysinfo() {
  hello();

  display_clear();
  display_cursor_set_pos(0,0);
  display_string("- System Info. -\n\n");

  display_string("Boot from ");
  if (BOOTED_FROM_SAMBA)
    display_string("SAM-BA");
  else
    display_string("ROM");
  display_end_line();

  display_string("Free RAM: ");
  display_uint(FREE_SIZE);
  display_end_line();

  systick_wait_ms(5000);
  goodbye();
}
