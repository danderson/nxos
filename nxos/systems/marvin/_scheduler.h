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

#include "scheduler.h"

/** Initialize the scheduler. */
void mv__scheduler_init();

/** Start running the scheduler.
 *
 * Returns when there are no more tasks to run.
 */
void mv__scheduler_run();

#endif /* __NXOS_MARVIN__SCHEDULER_H__ */
