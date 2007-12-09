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
void mv__scheduler_init();

/** Start running the scheduler.
 *
 * Returns when there are no more tasks to run.
 */
void mv__scheduler_run();

/** Set @a task to the blocked state. */
void mv__scheduler_task_block(mv_task_t *task);

/** Set @a task to the ready state. */
void mv__scheduler_task_unblock(mv_task_t *task);

#endif /* __NXOS_MARVIN__SCHEDULER_H__ */
