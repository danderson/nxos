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
#include "list.h"

#include "_scheduler.h"

/* Time in milliseconds (actually in number of systick callbacks)
 * between context switches.
 */
#define TASK_SWITCH_RESOLUTION 1

/* A task descriptor. */
struct mv_task {
  U32 *stack_base; /* The stack base (allocated pointer). */
  U32 *stack_current; /* The current position of the stack pointer. */

  /* The task structure is handled as a circularly linked list, as
   * defined by list.h.
   */
  struct mv_task *next, *prev;
};

/* The state of the scheduler. */
static struct {
  struct mv_task *tasks_ready; /* All the ready tasks waiting for CPU time. */
  struct mv_task *tasks_blocked; /* Unschedulable tasks. */

  struct mv_task *task_current; /* The task currently consuming CPU. */
  struct mv_task *task_idle; /* The idle task. */
} sched_state = { NULL, NULL, NULL, NULL };

/* The scheduler lock count. This is a recursive mutex that protects
 * the data in sched_state.
 */
static U32 sched_lock = 0;

/* Commands for tasks. These are transmitted to the scheduler from the
 * task that it preempted, and lets the task request some special operations.
 */
/* static enum { */
/*   TASK_NONE = 0, */
/*   TASK_YIELD, /\* The preempted task wants to yield to another task. *\/ */
/*   TASK_DIE,   /\* The preempted tasks asked to be killed. *\/ */
/* } task_command = TASK_NONE; */

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

  /* If the scheduler state is locked, nothing can be done. */
  if (sched_lock > 0) {
    if (cnt != 0)
      cnt = (cnt + 1) % TASK_SWITCH_RESOLUTION;
    return;
  }

  /* Task switching time! */
  if (cnt == 0) {
    sched_state.task_current->stack_current = mv__task_get_stack();
    if (mv_list_is_empty(sched_state.tasks_ready)) {
      sched_state.task_current = sched_state.task_idle;
    } else {
      sched_state.task_current = mv_list_get_head(sched_state.tasks_ready);
      mv_list_rotate_forward(sched_state.tasks_ready);
    }
    mv__task_set_stack(sched_state.task_current->stack_current);
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

  mv_list_init_singleton(t, t);

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
  sched_state.task_idle = new_task(task_idle, 128);
  /* The idle task doesn't start with a rolled up task state. Rewind its
   * current stack position.
   */
  sched_state.task_idle->stack_current += sizeof(nx_task_stack_t);
  sched_state.task_current = sched_state.task_idle;
}

void mv__scheduler_run() {
  nx_interrupts_disable();
  nx_systick_install_scheduler(scheduler_cb);
  mv__task_run_first(task_idle, sched_state.task_idle->stack_current);
}

void mv_scheduler_create_task(nx_closure_t func, U32 stack) {
  mv_task_t *t = new_task(func, stack);
  mv_scheduler_lock();
  mv_list_add_tail(sched_state.tasks_ready, t);
  mv_scheduler_unlock();
}

void mv_scheduler_lock() {
  /* Because the lock prevents the scheduler from preempting the task,
   * there is no need for fancy atomic operations here.
   */
  sched_lock++;
}

void mv_scheduler_unlock() {
  sched_lock--;
}
