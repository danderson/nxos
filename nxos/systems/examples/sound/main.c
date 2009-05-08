/* Copyright (c) 2009 the NxOS developers
 *
 * See AUTHORS for a full list of the developers.
 *
 * Redistribution of this file is permitted under
 * the terms of the GNU Public License (GPL) version 2.
 */

/* Sound driver example.
 *
 * The sound driver is one of the simplest drivers in NxOS, producing
 * beeps at a specified frequency on demand.
 */

#include "base/drivers/sound.h"
#include "base/drivers/systick.h" /* For nx_systick_wait_ms */

void main() {
  /* Play a 2kHz tone for 1 second. This function waits for the tone
   * to finish before continuing.
   */
  nx_sound_freq(2000, 1000);

  /* Another tone, at 4kHz, for 2 seconds. This time, we'll use the
   * asynchronous tone function, which just starts the tone and
   * returns immediately. The tone will play for the duration you
   * specified, or until the brick is halted, whichever happens first.
   */
  nx_sound_freq_async(4000, 2000);

  /* We'll use the explicit wait function to wait for the tone to
   * finish.
   */
  nx_systick_wait_ms(3000);
}
