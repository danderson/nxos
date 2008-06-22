/** @file array.h
 *  @brief Array management.
 */

/* Copyright (c) 2008 the NxOS developers
 *
 * See AUTHORS for a full list of the developers.
 *
 * Redistribution of this file is permitted under
 * the terms of the GNU Public License (GPL) version 2.
 */

#ifndef __NXOS_SYSTEMS_LEGOVM_LIBVM_ARRAY_H__
#define __NXOS_SYSTEMS_LEGOVM_LIBVM_ARRAY_H__

#include "base/types.h"

/** Initialize array management with default array data.
 *
 * @param dynamic_defaults A pointer to the region of the RXE file
 * containing the dynamic defaults data.
 * @param base_offset The dataspace offset to the start of dynamic
 * data. This value is equal to the size of the static dataspace.
 * @param dva_offset The dataspace offset to the dope vector array
 * describing the array layout.
 */
void lego_vm_array_init(const U8 *dynamic_defaults, const U32 base_offset,
                        const U32 dva_offset);

/** Delete an initialized array set. */
void lego_vm_array_destroy();

#endif /* __NXOS_SYSTEMS_LEGOVM_LIBVM_ARRAY_H__ */
