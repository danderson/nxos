#ifndef __NXTOS_AIC_H__
#define __NXTOS_AIC_H__

#include "mytypes.h"

typedef U32 aic_vector_t;

/* Priority levels for interrupt lines. */
typedef enum {
  AIC_INT_LEVEL_LOW = 2,
  AIC_INT_LEVEL_NORMAL = 4,
  AIC_INT_LEVEL_ABOVE_NORMAL = 5,
} aic_priority_t;

typedef void (*aic_isr_t)();

void aic_init();
void aic_install_isr(aic_vector_t vector, aic_priority_t prio, aic_isr_t isr);
void aic_enable(aic_vector_t vector);
void aic_disable(aic_vector_t vector);
void aic_set(aic_vector_t vector);
void aic_clear(aic_vector_t vector);

#endif
