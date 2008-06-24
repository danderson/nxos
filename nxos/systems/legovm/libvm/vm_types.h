/** @file vm_types.h
 *  @brief Type definitions and type manipulation helpers.
 */

/* Copyright (c) 2008 the NxOS developers
 *
 * See AUTHORS for a full list of the developers.
 *
 * Redistribution of this file is permitted under
 * the terms of the GNU Public License (GPL) version 2.
 */

#ifndef __NXOS_SYSTEMS_LEGOVM_LIBVM_VM_TYPES_H__
#define __NXOS_SYSTEMS_LEGOVM_LIBVM_VM_TYPES_H__

#include "base/types.h"

enum data_type {
  VOID = 0,
  UBYTE,
  SBYTE,
  UWORD,
  SWORD,
  ULONG,
  SLONG,
  ARRAY,
  CLUSTER,
  MUTEX,

  NUM_DATA_TYPES
};

extern const U8 data_type_size[NUM_DATA_TYPES];

static inline bool lego_vm_type_is_scalar(enum data_type type) {
  return (type != VOID && type < ARRAY);
}

/* Only returns valid results for scalar types. */
static inline bool lego_vm_scalar_is_signed(enum data_type type) {
  return (type % 2 == 0);
}

#endif /* __NXOS_SYSTEMS_LEGOVM_LIBVM_VM_TYPES_H__ */
