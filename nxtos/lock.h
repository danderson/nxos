#ifndef __ESTORM_LOCK_H__
#define __ESTORM_LOCK_H__

#include "types.h"

typedef volatile U8 spinlock; /* Basic spinlock type. */

#define SPINLOCK_INIT_UNLOCKED 0
#define SPINLOCK_INIT_LOCKED 1

#define spinlock_acquire(lock) { \
  U8 prev_value = 1; \
  while (prev_value) { \
    __asm__ volatile ("swpb %0, %1, [%2]" \
                      : "=r" (prev_value) \
                      : "0" (prev_value), "r" (&lock) \
                      : "memory"); \
  } \
}

#define spinlock_release(lock) { lock = 0; }

#endif /* __ESTORM_LOCK_H__ */
