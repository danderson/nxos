/*
 * Handling of the NXT's Periodic Interval Timer, which provides the
 * clock source for the main system timer.
 */

#ifndef __ESTORM_SYS_TIMER_H__
#define __ESTORM_SYS_TIMER_H__

/* Initialize the system clock that provides sleep support. */
void sys_timer_init();

/* Return the number of milliseconds elapsed since the system
 * started.
 */
unsigned long sys_timer_get_ms();

/* Busy-wait for MS milliseconds. */
void sys_timer_wait_ms(unsigned long ms);

/* Busy-wait for approximately NS nanoseconds. */
void sys_timer_wait_ns(unsigned long ns);

#endif
