/* Copyright (C) 2007 the NxOS developers
 *
 * See AUTHORS for a full list of the developers.
 *
 * Redistribution of this file is permitted under
 * the terms of the GNU Public License (GPL) version 2.
 */

#include "base/types.h"
#include "base/display.h"
#include "base/memmap.h"
#include "base/memalloc.h"
#include "base/drivers/systick.h"
#include "base/drivers/sound.h"
#include "base/drivers/motors.h"
#include "_scheduler.h"

static void test_beep() {
  nx_sound_freq(820, 500);
}

static void test_display() {
  static U32 counter = 0;
  counter++;
  nx_display_clear();
  nx_display_cursor_set_pos(0,0);
  nx_display_uint(counter);
  nx_display_end_line();
}

static void task_spawner() {
  int i;
  for (i=0; i<10; i++) {
    mv_scheduler_create_task(test_beep, 512);
    mv_scheduler_create_task(test_display, 512);
    nx_systick_wait_ms(2000);
  }
}

void main() {
  nx_memalloc_init();
  mv__scheduler_init();
  mv_scheduler_create_task(task_spawner, 512);
  mv__scheduler_run();
}
