#ifndef __NXOS_TASK_H__
#define __NXOS_TASK_H__

#include "base/mytypes.h"

typedef void (*task_main_t)();

extern void run_first_task(task_main_t task, U32 *stack);

#endif /* __NXOS_TASK_H__ */
