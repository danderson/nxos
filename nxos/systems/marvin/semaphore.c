/* Copyright (C) 2007 the NxOS developers
 *
 * See AUTHORS for a full list of the developers.
 *
 * Redistribution of this file is permitted under
 * the terms of the GNU Public License (GPL) version 2.
 */

#include "base/types.h"
#include "base/assert.h"
#include "base/memalloc.h"
#include "base/interrupts.h"
#include "base/display.h"

#include "list.h"
#include "_scheduler.h"

#include "semaphore.h"

struct sem_task_handle {
  mv_task_t *task;
  struct sem_task_handle *prev, *next;
};

struct mv_sem {
  S32 count; /* The number of available resources if >= 0, or the number
              * of tasks blocking on the semaphore if < 0.
              */
  struct sem_task_handle *blocked_tasks;
};

static struct sem_task_handle *make_sem_task_handle(mv_task_t *task) {
  struct sem_task_handle *h = nx_calloc(1, sizeof(*h));
  h->task = task;
  return h;
}

mv_sem_t *mv_semaphore_create(S32 count) {
  mv_sem_t *sem;

  NX_ASSERT(count >= 0);

  sem = nx_calloc(1, sizeof(*sem));
  sem->count = count;
  mv_list_init(sem->blocked_tasks);

  return sem;
}

extern struct bleh {
  mv_task_t *tasks_ready; /* All the ready tasks waiting for CPU time. */
  mv_task_t *tasks_blocked; /* Unschedulable tasks. */

  mv_task_t *task_current; /* The task currently consuming CPU. */
  mv_task_t *task_idle; /* The idle task. */
} sched_state;

void mv_semaphore_dec(mv_sem_t *sem) {
  mv_scheduler_lock();
  sem->count--;

  /* If we're beyond what the semaphore can handle, this task needs to
   * block. It will get resumed when/if the semaphore gets incremented.
   */
  if (sem->count < 0) {
    mv_task_t *current = mv_scheduler_get_current_task();
    struct sem_task_handle *h = make_sem_task_handle(current);

    /* Mark the task as blocked and enqueue it in the semaphore info. */
    mv__scheduler_task_block(current);
    mv_list_add_tail(sem->blocked_tasks, h);

    /* Finally, perform the elaborate dance required to yield to the
     * scheduler.
     */
    nx_display_cursor_set_pos(0,0);
    nx_display_hex((U32)sched_state.tasks_ready);
    nx_display_end_line();
    nx_display_hex((U32)sched_state.tasks_blocked);
    nx_interrupts_disable();
    mv_scheduler_yield(TRUE);
    nx_interrupts_enable();
  } else {
    mv_scheduler_unlock();
  }
}

void mv_semaphore_inc(mv_sem_t *sem) {
  mv_scheduler_lock();
  sem->count++;

  /* If we are/were beyond what the semaphore can handle, we need to
   * wake up one of the blocked tasks.
   */
  if (sem->count <= 0) {
    struct sem_task_handle *h = mv_list_pop_head(sem->blocked_tasks);
    mv__scheduler_task_unblock(h->task);
    nx_free(h);
  }

  mv_scheduler_unlock();
}
