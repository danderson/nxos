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
#include "vm_op_arithmetic_logic.h"

/* Small frameworklet to simplify the construction of these
 * operations.
 */
static inline bool operands_are_scalar_1op(void) {
  return (lego_vm_type_is_scalar(vm.dstoc[vm.instruction.arg1].type) &&
          lego_vm_type_is_scalar(vm.dstoc[vm.instruction.arg2].type));
}

#define SCALAR_1OP(operator) {                                          \
    U32 A = lego_vm_scalar_to_u32(vm.instruction.arg2);                 \
    lego_vm_u32_to_scalar(vm.instruction.arg1, operator A);             \
  }

#define TRY_SCALAR_1OP(operator)                                        \
  if (operands_are_scalar_1op()) {                                      \
    SCALAR_1OP(operator);                                               \
    return;                                                             \
  }

#define TRIVIAL_COMPLEX_1OP(func_name, operator)                        \
  static U32 func_name(enum data_type t __attribute__ ((unused)),       \
                       U32 d) {                                         \
    return operator d;                                                 \
  }

static inline bool operands_are_scalar_2op(void) {
  if (operands_are_scalar_1op() &&
      lego_vm_type_is_scalar(vm.dstoc[vm.instruction.other_args[0]].type))
    return TRUE;
  else
    return FALSE;
}

#define SCALAR_2OP(operator) {                                          \
    U32 A = lego_vm_scalar_to_u32(vm.instruction.arg2);                 \
    U32 B = lego_vm_scalar_to_u32(vm.instruction.other_args[0]);        \
    lego_vm_u32_to_scalar(vm.instruction.arg1, A operator B);           \
  }

#define TRY_SCALAR_2OP(operator)                                        \
  if (operands_are_scalar_2op()) {                                      \
    SCALAR_2OP(operator);                                               \
    return;                                                             \
  }

#define TRIVIAL_COMPLEX_2OP(func_name, operator)                        \
  static U32 func_name(enum data_type t1 __attribute__ ((unused)),      \
                       U32 d1,                                          \
                       enum data_type t2 __attribute__ ((unused)),      \
                       U32 d2) {                                        \
    return d1 operator d2;                                              \
  }

/* TODO(dave): Support for arithmetic operations over complex
 * types.
 */

TRIVIAL_COMPLEX_2OP(complex_add, +);
void lego_vm_op_add(void) {
  TRY_SCALAR_2OP(+);
  lego_vm_polymorphic_2op(complex_add);
}

TRIVIAL_COMPLEX_2OP(complex_sub, -);
void lego_vm_op_sub(void) {
  TRY_SCALAR_2OP(-);
  lego_vm_polymorphic_2op(complex_sub);
}

TRIVIAL_COMPLEX_1OP(complex_neg, -);
void lego_vm_op_neg(void) {
  TRY_SCALAR_1OP(-);
  lego_vm_polymorphic_1op(complex_neg);
}

TRIVIAL_COMPLEX_2OP(complex_mul, *);
void lego_vm_op_mul(void) {
  TRY_SCALAR_2OP(*);
  lego_vm_polymorphic_2op(complex_mul);
}

static inline U32 complex_div(enum data_type t1, U32 d1,
                              enum data_type t2, U32 d2) {
  /* Division by zero is equal to zero in our VM. */
  if (d2 == 0)
    return 0;

  bool negative = FALSE;

  if (lego_vm_scalar_is_signed(t1)) {
    negative = !negative;
    d1 = ~d1 + 1;
  }

  if (lego_vm_scalar_is_signed(t2)) {
    negative = !negative;
    d2 = ~d2 + 1;
  }

  /* This is an expensive operation, done in software. I hope it was
   * worth it.
   */
  U32 R = d1 / d2;

  if (negative)
    return ~R + 1;
  else
    return R;
}

void lego_vm_op_div(void) {
  if (operands_are_scalar_2op()) {
    U32 A = lego_vm_scalar_to_u32(vm.instruction.arg2);
    U32 B = lego_vm_scalar_to_u32(vm.instruction.other_args[0]);
    U32 R = complex_div(vm.dstoc[vm.instruction.arg2].type, A,
                        vm.dstoc[vm.instruction.other_args[0]].type, B);
    lego_vm_u32_to_scalar(vm.instruction.arg1, R);
    return;
  }

  lego_vm_polymorphic_2op(complex_div);
}

static inline U32 complex_mod(enum data_type t1, U32 d1,
                              enum data_type t2, U32 d2) {
  /* Remainder of division by zero is equal to the dividend. */
  if (d2 == 0)
    return d1;

  bool negative = FALSE;

  if (lego_vm_scalar_is_signed(t1)) {
    negative = !negative;
    d1 = ~d1 + 1;
  }

  if (lego_vm_scalar_is_signed(t2)) {
    negative = !negative;
    d2 = ~d2 + 1;
  }

  /* This is an expensive operation, done in software. I hope it was
   * worth it.
   */
  U32 R = d1 % d2;

  if (negative)
    return ~R + 1;
  else
    return R;
}

void lego_vm_op_mod(void) {
  if (operands_are_scalar_2op()) {
    U32 A = lego_vm_scalar_to_u32(vm.instruction.arg2);
    U32 B = lego_vm_scalar_to_u32(vm.instruction.other_args[0]);
    U32 R = complex_mod(vm.dstoc[vm.instruction.arg2].type, A,
                        vm.dstoc[vm.instruction.other_args[0]].type, B);
    lego_vm_u32_to_scalar(vm.instruction.arg1, R);
    return;
  }

  lego_vm_polymorphic_2op(complex_mod);
}

TRIVIAL_COMPLEX_2OP(complex_and, &);
void lego_vm_op_and(void) {
  TRY_SCALAR_2OP(&);
  lego_vm_polymorphic_2op(complex_and);
}

TRIVIAL_COMPLEX_2OP(complex_or,  |);
void lego_vm_op_or(void) {
  TRY_SCALAR_2OP(|);
  lego_vm_polymorphic_2op(complex_or);
}

TRIVIAL_COMPLEX_2OP(complex_xor, ^);
void lego_vm_op_xor(void) {
  TRY_SCALAR_2OP(^);
  lego_vm_polymorphic_2op(complex_xor);
}

TRIVIAL_COMPLEX_1OP(complex_not, ~);
void lego_vm_op_not(void) {
  TRY_SCALAR_1OP(~);
  lego_vm_polymorphic_1op(complex_not);
}
