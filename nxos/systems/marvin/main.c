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
#include "marvin/time.h"

mv_sem_t *beep_res;

static void beep_consumer() {
  while(1) {
    mv_semaphore_dec(beep_res);
    nx_sound_freq(820, 500);
  }
}

static void beep_producer() {
  while(1) {
    mv_time_sleep(2000);
    mv_semaphore_inc(beep_res);
  }
}

U32 sleep_iter = 0, sleep_time = 0, wakeup_time = 0;

void test_display() {
  U32 counter = 0;
  nx_display_clear();
  while(1) {
    counter++;
    nx_display_cursor_set_pos(0,0);
    nx_display_hex(counter);
    nx_display_end_line();
    nx_display_hex(sleep_iter);
    nx_display_end_line();
    nx_display_uint(sleep_time);
    nx_display_string(" / ");
    nx_display_uint(wakeup_time);
  }
}

void test_sleep() {
  while(1) {
    mv_time_sleep(100);
    sleep_time = nx_systick_get_ms();
    mv_time_sleep(100);
    wakeup_time = nx_systick_get_ms();
    sleep_iter++;
  }
}

void main() {
  nx_memalloc_init();
  mv__scheduler_init();
  beep_res = mv_semaphore_create(0);
  mv_scheduler_create_task(beep_consumer, 512);
  mv_scheduler_create_task(beep_producer, 512);
  mv_scheduler_create_task(test_display, 512);
  mv_scheduler_create_task(test_sleep, 512);
  mv__scheduler_run();
}
