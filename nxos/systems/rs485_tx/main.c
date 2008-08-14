/* Copyright (c) 2008 the NxOS developers
 *
 * See AUTHORS for a full list of the developers.
 *
 * Redistribution of this file is permitted under
 * the terms of the GNU Public License (GPL) version 2.
 */

#include "base/core.h"
#include "base/util.h"
#include "base/display.h"
#include "base/drivers/systick.h"
#include "base/drivers/avr.h"
#include "base/drivers/rs485.h"

/* The security hook lets the tester shut down the brick despite the
 * main thread of execution being locked up due to a bug. As long as
 * the system timer and AVR link are working, pressing the Cancel
 * button should power off the brick.
 */
static void security_hook(void) {
  if (nx_avr_get_button() == BUTTON_CANCEL)
    nx_core_halt();
}

/* 12 chars, including trailing \0. */
static U8 out_buffer[] = "hello world";
static volatile int lock;

static void send_callback(void) {
  static U8 flag = 1;

  nx_display_clear();
  nx_display_cursor_set_pos(0,6);
  nx_display_string(flag ? "T\n" : " \n");

  out_buffer[5] = (flag ? 0 : ' ');
  nx_display_string((const char *)out_buffer);
  flag = (flag+1) % 2;
  lock = 0;
}

void main (void) {
  nx_systick_install_scheduler(security_hook);
  nx_rs485_init();

  while (1) {
    lock = 1;
    nx_rs485_send(out_buffer, sizeof(out_buffer), send_callback);
    while (lock) {
      nx_systick_wait_ms(1000);
    }
  }
}

