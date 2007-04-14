#include "at91sam7s256.h"

#include "crt0.h"
#include "aic.h"
#include "avr.h"

static void system_1khz_update() {
  avr_1khz_update();
}

void system_services_init() {
  interrupts_disable();

  /* Install the interrupt handler for the system timer, and tell the
   * AIC to handle that interrupt.
   */
  aic_install_isr(AT91C_ID_PWMC, system_1khz_update);
  aic_enable(AT91C_ID_PWMC);

  interrupts_enable();
}
