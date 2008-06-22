/** @file assert.h
 *  @brief Non crosscompile substitute for base/assert.h
 */

/* Copyright (c) 2008 the NxOS developers
 *
 * See AUTHORS for a full list of the developers.
 *
 * Redistribution of this file is permitted under
 * the terms of the GNU Public License (GPL) version 2.
 */

#ifndef __NXOS_SYSTEMS_LEGOVM_LIBVM_LOCAL_BASE_ASSERT_H__
#define __NXOS_SYSTEMS_LEGOVM_LIBVM_LOCAL_BASE_ASSERT_H__

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define NX_ASSERT assert

static inline void NX_FAIL(const char *msg) {
  printf(msg);
  printf("\n");
  abort();
}

#endif /* __NXOS_SYSTEMS_LEGOVM_LIBVM_LOCAL_BASE_ASSERT_H__ */
