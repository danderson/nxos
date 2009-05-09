/* Copyright (c) 2009 the NxOS developers
 *
 * See AUTHORS for a full list of the developers.
 *
 * Redistribution of this file is permitted under
 * the terms of the GNU Public License (GPL) version 2.
 */

/* Motor control example.
 *
 * This example demonstrates the use of the motors API to control the
 * motors that can be connected to the NXT. If you want to run this
 * demo, plug a motor into the first output port. It is best to leave
 * it unconnected, as it may have movements that could damage a robot.
 */

#include "base/drivers/motors.h"
#include "base/drivers/systick.h" /* For nx_systick_wait_ms */
#include "base/display.h" /* for nx_display_uint */

/* Small delaying helper, which we use to give you time to see the
 * effects of the commands.
 */
static void wait() {
  nx_systick_wait_ms(2000);
}

void main() {
  /* On bootup, all the motors are stopped. Let's start one in
   * continuous mode. In this mode, the motor will continue at the
   * given speed until explicitely told to stop or do something else.
   */
  nx_motors_rotate(0, 100);

  wait();

  /* Speed control goes from -100 (full reverse) to 100 (full
   * forward). Let's reverse the motor's direction.
   */
  nx_motors_rotate(0, -100);

  wait();

  /* Now, stop the motor. There are two options here. Either don't
   * apply brakes, which lets the motor continue for a short while on
   * its inertia, or apply braking, which forcefully tries to bring
   * the motor to a halt as fast as possible. The following will
   * demonstrate both stop modes. First, a braking stop.
   */
  nx_motors_stop(0, TRUE);

  wait();
  nx_motors_rotate(0, 100);
  wait();

  /* And here, a coasting stop. */
  nx_motors_stop(0, FALSE);

  wait();

  /* You can also request rotation by a given angle, instead of just
   * blazing the motor on without limits. Note that there is no
   * precise feedback control built into the motor driver (yet), which
   * can cause it to overshoot the target angle because of its own
   * inertia. Let's rotate 90 degrees, with a braking finish.
   */
  nx_motors_rotate_angle(0, 100, 90, TRUE);

  wait();

  /* Finally, rotation can be set to stop after a given time. The
   * function call returns immediately, and the motor driver will take
   * care of stopping the motor after the specified time has
   * elapsed. Let's rotate in reverse, for 1 second, with a braking
   * finish.
   */
  nx_motors_rotate_time(0, -100, 1000, TRUE);

  wait();

  /* Finally, if this information has any value to you, you can query
   * the motor's current rotational position relative to its position
   * when it booted up. What you actually get here is the raw value of
   * the motor's tachymeter, which you should modulo 360 to get a more
   * sensible angular value.
   */
  nx_display_uint(nx_motors_get_tach_count(0));

  wait();
}
