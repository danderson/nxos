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

#include "vm_header.h"
#include "vm_array.h"

/* Return a pointer to data in the dynamic default stream, given its
 * theoretical position in RAM if it were copied there.
 */
static inline const U8 *real_offset(const U8 *dynamic_defaults,
                                    U32 offset) {
  return dynamic_defaults + (offset - vm.header->ds_initial_static_size);
}

void lego_vm_array_init(void) {
  const U8 *dynamic_defaults =
    (U8*)(vm.dstoc + vm.header->dstoc_entry_count) +
    vm.header->defaults_dynamic_offset;

  const dstoc_record *rec = vm.dstoc;
  const dstoc_record *end = vm.dstoc + vm.header->dstoc_entry_count;

  const dopevector *dva =
    (const dopevector*)real_offset(dynamic_defaults,
                                   vm.header->dope_vector_offset);

  /* Construct the vector array from the DVA root entry. We're
   * potentially overestimating the number of arrays here, but we'll
   * correct later.
   */
  vm.num_arrays = dva->element_count - 1;
  vm.arrays = nx_calloc(vm.num_arrays, sizeof(vector));

  /* We remap DVs as we go. This is the index of the next vector to
   * fill in.
   */
  U16 vec_index = 0;

  while (rec != end) {
    if (rec->type == ARRAY) {
      const U16 dv_index = *((U16*)&vm.ds_static[rec->data]);
      const dopevector *dv = &dva[dv_index];
      vector *vec = &vm.arrays[vec_index];

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
        memcpy(vec->data, real_offset(dynamic_defaults, dv->offset),
               vec->element_size * vec->element_count);
      }

      /* Write the offset into our vector array back into the static
       * dataspace instead of the DV index.
       */
      *((U16*)&vm.ds_static[rec->data]) = vec_index;
      vec_index++;

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
    } else {
      rec++;
    }
  }

  /* Now that all global vectors are initialized, we know exactly how
   * many vectors we have. Resize the definition array to match.
   */
  if (vec_index < vm.num_arrays) {
    vm.num_arrays = vec_index;
    vm.arrays = nx_realloc(vm.arrays, vm.num_arrays * sizeof(vector));
  }
}
