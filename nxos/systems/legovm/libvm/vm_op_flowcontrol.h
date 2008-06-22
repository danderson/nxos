/** @file vm_op_flowcontrol.h
 *  @brief Implementation of flow control opcodes.
 */

/* Copyright (c) 2008 the NxOS developers
 *
 * See AUTHORS for a full list of the developers.
 *
 * Redistribution of this file is permitted under
 * the terms of the GNU Public License (GPL) version 2.
 */

#ifndef __NXOS_SYSTEMS_LEGOVM_VM_OP_FLOWCONTROL_H__
#define __NXOS_SYSTEMS_LEGOVM_VM_OP_FLOWCONTROL_H__

#include "base/types.h"

void lego_vm_op_finclump(void);
void lego_vm_op_finclumpimmed(void);

#endif /* __NXOS_SYSTEMS_LEGOVM_VM_OP_FLOWCONTROL_H__ */
