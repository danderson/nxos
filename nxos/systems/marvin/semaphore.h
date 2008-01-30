/** @file semaphore.h
 *  @brief Marvin's semaphore implementation.
 */

/* Copyright (C) 2007 the NxOS developers
 *
 * See AUTHORS for a full list of the developers.
 *
 * Redistribution of this file is permitted under
 * the terms of the GNU Public License (GPL) version 2.
 */

#ifndef __NXOS_MARVIN_SEMAPHORE_H__
#define __NXOS_MARVIN_SEMAPHORE_H__

#include "base/types.h"

typedef struct mv_sem mv_sem_t;

/** Symbolic names for special kinds of semaphores that can be created. */
enum {
  SEM_PRIVATE = 0, /**< Private semaphore, used for inter-task
                      signalling and synchronization. */
  SEM_MUTEX = 1, /**< Mutex, a preemption-aware locking mechanism. */
};

/** Create and return a new semaphore initialized at @a count.
 *
 * The constant SEM_MUTEX can be used to create a semaphore acting as a
 * mutex lock (@a count = 1). The constant SEM_PRIVATE can be used to
 * create a private signalling semaphore (@a count = 0).
 *
 * @param count The initial value of the semaphore.
 * @return A new initialized semaphore on success, or NULL on error.
 *
 * @note @a count may not be less than zero.
 */
mv_sem_t *mv_semaphore_create(S32 count);

/** Acquire one resource of @a sem.
 *
 * The function call will block the task until a resource becomes
 * available, if none are immediately available.
 *
 * @param sem The semaphore to decrement.
 */
void mv_semaphore_dec(mv_sem_t *sem);

/** Attempt to acquire one resource of @a sem.
 *
 * The function call will not block. The return value indicates whether
 * it returned successfully, or after failing to acquire a resource.
 *
 * @param sem The semaphore to decrement.
 * @return TRUE if the resource was successfully acquire
 */
bool mv_semaphore_try_dec(mv_sem_t *sem);

/** Release one resource of @a sem.
 *
 * @param sem The semaphore to increment.
 */
void mv_semaphore_inc(mv_sem_t *sem);

/** Destroy @a sem and free any memory it uses.
 *
 * @param sem the semaphore to destroy.
 *
 * @warning Destroying a semaphore while tasks are blocked on it will
 * cause Marvin to assert and crash. Be sure that the semaphore is no
 * longer used before freeing it.
 */
void mv_semaphore_destroy(mv_sem_t *sem);

#endif /* __NXOS_MARVIN_SEMAPHORE_H__ */
