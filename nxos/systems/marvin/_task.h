/** @file _task.h
 *  @brief Assembler task access helpers for the scheduler.
 */

/* Copyright (C) 2007 the NxOS developers
 *
 * See AUTHORS for a full list of the developers.
 *
 * Redistribution of this file is permitted under
 * the terms of the GNU Public License (GPL) version 2.
 */

#ifndef __NXOS_MARVIN__TASK_H__
#define __NXOS_MARVIN__TASK_H__

#include "base/types.h"

void mv__task_run_first(nx_closure_t func, U32 *stack);
U32 *mv__task_get_stack(void);
void mv__task_set_stack(U32 *stack);

#endif /* __NXOS_MARVIN__TASK_H__ */
