/** @file vm_header.h
 *  @brief Structures and pointers to dissect RXE headers.
 */

/* Copyright (c) 2008 the NxOS developers
 *
 * See AUTHORS for a full list of the developers.
 *
 * Redistribution of this file is permitted under
 * the terms of the GNU Public License (GPL) version 2.
 */

#ifndef __NXOS_SYSTEMS_LEGOVM_LIBVM_VM_HEADER_H__
#define __NXOS_SYSTEMS_LEGOVM_LIBVM_VM_HEADER_H__

#include "base/types.h"

typedef struct __attribute__ ((__packed__)) {
  U8 magic[14]; /* Equal to "MindstormsNXT\0". */
  U16 version; /* 00 05 in big-endian, so 0x500 interpreted as a U16. */

  /*
   * Dataspace descriptor
   */
  U16 dstoc_entry_count; /* Number of variables in the DSTOC. */

  U16 ds_initial_size; /* Total initial size of the dataspace. */
  U16 ds_initial_static_size; /* Initial size, static only. */

  U16 defaults_block_size; /* Size in bytes of the defaults stream. */
  U16 defaults_dynamic_offset; /* Offset to dynamic defaults start. */
  U16 defaults_dynamic_size; /* Difference of the two above. */

  U16 memory_manager_head; /* DVA index to the head (?!) */
  U16 memory_manager_tail; /* DVA index to the tail (?!) */

  U16 dope_vector_offset; /* Initial location of DV data in the dataspace. */

  U16 clump_count; /* Number of clumps in the program. */
  U16 instruction_count; /* Number of instructions in the program. */
} rxe_header;

enum data_type {
  VOID = 0,
  UBYTE,
  SBYTE,
  UWORD,
  SWORD,
  ULONG,
  SLONG,
  ARRAY,
  CLUSTER,
  MUTEX,

  NUM_DATA_TYPES
};

extern const U8 data_type_size[NUM_DATA_TYPES];

/* DSTOC records describe variables in the executable. */
typedef struct __attribute__ ((__packed__)) {
  U8 type; /* A value from enum data_type */
  U8 is_zero_initialized;
  U16 data; /* Semantics depends on the type. */
} dstoc_record;

/* This is the dopevector type that the RXE defines. */
typedef struct __attribute__ ((__packed__)) {
  U16 offset; /* Offset in the dataspace to the array data. */
  U16 element_size; /* Size of each element. */
  U16 element_count; /* Number of elements. */

  /* The two following are things we don't care about in NxOS, but
   * that are present in the RXE.
   */
  U16 _unused;
  U16 _unused2;
} dopevector;

/* This simplified vector definition is what we actually use. */
typedef struct {
  U16 element_size; /* Size of each element. */
  U16 element_count; /* Number of elements. */
  U8 *data; /* Pointer to the array data. */
} vector;

typedef struct {
  /* These pointers are just handy handles to pieces of the RXE
   * header.
   */
  const rxe_header *header;
  const dstoc_record *dstoc;

  /* Pointer into the static dataspace initialized in RAM. */
  U8 *ds_static;

  /* Dynamic vectors initialized in RAM. */
  U32 num_arrays;
  vector *arrays;
} vm_state;

extern vm_state vm;

static inline U8 *static_var_addr(U32 entry) {
  return &vm.ds_static[vm.dstoc[entry].data];
}

#endif /* __NXOS_SYSTEMS_LEGOVM_LIBVM_VM_HEADER_H__ */
