/* Copyright (c) 2009 the NxOS developers
 *
 * See AUTHORS for a full list of the developers.
 *
 * Redistribution of this file is permitted under
 * the terms of the GNU Public License (GPL) version 2.
 */

/* RS485 transmitter kernel example.
 *
 * This kernel configures an RS485 link on sensor port 4, and
 * periodically writes string data on the bus. Check out rx_main.c for
 * the receiver side of this example.
 */

#include "base/core.h"
#include "base/drivers/avr.h"
#include "base/drivers/rs485.h"
#include "base/drivers/systick.h"
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

/* This function is called by the RS485 driver when a requested write
 * completes.
 */
static void send_callback(nx_rs485_error_t err) {
  if (err == RS485_SUCCESS) {
    /* The write completed successfully, release the spinlock and
     * allow the main code to continue.
     */
    nx_spinlock_release(&lock);
  } else {
    /* There was some error. You can switch on the exact value and
     * report specific errors, but for this demo we'll just display an
     * error and fail.
     */
    nx_display_clear();
    nx_display_string("TX error!");
  }
}

void main() {
  /* We'll use this buffer to read data from the RS485 bus. This needs
   * to be volatile because it gets modified outside of the main code
   * path (in interrupt handlers).
   */
  U8 buffer_hello[12] = "Hello world";
  U8 buffer_empty[12] = {0};

  /* Install our emergency shutdown hook. See above for details. */
  nx_systick_install_scheduler(shutdown_hook);

  /* Initialize the RS485 driver for communication at 9600 bits per
   * second with no timeout. If you need special communication modes
   * (number of start/stop bits, parity checks...), you can specify an
   * explicit mode register value.
   */
  nx_rs485_init(RS485_BR_9600, 0, 0, FALSE);

  for (int i = 0; i < 10; ++i) {
    /* This is just to alternate between sending two different
     * messages. One time we send "hello world", the other an empty
     * string.
     */
    U8 *buffer = (i % 2 == 0) ? buffer_hello : buffer_empty;

    /* Display what we are transmitting, for the record */
    nx_display_clear();
    nx_display_string("Iteration ");
    nx_display_uint(i+1);
    nx_display_string("\n");
    nx_display_string((char*)buffer);

    /* Wait a bit before actually sending, to get a nice obvious
     * delayed transmission effect.
     */
    nx_systick_wait_ms(1000);

    /* Try to write 12 bytes to the RS485 bus. This call merely
     * instructs the RS485 driver to start writing data, and returns
     * immediately.
     *
     * Once the read of all 12 bytes is complete, the given callback
     * function will be called, with a status value, indicating
     * whether or not there was an error.
     */
    nx_rs485_send(buffer, sizeof(buffer), send_callback);

    /* Since the above function returns immediately, we need a way to
     * wait for the receive to complete. How you do this depends on
     * how you want your application kernel to work, but in this case,
     * we'll just block on our spinlock, until the callback unlocks it
     * after the write completes.
     */
    nx_spinlock_acquire(&lock);

    /* Wait before transmitting the next round, again purely for
     * effect.
     */
    nx_systick_wait_ms(1000);
  }
}
