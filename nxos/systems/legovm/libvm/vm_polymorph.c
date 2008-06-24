/* Copyright (c) 2008 the NxOS developers
 *
 * See AUTHORS for a full list of the developers.
 *
 * Redistribution of this file is permitted under
 * the terms of the GNU Public License (GPL) version 2.
 */

#include "base/types.h"
#include "base/assert.h"

#include "vm_header.h"
#include "vm_polymorph.h"
#include "vm_types.h"

#define MASK(type) (1 << (sizeof(type) - 1))
#define SIGN_EXTEND(type, data_ptr) \
  (((type*)data)[0] ^ MASK(type)) - MASK(type)

bool lego_vm_check_type_compatibility(U32 in1, U32 in2, U32 out,
                                      bool *aggregate_types_op) {
  if (lego_vm_type_is_scalar(vm.dstoc[in1].type) &&
      lego_vm_type_is_scalar(vm.dstoc[in2].type) &&
      lego_vm_type_is_scalar(vm.dstoc[out].type)) {
    *aggregate_types_op = FALSE;
    return TRUE;
  }

  /* TODO(dave): Check type compatibility of complex aggregate
   * types.
   */
  return FALSE;
}

U32 lego_vm_scalar_to_u32(U32 idx) {
  const U8 *data = static_var_addr(idx);

  switch (vm.dstoc[idx].type) {
    /* For unsigned scalars, not much to do, just return the right
     * amount of data.
     */
  case UBYTE:
    return data[0];
  case UWORD:
    return ((U16*)data)[0];
  case ULONG:
    return ((U32*)data)[0];

    /* For signed scalars, we need to do some sign extension */
  case SBYTE:
    return SIGN_EXTEND(U8, data);
  case SWORD:
    return SIGN_EXTEND(U16, data);
  case SLONG:
    return SIGN_EXTEND(U32, data);
  }

  return 0;
}

void lego_vm_u32_to_scalar(U32 idx, U32 value) {
  U8 *data = static_var_addr(idx);

  switch (vm.dstoc[idx].type) {
  case UBYTE:
  case SBYTE:
    data[0] = (U8)(value & 0xFF);
    break;
  case UWORD:
  case SWORD:
    ((U16*)data)[0] = (U16)(value & 0xFFFF);
    break;
  case ULONG:
  case SLONG:
    ((U32*)data)[0] = value;
    break;
  }
}

void lego_vm_polymorphic_op(U32 in1, U32 in2, U32 out, operation_func op) {
  in1 = in2 = out = (U32)op;
  NX_FAIL("Not implemented");
}
