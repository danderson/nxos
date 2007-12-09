/** @file time.h
 *  @brief Marvin's time API.
 */

/* Copyright (C) 2007 the NxOS developers
 *
 * See AUTHORS for a full list of the developers.
 *
 * Redistribution of this file is permitted under
 * the terms of the GNU Public License (GPL) version 2.
 */

#ifndef __NXOS_MARVIN_TIME_H__
#define __NXOS_MARVIN_TIME_H__

/* Sleep for @a ms milliseconds.
 *
 * @param ms The number of milliseconds to sleep.
 *
 * @note This API is roughly equivalent to the baseplate's
 * nx_systick_wait_ms(), except that this function is scheduler-aware,
 * and therefore does not busy-wait the CPU.
 */
void mv_time_sleep(U32 ms);

#endif /* __NXOS_MARVIN_TIME_H__ */
