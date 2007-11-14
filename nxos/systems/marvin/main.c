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

static U32 free_mem = 0;

static void test_beep() {
  while(1) {
    nx_sound_freq(440, 500);
    nx_systick_wait_ms(1500);
  }
}

static void test_display() {
  U32 counter = 0;
  while(1) {
    counter++;
    if (!counter)
      nx_display_clear();
    nx_display_cursor_set_pos(0,0);
    nx_display_uint(counter);
    nx_display_end_line();

    nx_display_uint(free_mem);
    nx_display_string("kB used");
  }
}

static void test_motor() {
  S32 speed = 100;
  while(1) {
    nx_motors_rotate_time(0, speed, 500, TRUE);
    nx_systick_wait_ms(1000);
    speed = -speed;
  }
}

static void test_malloc() {
  U32 size = 1;
  void *mem;
  while(1) {
    mem = nx_malloc(size);
    free_mem = size; //nx_mem_used();
    nx_systick_wait_ms(1200);
    nx_free(mem);
    /* Increment by a prime modulo another
     * prime, to get an interesting cycle.
     */
    size = (size + 1300);// % 1031;
  }
}

void main() {
  nx_memalloc_init();
  mv__scheduler_init();
  mv_scheduler_create_task(test_beep, 512);
  mv_scheduler_create_task(test_display, 512);
  mv_scheduler_create_task(test_motor, 512);
  mv_scheduler_create_task(test_malloc, 512);
  mv__scheduler_run();
}
