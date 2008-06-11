/* Copyright (c) 2007,2008 the NxOS developers
 *
 * See AUTHORS for a full list of the developers.
 *
 * Redistribution of this file is permitted under
 * the terms of the GNU Public License (GPL) version 2.
 */

#include "base/core.h"
#include "base/drivers/systick.h"
#include "base/drivers/avr.h"

#include "tests/tests.h"

/* The security hook lets the tester shut down the brick despite the
 * main thread of execution being locked up due to a bug. As long as the
 * system timer and AVR link are working, pressing the Cancel button
 * should power off the brick.
 */
static void security_hook(void) {
  if (nx_avr_get_button() == BUTTON_CANCEL)
    nx_core_halt();
}

void main(void) {
  nx_systick_install_scheduler(security_hook);

  //tests_all();
  //tests_usb();
  //tests_bt();
  //tests_usb_hardcore();
  //tests_radar();
  //tests_util();
  tests_defrag();
}
