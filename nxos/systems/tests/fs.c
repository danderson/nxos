/* Copyright (c) 2008 the NxOS developers
 *
 * See AUTHORS for a full list of the developers.
 *
 * Redistribution of this file is permitted under
 * the terms of the GNU Public License (GPL) version 2.
 */

#include "base/types.h"
#include "base/at91sam7s256.h"
#include "base/display.h"
#include "base/util.h"
#include "base/drivers/avr.h"
#include "base/drivers/_efc.h"
#include "base/drivers/systick.h"
#include "base/lib/fs/fs.h"
#include "fs.h"

#define TEST_ZONE_START 128
#define TEST_ZONE_END 256

union U32tochar {
  U32 integers[8];
  char chars[32];
};

static bool spawn_file(char *filename, size_t bytes) {
  fs_err_t err;
  fs_fd_t fd;

  err = nx_fs_open(filename, FS_FILE_MODE_CREATE, &fd);
  if (err != FS_ERR_NO_ERROR) {
    return FALSE;
  }

  for (; bytes>0; bytes--) {
    nx_fs_write(fd, 'A' + nx_systick_get_ms() % 26);
  }

  nx_fs_close(fd);
  return TRUE;
}

static bool remove_file(char *filename) {
  fs_err_t err;
  fs_fd_t fd;

  err = nx_fs_open(filename, FS_FILE_MODE_OPEN, &fd);
  if (err == FS_ERR_NO_ERROR) {
    return nx_fs_unlink(fd) == FS_ERR_NO_ERROR;
  }

  return FALSE;
}

static void cleanup(void) {
  nx_fs_soft_format();
}

static void setup(void) {
  cleanup();
}

static void destroy(void) {
  cleanup();
}


void fs_test_dump(void) {
  destroy();
  setup();
  spawn_file("test1", 10);
  spawn_file("test2", 400);
  spawn_file("test3", 200);

  nx_fs_dump();
  while (nx_avr_get_button() != BUTTON_OK);
  destroy();
}

void fs_test_infos(void) {
  U32 files = 0, used = 0, free_pages = 0, wasted = 0;

  nx_display_clear();
  nx_display_string("- FS stats -\n\n");

  nx_fs_get_occupation(&files, &used, &free_pages, &wasted);

  nx_display_uint(files);
  nx_display_string(" file(s).\n");

  nx_display_uint(used);
  nx_display_string("B used.\n");

  nx_display_uint(free_pages);
  nx_display_string(" free page(s).\n");

  nx_display_uint(wasted);
  nx_display_string("B wasted.\n");
}

void fs_test_defrag_empty(void) {
  setup();

  nx_display_clear();
  nx_display_string("Starting...\n");

  nx_display_string("Defrag: ");
  nx_fs_defrag_simple();
  nx_display_string("done.\n");

  nx_fs_dump();
  while (nx_avr_get_button() != BUTTON_OK);
  nx_systick_wait_ms(500);

  destroy();
}

void fs_test_defrag_simple(void) {
  setup();

  nx_display_clear();
  nx_display_string("Starting...\n");

  spawn_file("test1", 10);   // 128:test1 (1 page)
  spawn_file("test2", 400);  // 129:test2 (2 pages - removed)
  spawn_file("test3", 200);  // 131:test3 (1 pages)
  spawn_file("test4", 10);   // 133:test4 (1 page - removed)
  spawn_file("test5", 600);  // 134:test5 (3 pages)
  remove_file("test2");
  remove_file("test4");

  nx_fs_dump();
  while (nx_avr_get_button() != BUTTON_OK);
  nx_systick_wait_ms(500);

  nx_display_clear();
  nx_display_string("Defrag: ");
  nx_fs_defrag_simple();
  nx_display_string("done.\n");
  while (nx_avr_get_button() != BUTTON_OK);
  nx_systick_wait_ms(500);

  nx_display_clear();
  nx_fs_dump();
  while (nx_avr_get_button() != BUTTON_OK);
  nx_systick_wait_ms(500);

  destroy();
}

void fs_test_defrag_for_file(void) {
  U32 metadata[4*EFC_PAGE_WORDS] = {0};
  union U32tochar nameconv;

  setup();

  metadata[0] = (0x42 << 24);
  memcpy(nameconv.chars, (void *)"test42", 6);
  memcpy(metadata+2, nameconv.integers, 32);

  nx_display_string("Starting...\n");

  spawn_file("test1", 3000);
  //spawn_file("test2", 30000);
  //spawn_file("test3", 3000);
  //remove_file("test2");
  nx__efc_write_page(metadata, 1020);

  nx_fs_dump();
  while (nx_avr_get_button() != BUTTON_OK);
  nx_systick_wait_ms(500);

  nx_display_clear();
  nx_display_string("Defrag: ");
  nx_display_uint(nx_fs_defrag_for_file_by_name("test1"));
  nx_display_string(" done.\n");
  while (nx_avr_get_button() != BUTTON_OK);
  nx_systick_wait_ms(500);

  nx_display_clear();
  nx_fs_dump();
  while (nx_avr_get_button() != BUTTON_OK);
  nx_systick_wait_ms(500);

  destroy();
}

void fs_test_defrag_best_overall(void) {
  setup();

  nx_display_string("Starting...\n");

  spawn_file("test1", 106968);
  spawn_file("test2", 12750);
  spawn_file("test3", 106968);
  remove_file("test2");

  nx_fs_dump();
  while (nx_avr_get_button() != BUTTON_OK);
  nx_systick_wait_ms(500);

  nx_display_clear();
  nx_display_string("Defrag: ");
  nx_display_uint(nx_fs_defrag_best_overall());
  nx_display_string(" done.\n");
  while (nx_avr_get_button() != BUTTON_OK);
  nx_systick_wait_ms(500);

  nx_display_clear();
  nx_fs_dump();
  while (nx_avr_get_button() != BUTTON_OK);
  nx_systick_wait_ms(500);

  destroy();
}
