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
#include "motors.h"

static bool test_silent = FALSE;

static void hello() {
  if (test_silent)
    return;
  sound_freq(1000, 100);
  systick_wait_ms(50);
  sound_freq(2000, 100);
  systick_wait_ms(900);
}

static void goodbye() {
  if (test_silent)
    return;
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


void tests_display() {
  char buf[2] = { 0, 0 };
  int i;

  hello();

  display_clear();
  display_cursor_set_pos(0, 0);

  display_string("- Display test -\n"
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

  display_clear();
  display_cursor_set_pos(0,0);
  display_string("-- Sound test --\n"
                 "----------------\n");

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


void
tests_motor() {
  hello();

  display_clear();
  display_cursor_set_pos(0,0);
  display_string("--- AVR test ---\n"
                 "----------------\n");

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


void tests_tachy() {
  int i;
  hello();

  motors_rotate_angle(0, 80, 1024, TRUE);
  motors_rotate_time(1, -80, 3000, FALSE);
  motors_rotate(2, 80);

  for (i=0; i<30; i++) {
    display_clear();
    display_cursor_set_pos(0,0);

    display_clear();
    display_cursor_set_pos(0,0);
    display_string("Tachymeter  test\n"
                   "----------------\n");

    display_string("Tach A: ");
    display_hex(motors_get_tach_count(0));
    display_end_line();

    display_string("Tach B: ");
    display_hex(motors_get_tach_count(1));
    display_end_line();

    display_string("Tach C: ");
    display_hex(motors_get_tach_count(2));
    display_end_line();

    display_string("Refresh: ");
    display_uint(i);
    display_end_line();

    systick_wait_ms(250);
  }

  motors_stop(2, TRUE);

  goodbye();
}


void tests_sysinfo() {
  U32 i, t;
  const U32 display_seconds = 15;
  U8 avr_major, avr_minor;
  hello();

  avr_get_version(&avr_major, &avr_minor);

  for (i=0; i<(display_seconds*4); i++) {
    if (i % 4 == 0)
      t = systick_get_ms();

    display_clear();
    display_cursor_set_pos(0,0);
    display_string("- System  info -\n"
                   "----------------\n");

    display_string("Time  : ");
    display_uint(t);
    display_end_line();

    display_string("Boot from ");
    if (BOOT_FROM_SAMBA)
      display_string("SAM-BA");
    else
      display_string("ROM");
    display_end_line();

    display_string("Free RAM: ");
    display_uint(USERSPACE_SIZE);
    display_end_line();

    display_string("Buttons: ");
    display_uint(avr_get_button());
    display_end_line();

    display_string("AVR Ver.: ");
    display_uint(avr_major);
    display_string(".");
    display_uint(avr_minor);

    systick_wait_ms(250);
  }

  goodbye();
}


void tests_all() {
  test_silent = TRUE;

  tests_display();
  tests_sound();
  tests_motor();
  tests_tachy();
  tests_sysinfo();

  test_silent = FALSE;
  goodbye();
}
