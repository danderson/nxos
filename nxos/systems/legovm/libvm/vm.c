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

static const U8 magic[14] = "MindstormsNXT";
static const U16 version = 0x500;

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

static inline const U8 *get_dynamic_offset(const U8 *defaults,
                                           const U16 offset) {
  return defaults + (offset - vm.header->ds_initial_static_size);
}

static inline const dopevector *get_dopevector(const U8 *defaults,
                                               const U16 offset) {
  return (dopevector*) get_dynamic_offset(defaults, offset);
}

static const dstoc_record *init_array(const dstoc_record *rec,
                                      const U8 *dynamic_defaults,
                                      const dopevector *dva,
                                      const U16 dopevector_index,
                                      vector *vec) {
  const dopevector *dv = &dva[dopevector_index];

  /* Arrays of arrays are treated specially, since we directly
   * store vector structs in that case.
   */
  if (rec[1].type == ARRAY) {
    vec->element_size = sizeof(vector);
  } else {
    vec->element_size = dv->element_size;
  }
  vec->element_count = dv->element_count;
  if (vec->element_count > 0) {
    NX_ASSERT(rec[1].type != ARRAY);
    vec->data = nx_malloc(vec->element_size * vec->element_count);
    memcpy(vec->data, get_dynamic_offset(dynamic_defaults, dv->offset),
           vec->element_size * vec->element_count);
  }

  /* Write the offset into our vector array into the static dataspace
   * instead of the DV index.
   */
  *((U16*)&vm.ds_static[rec->data]) = vec - vm.arrays;

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

  return rec;
}

static void init_static_dataspace() {
  const dstoc_record *rec = vm.dstoc;
  const dstoc_record *end = vm.dstoc + vm.header->dstoc_entry_count;
  const U8 *static_defaults = (U8*)end;
  const U8 *dynamic_defaults =
    static_defaults + vm.header->defaults_dynamic_offset;
  const dopevector *dva = get_dopevector(dynamic_defaults,
                                         vm.header->dope_vector_offset);

  /* Construct the vector array from the DVA root entry. */
  vm.num_arrays = dva->element_count - 1;
  vm.arrays = nx_calloc(vm.num_arrays, sizeof(vector));

  vector *vec = vm.arrays;

  while (rec != end) {
    const U32 size = data_type_size[rec->type];

    switch (rec->type) {
    case VOID:
    case CLUSTER:
      rec++;
      break;

    case ARRAY:
      rec = init_array(rec, dynamic_defaults, dva,
                       *((U16*)static_defaults), vec++);
      static_defaults += size;
      break;

    default:
      if (rec->is_zero_initialized) {
        memset(&vm.ds_static[rec->data], 0, size);
      } else {
        memcpy(&vm.ds_static[rec->data], static_defaults, size);
        static_defaults += size;
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
  return TRUE;
}
