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
#include "vm_op_arithmetic.h"

/* TODO(dave): Support for arithmetic operations over complex
 * types.
 */

void lego_vm_op_add(void) {
  bool complex;
  if (!lego_vm_check_type_compatibility(vm.instruction.arg2,
                                        vm.instruction.other_args[0],
                                        vm.instruction.arg1,
                                        &complex)) {
    vm.state = CRASHED;
  } else if (complex) {
    lego_vm_polymorphic_op(vm.instruction.arg2,
                           vm.instruction.other_args[0],
                           vm.instruction.arg1,
                           NULL);
  } else {
    U32 A = lego_vm_scalar_to_u32(vm.instruction.arg2);
    U32 B = lego_vm_scalar_to_u32(vm.instruction.other_args[0]);
    lego_vm_u32_to_scalar(vm.instruction.arg1, A + B);
  }
}

void lego_vm_op_sub(void) {
  bool complex;
  if (!lego_vm_check_type_compatibility(vm.instruction.arg2,
                                        vm.instruction.other_args[0],
                                        vm.instruction.arg1,
                                        &complex)) {
    vm.state = CRASHED;
  } else if (complex) {
    lego_vm_polymorphic_op(vm.instruction.arg2,
                           vm.instruction.other_args[0],
                           vm.instruction.arg1,
                           NULL);
  } else {
    U32 A = lego_vm_scalar_to_u32(vm.instruction.arg2);
    U32 B = lego_vm_scalar_to_u32(vm.instruction.other_args[0]);
    lego_vm_u32_to_scalar(vm.instruction.arg1, A - B);
  }
}

void lego_vm_op_neg(void) {
  if (lego_vm_type_is_scalar(vm.dstoc[vm.instruction.arg1].type) &&
      lego_vm_type_is_scalar(vm.dstoc[vm.instruction.arg2].type)) {
    U32 A = lego_vm_scalar_to_u32(vm.instruction.arg2);
    lego_vm_u32_to_scalar(vm.instruction.arg1, ~A + 1);
  } else {
    vm.state = CRASHED;
  }
}

void lego_vm_op_mul(void) {
  bool complex;
  if (!lego_vm_check_type_compatibility(vm.instruction.arg2,
                                        vm.instruction.other_args[0],
                                        vm.instruction.arg1,
                                        &complex)) {
    vm.state = CRASHED;
  } else if (complex) {
    lego_vm_polymorphic_op(vm.instruction.arg2,
                           vm.instruction.other_args[0],
                           vm.instruction.arg1,
                           NULL);
  } else {
    U32 A = lego_vm_scalar_to_u32(vm.instruction.arg2);
    U32 B = lego_vm_scalar_to_u32(vm.instruction.other_args[0]);
    lego_vm_u32_to_scalar(vm.instruction.arg1, A * B);
  }
}

void lego_vm_op_div(void) {
  bool complex;
  if (!lego_vm_check_type_compatibility(vm.instruction.arg2,
                                        vm.instruction.other_args[0],
                                        vm.instruction.arg1,
                                        &complex)) {
    vm.state = CRASHED;
  } else if (complex) {
    lego_vm_polymorphic_op(vm.instruction.arg2,
                           vm.instruction.other_args[0],
                           vm.instruction.arg1,
                           NULL);
  } else {
    U32 B = lego_vm_scalar_to_u32(vm.instruction.other_args[0]);

    /* Division by zero is equal to zero in our VM. */
    if (B == 0) {
      lego_vm_u32_to_scalar(vm.instruction.arg1, 0);
      return;
    }

    bool negative = FALSE;
    U32 A = lego_vm_scalar_to_u32(vm.instruction.arg2);

    if (lego_vm_scalar_is_signed(vm.dstoc[vm.instruction.arg2].type)) {
      negative = !negative;
      A = ~A + 1;
    }
    if (lego_vm_scalar_is_signed(vm.dstoc[vm.instruction.other_args[0]].type)) {
      negative = !negative;
      B = ~B + 1;
    }

    /* This is an expensive operation, done in software. I hope it was
     * worth it.
     */
    U32 R = A / B;

    if (negative)
      R = ~R + 1;

    lego_vm_u32_to_scalar(vm.instruction.arg1, R);
  }
}

void lego_vm_op_mod(void) {
  bool complex;
  if (!lego_vm_check_type_compatibility(vm.instruction.arg2,
                                        vm.instruction.other_args[0],
                                        vm.instruction.arg1,
                                        &complex)) {
    vm.state = CRASHED;
  } else if (complex) {
    lego_vm_polymorphic_op(vm.instruction.arg2,
                           vm.instruction.other_args[0],
                           vm.instruction.arg1,
                           NULL);
  } else {
    U32 A = lego_vm_scalar_to_u32(vm.instruction.arg2);
    U32 B = lego_vm_scalar_to_u32(vm.instruction.other_args[0]);

    /* Remainder of division by zero is the dividend in our VM. */
    if (B == 0) {
      lego_vm_u32_to_scalar(vm.instruction.arg1, A);
      return;
    }

    bool negative = FALSE;

    if (lego_vm_scalar_is_signed(vm.dstoc[vm.instruction.arg2].type)) {
      negative = !negative;
      A = ~A + 1;
    }
    if (lego_vm_scalar_is_signed(vm.dstoc[vm.instruction.other_args[0]].type)) {
      negative = !negative;
      B = ~B + 1;
    }

    /* This is an expensive operation, done in software. I hope it was
     * worth it.
     */
    U32 R = A % B;

    if (negative)
      R = ~R + 1;

    lego_vm_u32_to_scalar(vm.instruction.arg1, R);
  }
}
