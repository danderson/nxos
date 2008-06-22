/** @file vm.h
 *  @brief The Lego virtual machine.
 */

/* Copyright (c) 2008 the NxOS developers
 *
 * See AUTHORS for a full list of the developers.
 *
 * Redistribution of this file is permitted under
 * the terms of the GNU Public License (GPL) version 2.
 */

#ifndef __NXOS_SYSTEMS_LEGOVM_VM_H__
#define __NXOS_SYSTEMS_LEGOVM_VM_H__

#include "base/types.h"

bool lego_vm_init(const U8 *program);

bool lego_vm_run(void);

void lego_vm_destroy(void);

#endif /* __NXOS_SYSTEMS_LEGOVM_VM_H__ */
