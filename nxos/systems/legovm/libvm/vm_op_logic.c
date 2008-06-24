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
#include "vm_op_logic.h"

/* TODO(dave): Support for logic operations over complex
 * types.
 */

void lego_vm_op_and(void) {
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
    lego_vm_u32_to_scalar(vm.instruction.arg1, A & B);
  }
}

void lego_vm_op_or(void) {
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
    lego_vm_u32_to_scalar(vm.instruction.arg1, A | B);
  }
}

void lego_vm_op_xor(void) {
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
    lego_vm_u32_to_scalar(vm.instruction.arg1, A ^ B);
  }
}

void lego_vm_op_not(void) {
  if (lego_vm_type_is_scalar(vm.dstoc[vm.instruction.arg1].type) &&
      lego_vm_type_is_scalar(vm.dstoc[vm.instruction.arg2].type)) {
    U32 A = lego_vm_scalar_to_u32(vm.instruction.arg2);
    lego_vm_u32_to_scalar(vm.instruction.arg1, ~A);
  } else {
    vm.state = CRASHED;
  }
}

