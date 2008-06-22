/** @file util.h
 *  @brief Non crosscompile substitute for base/util.h
 */

/* Copyright (c) 2008 the NxOS developers
 *
 * See AUTHORS for a full list of the developers.
 *
 * Redistribution of this file is permitted under
 * the terms of the GNU Public License (GPL) version 2.
 */

#ifndef __NXOS_SYSTEMS_LEGOVM_LIBVM_LOCAL_BASE_UTIL_H__
#define __NXOS_SYSTEMS_LEGOVM_LIBVM_LOCAL_BASE_UTIL_H__

/* For memcpy() and memset() */
#include <string.h>

#include "base/types.h"

static inline bool streq(const char *a, const char *b) {
  return strcmp(a, b) == 0;
}

#endif /* __NXOS_SYSTEMS_LEGOVM_LIBVM_LOCAL_BASE_UTIL_H__ */
