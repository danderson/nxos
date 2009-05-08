/* Copyright (c) 2009 the NxOS developers
 *
 * See AUTHORS for a full list of the developers.
 *
 * Redistribution of this file is permitted under
 * the terms of the GNU Public License (GPL) version 2.
 */

/* System shutdown example.
 *
 * This example illustrates powering down the brick, as well as the
 * shutdown handler functionality.
 *
 * Application kernels can register a single handler function that
 * will be called when the brick shuts down. This handler lets the
 * application kernel halt itself cleanly, before the baseplate starts
 * the actual power-off sequence.
 */

#include "base/core.h"
#include "base/display.h" /* For nx_display_string */
#include "base/drivers/systick.h" /* For nx_systick_wait_ms */

/* Our shutdown handler. At this point, all of the baseplate's
 * functionality is still available.
 */
static void handle_shutdown() {
  /* Just display something, to show we got called, wait a bit, and
   * then let the shutdown proceed normally.
   */
  nx_display_string("Shutdown handler called!");
  nx_systick_wait_ms(1000);
}

void main() {
  /* Any shutdown after this function call will call the shutdown
   * handler.
   */
  nx_core_register_shutdown_handler(handle_shutdown);

  /* There are two ways to shut down cleanly. One is to return from
   * main(), which will automatically trigger a shutdown in the
   * baseplate. The other is to explicitely call nx_core_halt from
   * anywhere. That function call never returns, and powers off the
   * brick.
   */
  nx_core_halt();
}
