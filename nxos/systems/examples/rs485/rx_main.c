/* Copyright (c) 2009 the NxOS developers
 *
 * See AUTHORS for a full list of the developers.
 *
 * Redistribution of this file is permitted under
 * the terms of the GNU Public License (GPL) version 2.
 */

/* RS485 receiver kernel example.
 *
 * This kernel configures an RS485 link on sensor port 4, and displays
 * whatever it reads from the RS485 bus on the screen. Check out
 * tx_main.c for the transmitter side of this example.
 */

#include "base/core.h"
#include "base/drivers/avr.h"
#include "base/drivers/systick.h"
#include "base/drivers/rs485.h"
#include "base/lock.h"
#include "base/display.h" /* For various display functions. */

/* A simple shutdown hook that lets you power down the brick if
 * something goes wrong. See the scheduler example kernel for details
 * on this.
 */
static void shutdown_hook(void) {
  if (nx_avr_get_button() == BUTTON_CANCEL)
    nx_core_halt();
}

/* Spinlock used for synchronization between the read callback and the
 * main code.
 */
static spinlock lock = SPINLOCK_INIT_LOCKED;

/* This function is called by the RS485 driver when a requested read
 * completes.
 */
static void recv_callback(nx_rs485_error_t err) {
  if (err == RS485_SUCCESS) {
    /* The read completed successfully, release the spinlock and allow
     * the main code to continue.
     */
    nx_spinlock_release(&lock);
  } else {
    /* There was some error. You can switch on the exact value and
     * report specific errors, but for this demo we'll just display an
     * error and fail.
     */
    nx_display_clear();
    nx_display_string("RX error!");
  }
}

void main() {
  /* We'll use this buffer to read data from the RS485 bus. */
  U8 buffer[12];

  /* Install our emergency shutdown hook. See above for details. */
  nx_systick_install_scheduler(shutdown_hook);

  /* Initialize the RS485 driver for communication at 9600 bits per
   * second with no timeout. If you need special communication modes
   * (number of start/stop bits, parity checks...), you can specify an
   * explicit mode register value.
   */
  nx_rs485_init(RS485_BR_9600, 0, 0, FALSE);

  for (int i = 0; i < 10; ++i) {
    /* Try to read 12 bytes from the RS485 bus. This call merely
     * instructs the RS485 driver to start receiving data, and returns
     * immediately.
     *
     * Once the read of all 12 bytes is complete, the given callback
     * function will be called, with a status value, indicating
     * whether or not there was an error.
     */
    nx_rs485_recv(buffer, sizeof(buffer), recv_callback);

    /* Since the above function returns immediately, we need a way to
     * wait for the receive to complete. How you do this depends on
     * how you want your application kernel to work, but in this case,
     * we'll just block on our spinlock, until the callback unlocks it
     * after the read completes.
     */
    nx_spinlock_acquire(&lock);

    /* Display the received data, then loop around for another
     * read.
     */
    nx_display_clear();
    nx_display_string("Iteration ");
    nx_display_uint(i+1);
    nx_display_string("\n");
    nx_display_string((char*)buffer);
  }
}
