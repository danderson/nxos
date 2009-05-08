/* Copyright (c) 2009 the NxOS developers
 *
 * See AUTHORS for a full list of the developers.
 *
 * Redistribution of this file is permitted under
 * the terms of the GNU Public License (GPL) version 2.
 */

/* Timing services.
 *
 * NxOS provides basic timing services, allowing you to determine the
 * elapsed time since startup, as well as wait for a given time
 * interval. This functionality is in the 'systick' (for "system
 * tick") driver.
 *
 * Note that the systick driver has other functionality, related to
 * running an operating system task scheduler. This functionality is
 * demonstrated in the 'scheduler' example kernel.
 */

#include "base/assert.h" /* for NX_ASSERT */
#include "base/drivers/systick.h"

void main() {
  /* You can get the current time since bootup. The NXT does not
   * include a real time clock, so systick can only tell you the time
   * since bootup, not the "wall clock" time.
   */
  const U32 time_since_bootup = nx_systick_get_ms();

  /* You can use systick to wait for a specified time interval, either
   * in milliseconds or nanoseconds. The latter is used for very high
   * resolution timing loops, and should only really be useful to
   * drivers.
   */
  nx_systick_wait_ms(2000);
  nx_systick_wait_ns(42);

  /* At this point, system time is guaranteed to be greater than
   * before the wait calls. However, due to the way NxOS handles time,
   * more time than 2000ms + 42ns may have elapsed.
   */
  const U32 time_after_wait = nx_systick_get_ms();
  const U32 time_delta = time_after_wait - time_since_bootup;
  NX_ASSERT(time_delta > 2000);
}
