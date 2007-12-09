/* Copyright (C) 2007 the NxOS developers
 *
 * See AUTHORS for a full list of the developers.
 *
 * Redistribution of this file is permitted under
 * the terms of the GNU Public License (GPL) version 2.
 */

#include "base/types.h"
#include "base/display.h"
#include "base/memalloc.h"
#include "base/drivers/systick.h"
#include "base/drivers/sound.h"

#include "marvin/_scheduler.h"
#include "marvin/semaphore.h"

mv_sem_t *beep_res;

static void beep_consumer() {
  while(1) {
    mv_semaphore_dec(beep_res);
    nx_sound_freq(820, 500);
  }
}

static void beep_producer() {
  while(1) {
    nx_systick_wait_ms(2000);
    mv_semaphore_inc(beep_res);
  }
}

void main() {
  nx_memalloc_init();
  mv__scheduler_init();
  beep_res = mv_semaphore_create(0);
  mv_scheduler_create_task(beep_consumer, 512);
  mv_scheduler_create_task(beep_producer, 512);
  mv__scheduler_run();
}
