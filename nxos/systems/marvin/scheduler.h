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

#endif /* __NXOS_MARVIN_SCHEDULER_H__ */
