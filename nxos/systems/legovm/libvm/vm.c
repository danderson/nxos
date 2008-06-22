/* Copyright (c) 2008 the NxOS developers
 *
 * See AUTHORS for a full list of the developers.
 *
 * Redistribution of this file is permitted under
 * the terms of the GNU Public License (GPL) version 2.
 */

#include "base/types.h"
#include "base/util.h"
#include "base/assert.h"
#include "base/lib/memalloc/memalloc.h"

#include "vm.h"
#include "vm_header.h"
#include "vm_opdecoder.h"
#include "vm_dataspace.h"
#include "vm_opcodes.h"

/* Mapping from data types to size in bytes. VOID and CLUSTER are
 * listed as size 0 because they are irrelevant for this mapping.
 */
const U8 data_type_size[NUM_DATA_TYPES] = {
  0, /* VOID */
  1, /* UBYTE */
  1, /* SBYTE */
  2, /* UWORD */
  2, /* SWORD */
  4, /* ULONG */
  4, /* SLONG */
  2, /* ARRAY */
  0, /* CLUSTER */
  4  /* MUTEX */
};

/* The actual VM state. */
vm_state vm;

static void init_clumps(void) {
  const dstoc_record *end = vm.dstoc + vm.header->dstoc_entry_count;
  const U8 *clumps_ptr = (U8*)end + vm.header->defaults_block_size;

  /* The clump segment is 16 bit aligned, so we potentially need to
   * adjust the offset.
   */
  if ((U32) clumps_ptr & 0x1)
    clumps_ptr++;

  vm.clump_records = (clump_record*)clumps_ptr;
  vm.runtime_clumps = nx_calloc(vm.header->clump_count, sizeof(clump));

  const U8 *dependencies = (U8*)(vm.clump_records + vm.header->clump_count);
  const U8 *codespace = dependencies;

  /* Iterate once through all clumps, just to determine where the heck
   * codespace is.
   */
  int i;
  const clump_record *rec;
  for (i = 0, rec = vm.clump_records;
       i < vm.header->clump_count;
       i++, rec++)
    codespace += rec->dependent_count;

  /* Same 16-bit alignment trick as above. */
  if ((U32) codespace & 0x1)
    codespace++;

  /* Now, go through the clumps for serious, initializing the runtime
   * clump data.
   */
  clump *rt_clump;
  for (i = 0, rec = vm.clump_records, rt_clump = vm.runtime_clumps;
       i < vm.header->clump_count;
       i++, rec++, rt_clump++) {
    rt_clump->fire_count = rec->fire_count;
    rt_clump->start_pc = (U16*)(codespace + rec->code_offset);
    rt_clump->current_pc = rt_clump->start_pc;
    rt_clump->dependents_start = dependencies;
    dependencies += rec->dependent_count;
  }
}

bool lego_vm_init(const U8 *program) {
  const U8 magic[14] = "MindstormsNXT";
  const U16 version = 0x500;

  vm.header = (const rxe_header*) program;
  if (!streq((char*) vm.header->magic, (char*) magic) ||
      vm.header->version != version)
    return FALSE;

  vm.dstoc = (const dstoc_record*) (program + sizeof(rxe_header));
  vm.ds_static = nx_malloc(vm.header->ds_initial_static_size);

  lego_vm_init_dataspace();
  init_clumps();

  return TRUE;
}

void lego_vm_run(void) {
  for (U32 i = 0; i < vm.header->clump_count; i++) {
    clump *cl = &vm.runtime_clumps[i];
    if (cl->fire_count == 0) {
      cl->current_pc = lego_vm_decode_instruction(cl->current_pc);
      lego_vm_exec_opcode();
    }
  }
}

void lego_vm_destroy(void) {
  if (vm.ds_static != NULL)
    nx_free(vm.ds_static);

  if (vm.num_arrays > 0)
    /* TODO(dave): Free sub-arrays properly. */
    nx_free(vm.arrays);

  if (vm.runtime_clumps != NULL)
    nx_free(vm.runtime_clumps);

  memset(&vm, 0, sizeof(vm));
}
