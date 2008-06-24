/** @file vm_polymorph.h
 *  @brief Implementation of generic 2-operand polymorphic operations.
 */

/* Copyright (c) 2008 the NxOS developers
 *
 * See AUTHORS for a full list of the developers.
 *
 * Redistribution of this file is permitted under
 * the terms of the GNU Public License (GPL) version 2.
 */

#ifndef __NXOS_SYSTEMS_LEGOVM_VM_POLYMORPH_H__
#define __NXOS_SYSTEMS_LEGOVM_VM_POLYMORPH_H__

#include "base/types.h"
#include "vm_types.h"

/* Check that input types for a 2 input, 1 output opcode are
 * compatible. Returns TRUE if they are, with *complex_op set to TRUE
 * if the operation involves manipulation of complex aggregate types.
 */
bool lego_vm_check_type_compatibility(U32 in1, U32 in2, U32 out,
                                      bool *aggregate_types_op);

/* Converts any scalar type to a 4 byte common representation. If the
 * origin type is a signed integer, it is sign-extended.
 */
U32 lego_vm_scalar_to_u32(U32 idx);

/* Converts a 4 byte common representation integer to any scalar
 * type. Some conversions may result in truncation or undefined
 * behavior. Don't do it.
 */
void lego_vm_u32_to_scalar(U32 idx, U32 value);

/* Type for an operation function. */
typedef U32 (*operation_func)(enum data_type, U32, enum data_type, U32);

/* Apply an operation function over complex aggregate imputs, storing
 * the result as appropriate depending on the output type.
 */
void lego_vm_polymorphic_op(U32 in1, U32 in2, U32 out, operation_func op);

#endif /* __NXOS_SYSTEMS_LEGOVM_VM_POLYMORPH_H__ */
