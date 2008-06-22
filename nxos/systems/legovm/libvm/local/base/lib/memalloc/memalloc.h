/** @file memalloc.h
 *  @brief Non crosscompile substitute for base/lib/memalloc/memalloc.h
 */

/* Copyright (c) 2008 the NxOS developers
 *
 * See AUTHORS for a full list of the developers.
 *
 * Redistribution of this file is permitted under
 * the terms of the GNU Public License (GPL) version 2.
 */

#ifndef __NXOS_SYSTEMS_LEGOVM_LIBVM_LOCAL_BASE_LIB_MEMALLOC_MEMALLOC_H__
#define __NXOS_SYSTEMS_LEGOVM_LIBVM_LOCAL_BASE_LIB_MEMALLOC_MEMALLOC_H__

#include <stdlib.h>

#define nx_malloc malloc
#define nx_calloc calloc
#define nx_free free
#define nx_realloc realloc

#endif /* __NXOS_SYSTEMS_LEGOVM_LIBVM_LOCAL_BASE_LIB_MEMALLOC_MEMALLOC_H__ */
