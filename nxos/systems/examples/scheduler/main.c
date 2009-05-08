/* Copyright (c) 2009 the NxOS developers
 *
 * See AUTHORS for a full list of the developers.
 *
 * Redistribution of this file is permitted under
 * the terms of the GNU Public License (GPL) version 2.
 */

/* Scheduler management example.
 *
 * While the NxOS baseplate does not provide any facilities for task
 * scheduling (ie. multitasking), it does offer some basic
 * functionality to application kernels, allowing them to register
 * their own scheduler with the baseplate's interrupt dispatch
 * system.
 *
 * Using this facility, application kernels can register their own
 * scheduler and have it invoked at regular intervals by the kernel.
 *
 * Note that in this context, "scheduler" doesn't just mean task
 * scheduling. It may be used for alltogether different
 * purposes. Anything which can be built on top of a function that is
 * run periodically outside the normal execution flow can be
 * implemented using this scheduler facility.
 */

#include "base/drivers/systick.h"
#include "base/display.h" /* For display functions */

/* Let's have a global counter. "scheduling" in our system simply
 * means incrementing this counter's value. Think of it as a less
 * precise alter ego of the systick driver's system clock.
 */
static volatile U32 counter = 0;

/* Our scheduler function. This function gets called at regular
 * intervals by the NxOS baseplate. Our "scheduler" is very simple, it
 * just increments a counter, to show that it does get called.
 *
 * The function executes in an interrupt context, which means that you
 * have only limited stack space and may not be able to use any device
 * driver you wish from here (this limitation, if it exists, is
 * documented by the device driver).
 */
static void schedule() {
  counter++;
}

/* A small helper to display the counter value. */
static void show_counter() {
  nx_display_clear();
  nx_display_uint(counter);
  nx_systick_wait_ms(1000);
}

void main() {
  /* Install the scheduler callback. As of here, the scheduler will be
   * called periodically.
   */
  nx_systick_install_scheduler(schedule);

  /* By the time the CPU gets round to displaying this, the counter
   * should already be non-zero.
   */
  show_counter();

  /* Now it should have advanced more. The scheduler gets called
   * approximately 1000 times per second.
   */
  show_counter();

  /* The main code path can also force an explicit scheduler
   * invocation if it so desires. Execution of the scheduler callback
   * still happens in an interrupt handler context, but the interrupt
   * is triggered without waiting for the next periodic tick.
   */
  nx_systick_call_scheduler();

  /* The scheduler can be temporarily masked, preventing it from being
   * run periodically. This is conceptually similar to masking device
   * interrupts, except that here, only the scheduler function we
   * registered is prevented from executing.
   */
  nx_systick_mask_scheduler();

  /* Just to show that the counter is no longer incrementing. */
  show_counter();
  show_counter();

  /* And of course, when you're ready, the scheduler can be
   * unmasked.
   */
  nx_systick_unmask_scheduler();

  /* Just to show that the counter is once again incrementing. */
  show_counter();
  show_counter();

  /* Short wait to let you see the counter, before the brick shuts
   * down.
   */
  nx_systick_wait_ms(2000);
}
