/* Copyright (c) 2008 the NxOS developers
 *
 * See AUTHORS for a full list of the developers.
 *
 * Redistribution of this file is permitted under
 * the terms of the GNU Public License (GPL) version 2.
 */

#include "base/types.h"
#include "base/display.h"
#include "base/util.h"
#include "base/lib/fs/fs.h"
#include "base/lib/gui/gui.h"

#include "main.h"

#define TEST_DATA "print hello world\nmove A,B 100 1500\nwait 2000\nplay 1500 1000 sync\nprint done"
#define DATA_SIZE strlen(TEST_DATA)

void record(char *filename) {
  fs_fd_t fd;
  fs_err_t err;
  size_t i;

  nx_display_clear();
  nx_display_string("Creating file...\n");

  err = nx_fs_open(filename, FS_FILE_MODE_CREATE, &fd);
  if (err == FS_ERR_FILE_ALREADY_EXISTS) {
    if (nx_gui_text_menu_yesno("Overwrite?")) {
      nx_display_clear();

      nx_fs_open(filename, FS_FILE_MODE_OPEN, &fd);
      if (nx_fs_unlink(fd) != FS_ERR_NO_ERROR) {
        nx_display_string("Erase error.\n");
        return;
      }

      if (nx_fs_open(filename, FS_FILE_MODE_CREATE, &fd) != FS_ERR_NO_ERROR) {
        nx_display_string("Create error.\n");
        return;
      }
    } else {
      nx_display_string("Aborting.\n");
      return;
    }
  }

  nx_display_string("File opened.\n\n");
  nx_display_string("Writing...\n");

  for (i=0; i<DATA_SIZE; i++) {
    err = nx_fs_write(fd, (U8)TEST_DATA[i]);

    if (err != FS_ERR_NO_ERROR) {
      nx_display_string("Err: ");
      nx_display_uint(err);
      nx_display_end_line();
    }
  }

  nx_display_uint(nx_fs_get_filesize(fd));
  nx_display_string("/");
  nx_display_uint(DATA_SIZE);
  nx_display_string("B written.\n");
  nx_fs_close(fd);
}


