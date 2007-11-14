/* Copyright (C) 2007 the NxOS developers
 *
 * See AUTHORS for a full list of the developers.
 *
 * Redistribution of this file is permitted under
 * the terms of the GNU Public License (GPL) version 2.
 */

#include "base/core.h"
#include "base/memalloc.h"
#include "base/assert.h"
#include "base/interrupts.h"
#include "base/display.h"
#include "base/drivers/systick.h"
#include "base/drivers/avr.h"

#include "_task.h"

#include "_scheduler.h"

/* Time in milliseconds (actually in number of systick callbacks)
 * between context switches.
 */
#define TASK_SWITCH_RESOLUTION 1

/* A task descriptor. */
struct mv_task {
  U32 *stack_base; /* The stack base (allocated pointer). */
  U32 *stack_current; /* The current position of the stack pointer. */
  struct mv_task *next; /* Pointer to the next task. */
};

/* The list of tasks that are available for running. */
mv_task_t *available_tasks = NULL;

/* The currently running task. */
mv_task_t *current_task = NULL;

/* The idle task. This one is only executed when no other tasks can
 * run. It is also the first task that the scheduler fires up on
 * bootup.
 */
mv_task_t *idle_task = NULL;

/* This is where most of the magic happens. This function gets called
 * every millisecond to make a scheduling decision.
 */
static void scheduler_cb() {
  /* We want to schedule every TASK_SWITCH_RESOLUTION ms, so we keep a
   * counter to decide when to schedule.
   */
  static int cnt = 0;

  /* Security mechanism: in case the system crashes, as long as the
   * scheduler is still running, the brick can be powered off.
   */
  if (nx_avr_get_button() == BUTTON_CANCEL)
    nx_core_halt();

  if (cnt == 0) {
    current_task->stack_current = mv__task_get_stack();
    if (current_task == idle_task && available_tasks != NULL)
      current_task = available_tasks;
    else if (current_task->next == NULL)
      current_task = available_tasks;
    else
      current_task = current_task->next;
    mv__task_set_stack(current_task->stack_current);
  }

  cnt = (cnt + 1) % TASK_SWITCH_RESOLUTION;
}

static mv_task_t *new_task(nx_closure_t func, U32 stack_size) {
  mv_task_t *t;
  nx_task_stack_t *s;

  NX_ASSERT_MSG((stack_size & 0x3) == 0, "Stack must be\n4-byte aligned");

  t = nx_calloc(1, sizeof(*t));
  t->stack_base = nx_calloc(1, stack_size);
  t->stack_current = t->stack_base + (stack_size >> 2) - sizeof(*s);
  s = (nx_task_stack_t*)t->stack_current;
  s->pc = (U32)func;
  s->cpsr = 0x1F; /* TODO: Nice define. */
  if (s->pc & 0x1) {
    s->pc &= 0xFFFFFFFE;
    s->cpsr |= 0x20;
  }

  return t;
}

static void task_idle() {
  /* The idle task is where the scheduler first starts up, with
   * interrupt handling disabled. So we reenable it before getting on
   * with out Important Work: doing nothing.
   */
  nx_interrupts_enable();
  while(1);
}

void mv__scheduler_init() {
  idle_task = new_task(task_idle, 128);
  /* The idle task doesn't start with a rolled up task state. Rewind its
   * current stack position.
   */
  idle_task->stack_current += sizeof(nx_task_stack_t);
  current_task = idle_task;
}

void mv__scheduler_run() {
  nx_interrupts_disable();
  nx_systick_install_scheduler(scheduler_cb);
  mv__task_run_first(task_idle, idle_task->stack_current);
}

void mv_scheduler_create_task(nx_closure_t func, U32 stack) {
  mv_task_t *t = new_task(func, stack);
  t->next = available_tasks;
  available_tasks = t;
}
