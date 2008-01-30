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
#include "base/display.h"

#include "marvin/list.h"
#include "marvin/_scheduler.h"

#include "marvin/semaphore.h"

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
    mv__scheduler_task_block();
    mv_list_add_tail(sem->blocked_tasks, h);
  }

  mv_scheduler_unlock();
}

bool mv_semaphore_try_dec(mv_sem_t *sem) {
  bool success = FALSE;
  mv_scheduler_lock();
  if (sem->count > 0) {
    sem->count--;
    success = TRUE;
  }
  mv_scheduler_unlock();
  return success;
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

void mv_semaphore_destroy(mv_sem_t *sem) {
  mv_scheduler_lock();
  NX_ASSERT(sem->count >= 0);
  nx_free(sem);
  mv_scheduler_unlock();
}
