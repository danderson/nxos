/* Copyright (c) 2008 the NxOS developers
 *
 * See AUTHORS for a full list of the developers.
 *
 * Redistribution of this file is permitted under
 * the terms of the GNU Public License (GPL) version 2.
 */

#include "base/display.h"
#include "base/drivers/_efc.h"
#include "base/drivers/systick.h"
#include "base/lib/fs/fs.h"
#include "fs.h"

#define TEST_ZONE_START 128
#define TEST_ZONE_END 256

static void setup(void) {
  /* Nothing to do ? */
}

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

static void destroy(void) {
  U32 nulldata[EFC_PAGE_WORDS] = {0};
  int i;

  for (i=TEST_ZONE_START; i<TEST_ZONE_END; i++) {
    nx__efc_write_page(nulldata, i);
  }
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

void fs_test_defrag_simple(void) {
  setup();

  spawn_file("test1", 10);
  spawn_file("test2", 400);
  spawn_file("test3", 200);
  remove_file("test2");

  nx_display_string("Files created.");

  destroy();
}
