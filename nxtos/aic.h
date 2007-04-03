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

#ifndef __ESTORM_AIC_H__
#define __ESTORM_AIC_H__

typedef long int aic_vector_t;

/* Enable the interrupt VECTOR in the AIC. */
void aic_enable(aic_vector_t vector);

/* Disable the interrupt VECTOR in the AIC. */
void aic_disable(aic_vector_t vector);

/* Install the given ISR as the Interrupt Service Routine for the
 * given interrupt VECTOR. On return, ISR is installed but the VECTOR
 * line is left masked in the AIC. You need to enable yourself when
 * ready with a call to aic_enable().
 */
void aic_install_isr(aic_vector_t vector, void (*isr)());

/* Manually force the AIC to trigger an irq exception for the given
 * interrupt VECTOR.
 */
inline void aic_trigger_irq(aic_vector_t vector);

#endif
