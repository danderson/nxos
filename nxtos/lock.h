#ifndef __NXTOS_LOCK_H__
#define __NXTOS_LOCK_H__

#include "mytypes.h"

typedef volatile U8 spinlock; /* Basic spinlock type. */

#define SPINLOCK_INIT_UNLOCKED 0
#define SPINLOCK_INIT_LOCKED 1

extern void spinlock_acquire_from_ref(spinlock *lock);
#define spinlock_acquire(lock) spinlock_acquire_from_ref(&lock)
extern bool spinlock_try_acquire_from_ref(spinlock *lock);
#define spinlock_try_acquire(lock) spinlock_try_acquire_from_ref(&lock)
#define spinlock_release(lock) { lock = 0; }

#endif /* __NXTOS_LOCK_H__ */
