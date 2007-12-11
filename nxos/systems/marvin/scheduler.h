/** @file scheduler.h
 *  @brief Marvin's scheduler API.
 */

/* Copyright (C) 2007 the NxOS developers
 *
 * See AUTHORS for a full list of the developers.
 *
 * Redistribution of this file is permitted under
 * the terms of the GNU Public License (GPL) version 2.
 */

#ifndef __NXOS_MARVIN_SCHEDULER_H__
#define __NXOS_MARVIN_SCHEDULER_H__

#include "base/types.h"

typedef struct mv_task mv_task_t;

/** Create a new task executing @a func, with @a stack bytes of stack.
 *
 * The task is placed in the ready state and enqueued for CPU time.
 *
 * @param func The function the new task should execute.
 * @param stack The size of the task stack in bytes.
 *
 * @warning The stack should have sizeof(nx_task_stack_t) bytes
 * available for task switching at all times.
 *
 * @warning Currently this function can only be run before the scheduler
 * starts up.
 *
 * @note The usual size for the task stack is 1k, ie. 1024 bytes.
 */
void mv_scheduler_create_task(nx_closure_t func, U32 stack);

/** Explicitely yield the CPU.
 *
 * This will cause the calling task to be preempted. You shouldn't
 * really need this, except maybe for testing purposes.
 *
 * @param unlock If TRUE, the scheduler will be atomically unlocked just
 *               before yielding.
 */
void mv_scheduler_yield(bool unlock);

/** Return a handle to the current task.
 *
 * @return The mv_task_t handle of the current task.
 */
mv_task_t *mv_scheduler_get_current_task(void);

/** Increment the scheduler lock.
 *
 * The scheduler lock is recursive. If you lock it N times, you must
 * unlock it N times (or reset it to zero) to effectively unlock the
 * scheduler.
 *
 * @note While the scheduler is locked, the currently running task
 * effectively cannot be preempted. Be sure to keep the locking short.
 */
void mv_scheduler_lock(void);

/** Decrement the scheduler lock.
 *
 * If the scheduler lock reaches zero, the scheduler state is unlocked
 * and the currently running task becomes preemptable again.
 */
void mv_scheduler_unlock(void);

#endif /* __NXOS_MARVIN_SCHEDULER_H__ */
