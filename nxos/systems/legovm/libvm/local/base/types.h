/** @file types.h
 *  @brief Non crosscompile substitute for base/types.h
 */

/* Copyright (c) 2008 the NxOS developers
 *
 * See AUTHORS for a full list of the developers.
 *
 * Redistribution of this file is permitted under
 * the terms of the GNU Public License (GPL) version 2.
 */

#ifndef __NXOS_SYSTEMS_LEGOVM_LIBVM_LOCAL_BASE_TYPES_H__
#define __NXOS_SYSTEMS_LEGOVM_LIBVM_LOCAL_BASE_TYPES_H__

#include <stdlib.h>

typedef unsigned char U8;
typedef signed char S8;
typedef unsigned short U16;
typedef unsigned long U32;
typedef U8 bool;
#define FALSE (0)
#define TRUE (!FALSE)

#endif /* __NXOS_SYSTEMS_LEGOVM_LIBVM_LOCAL_BASE_TYPES_H__ */
