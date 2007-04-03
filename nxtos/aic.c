/*
 * Advanced Interrupt Controller routines
 *
 * The AIC was initialized by the bootstrap code, so we only have
 * utility functions to install interrupts and mask selected interrupt
 * lines.
 *
 * This code is very heavily inspired by the work of Charles Manning,
 * of Lejos fame.
 */

#include "at91sam7s256.h"

#include "aic.h"

void aic_enable(aic_vector_t vector) {
  *AT91C_AIC_IECR = (1 << vector);
}

void aic_disable(aic_vector_t vector) {
  *AT91C_AIC_IDCR = (1 << vector);
}

void aic_install_isr(aic_vector_t vector, void (*isr)()) {
  /* Disable the interrupt we're installing. Getting interrupted while
   * we are tweaking it could be bad.
   */
  aic_disable(vector);

  /* Set the irq mode to positive edge triggered and priority 0. */
  AT91C_AIC_SMR[vector] = (AT91C_AIC_PRIOR_LOWEST |
                           AT91C_AIC_SRCTYPE_INT_EDGE_TRIGGERED);

  /* Install the provided ISR. */
  AT91C_AIC_SVR[vector] = (unsigned int)isr;
}

inline void aic_trigger_irq(aic_vector_t vector) {
  *AT91C_AIC_ISCR = (1 << vector);
}
