/* Copyright (c) 2009 the NxOS developers
 *
 * See AUTHORS for a full list of the developers.
 *
 * Redistribution of this file is permitted under
 * the terms of the GNU Public License (GPL) version 2.
 */

/* AVR driver usage example.
 *
 * This example illustrates the use of the few functions found in the
 * "AVR" driver. These are simple functions provided by the NXT's AVR
 * coprocessor, which constantly communicates with the system's main
 * CPU (which is running NxOS).
 *
 * The AVR also handles some more complex functionality, such as power
 * and motor management, but these functionalities have their own
 * dedicated driver interfaces with more complex APIs.
 */

#include "base/drivers/avr.h"
#include "base/display.h" /* For various display functions. */
#include "base/drivers/systick.h" /* For nx_systick_wait_ms */

/* This example kernel is simply going to display all the information
 * that it can obtain from the AVR driver on the brick's screen. This
 * helper function is the one doing the displaying.
 *
 * To allow you to shutdown the brick, we'll also return FALSE from
 * this function when we detect that the "cancel" button (the small
 * one beneath the big orange one) has been pressed.
 */
static bool display_avr_info() {
  /* Retrieve the button press state. Due to how the button detection
   * is physically implemented, we can only detect one button press at
   * any one time.
   */
  const nx_avr_button_t button_state = nx_avr_get_button();

  /* Ask the AVR for the battery voltage measurement, in
   * millivolts. This gives us an indication of the power still
   * available in the battery, as voltage starts to drop sharply as
   * the batteries start to run out.
   */
  const U32 battery_voltage = nx_avr_get_battery_voltage();

  /* Ask the AVR for the battery type. The NXT can be powered either
   * by AA batteries, or by a rechargeable power pack sold by
   * lego. This lets us differenciate between the two.
   */
  const bool is_aa_batteries = nx_avr_battery_is_aa();

  /* Ask the AVR for its firmware version. This is currently more for
   * amusement than anything else, since there are no
   * version-dependent features in the AVR's firmware (indeed, it
   * cannot be upgraded in the field).
   */
  U8 major, minor;
  nx_avr_get_version(&major, &minor);

  /* Now, display all of this information. See the 'display' example
   * kernel if you want more details about these display functions.
   */
  nx_display_clear();
  nx_display_string("Button: ");
  switch (button_state) {
    case BUTTON_NONE:
      nx_display_string("None");
      break;
    case BUTTON_OK:
      nx_display_string("OK");
      break;
    case BUTTON_CANCEL:
      nx_display_string("Cancel");
      break;
    case BUTTON_LEFT:
      nx_display_string("Left");
      break;
    case BUTTON_RIGHT:
      nx_display_string("Right");
      break;
  }
  nx_display_end_line();
  nx_display_string("Bat: ");
  nx_display_uint(battery_voltage);
  nx_display_string("mV\nType: ");
  if (is_aa_batteries)
    nx_display_string("AA\n");
  else
    nx_display_string("Pack\n");
  nx_display_string("Ver: ");
  nx_display_uint(major);
  nx_display_string(".");
  nx_display_uint(minor);

  /* If the button being pressed is 'Cancel', ask main() to shut down. */
  return (button_state != BUTTON_CANCEL);
}

void main() {
  /* Display the AVR information in a 1-second loop, until the display
   * function tells us to shut down.
   */
  while (display_avr_info())
    nx_systick_wait_ms(1000);

  /* Wait a bit here, to let you see the final status before shutdown. */
  nx_systick_wait_ms(1000);
}
