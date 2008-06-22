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
#include "vm_array.h"

static const U8 magic[14] = "MindstormsNXT";
static const U16 version = 0x500;

/* Mapping from data types to size in bytes. VOID, CLUSTER and MUTEX
 * are listed as size 0 because they are irrelevant for this
 * mapping.
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
  0  /* MUTEX */
};

/* The actual VM state. */
vm_state vm;

static void init_static_dataspace() {
  const dstoc_record *rec = vm.dstoc;
  const dstoc_record *end = vm.dstoc + vm.header->dstoc_entry_count;
  const U8 *defaults = (U8*)end;

  while (rec != end) {
    const U32 size = data_type_size[rec->type];
    switch (rec->type) {
    case VOID:
    case CLUSTER:
      rec++;
      break;

    case MUTEX:
      *((U32*)&vm.ds_static[rec->data]) = 0xFFFFFFFF;
      defaults += 4;
      rec++;
      break;

    case ARRAY:
      /* Grab the initial DV offset from the defaults stream. */
      memcpy(&vm.ds_static[rec->data], defaults, size);
      defaults += size;

      /* Skip past all records that define the inner type of the
       * array. This is only actually relevant if the inner type is
       * another array, or a cluster. Otherwise, it skips a single
       * DSTOC entry.
       */
      U32 skip = 1;
      while (skip--) {
        if (rec->type == CLUSTER)
          skip += rec->data;
        if (rec->type == ARRAY)
          skip++;
        rec++;
      }
      break;

    default:
      if (rec->is_zero_initialized) {
        memset(&vm.ds_static[rec->data], 0, size);
      } else {
        memcpy(&vm.ds_static[rec->data], defaults, size);
        defaults += size;
      }
      rec++;
    }
  }
}

bool lego_vm_init(const U8 *program) {
  vm.header = (const rxe_header*) program;

  if (!streq((char*)vm.header->magic, (char*)magic) ||
      vm.header->version != version)
    return FALSE;

  vm.dstoc = (const dstoc_record*) (program + sizeof(rxe_header));
  vm.ds_static = nx_malloc(vm.header->ds_initial_static_size);

  init_static_dataspace();
  lego_vm_array_init();
  return TRUE;
}
