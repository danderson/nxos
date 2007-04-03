#include "at91sam7s256.h"

#include "crt0.h"
#include "aic.h"

/* The board is clocked at 48MHz */
#define CLOCK_FREQ 48000000

/* The Periodic Interval Timer runs at 3MHz. */
#define PIT_FREQ (CLOCK_FREQ/16)

/* This counter keeps the system time. It is currently used only for
 * the sleep code.
 */
static volatile unsigned long tick_ms;

static void sys_timer_isr() {
  unsigned long status;

  /* The PIT requires reading the status register to acknowledge the
   * interrupt. Let's do that.
   */
  status = *AT91C_PITC_PIVR;

  /* Increment the system counter. */
  tick_ms++;

  /* Manually trigger the low priority kernel interrupt. This low
   * priority interrupt will do all the slower tasks, that can be
   * interrupted by hardware needing fast irq handling.
   *
   * TODO: Enable this when it has something to do.
   */
  //aic_trigger_irq(AT91C_ID_PWMC);
}

void sys_timer_init() {
  interrupts_disable();

  /* Set the Periodic Interval Timer to a tiny interval, and set it
   * disabled. This way, it should wrap around and shut down quickly,
   * if it's already running.
   */
  *AT91C_PITC_PIMR = 1;

  /* Wait for the timer's internal counter to return to zero. */
  while (*AT91C_PITC_PIVR & AT91C_PITC_CPIV);

  /* Install the interrupt handler for the system timer, and tell the
   * AIC to handle that interrupt.
   */
  aic_install_isr(AT91C_ID_SYS, sys_timer_isr);
  aic_enable(AT91C_ID_SYS);

  /* Configure the Periodic Interval Timer with a frequency of
   * 1000Hz. The timer is enabled, and will raise the interrupt we
   * installed previously 1000 times a second.
   */
  *AT91C_PITC_PIMR = (((PIT_FREQ/1000)-1) |
                      AT91C_PITC_PITEN |
                      AT91C_PITC_PITIEN);

  interrupts_enable();
}

unsigned long sys_timer_get_ms() {
  return tick_ms;
}

void sys_timer_wait_ms(unsigned long ms) {
  volatile unsigned long final = tick_ms + ms;

  while (tick_ms < final);
}

void sys_timer_wait_ns(unsigned long ns) {
  volatile unsigned long i = (ns >> 7) + 1;

  while (i) i--;
}
