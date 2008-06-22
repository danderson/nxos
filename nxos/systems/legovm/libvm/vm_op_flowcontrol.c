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
#include "vm_op_flowcontrol.h"

static inline void stop_current_clump(void) {
  vm.current_clump->fire_count = ~0;
  vm.num_active_clumps--;
}

static inline void fire_clump(const U32 clump_idx) {
  NX_ASSERT(vm.runtime_clumps[clump_idx].fire_count > 0);

  if (--vm.runtime_clumps[clump_idx].fire_count == 0)
    vm.num_active_clumps++;
}

void lego_vm_op_finclump(void) {
  stop_current_clump();

  if (vm.instruction.arg1 > 255 || vm.instruction.arg2 > 255)
    return;

  for (U32 i = vm.instruction.arg1; i <= vm.instruction.arg2; i++)
    fire_clump(vm.current_clump->dependents_start[i]);
}

void lego_vm_op_finclumpimmed(void) {
  stop_current_clump();

  fire_clump(vm.instruction.arg1);
}
