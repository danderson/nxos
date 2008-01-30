/** @file _scheduler.h
 *  @brief Marvin's internal scheduler API.
 */

/* Copyright (C) 2007 the NxOS developers
 *
 * See AUTHORS for a full list of the developers.
 *
 * Redistribution of this file is permitted under
 * the terms of the GNU Public License (GPL) version 2.
 */

#ifndef __NXOS_MARVIN__SCHEDULER_H__
#define __NXOS_MARVIN__SCHEDULER_H__

#include "marvin/scheduler.h"

/** Initialize the scheduler. */
void mv__scheduler_init(void);

/** Start running the scheduler.
 *
 * Returns when there are no more tasks to run.
 */
void mv__scheduler_run(void);

/** Set the current task to the blocked state.
 *
 * @note This function only sets the blocked state, but does not preempt
 * the task. This is to allow further setup and twiddling by the caller
 * before the scheduler is unlocked.
 */
void mv__scheduler_task_block(void);

/** Set @a task to the ready state.
 *
 * @param task The task to set unblocked.
 */
void mv__scheduler_task_unblock(mv_task_t *task);

/** Suspend the running task for @a time milliseconds.
 *
 * This call will block the task until its wakeup time.
 *
 * @param time The number of milliseconds before resuming @a task.
 */
void mv__scheduler_task_suspend(U32 time);

#endif /* __NXOS_MARVIN__SCHEDULER_H__ */
