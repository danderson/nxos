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
#include "base/drivers/sound.h"

/* The security hook lets the tester shut down the brick despite the
 * main thread of execution being locked up due to a bug. As long as
 * the system timer and AVR link are working, pressing the Cancel
 * button should power off the brick.
 */
static void security_hook(void) {
  if (nx_avr_get_button() == BUTTON_CANCEL)
    nx_core_halt();
}

static U8 in_buffer[12];
static volatile int lock;

static void recv_callback(nx_rs485_error_t err) {
  nx_display_cursor_set_pos(0,0);
  nx_display_string("           ");
  nx_display_cursor_set_pos(0,0);
  nx_display_string((const char *)in_buffer);

  if (err == RS485_SUCCESS) {
    memset(in_buffer, 0, sizeof(in_buffer));
    lock = 0;
  }
}

void main(void) {
  bool flag = TRUE;

  nx_systick_install_scheduler(security_hook);
  nx_rs485_init(RS485_BR_9600, 0, 0, FALSE);

  while (1) {
    nx_display_cursor_set_pos(0,6);
    nx_display_string(flag ? "R" : " ");
    flag = ~flag;
    lock = 1;
    nx_rs485_recv(in_buffer, sizeof(in_buffer), recv_callback);
    while (lock) {
      nx_systick_wait_ms(1000);
      nx_sound_freq_async(2000, 200);
    }
  }
}
