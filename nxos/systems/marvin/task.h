#ifndef __NXOS_TASK_H__
#define __NXOS_TASK_H__

#include "base/mytypes.h"

extern void run_first_task(void (task)(void), U32 *stack);
extern U32 *swap_task_stacks(U32 *new_stack);
extern volatile U32 *get_system_stack();
extern U32 get_cpsr();

#endif /* __NXOS_TASK_H__ */
