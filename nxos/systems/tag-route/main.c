/* Copyright (c) 2008 the NxOS developers
 *
 * See AUTHORS for a full list of the developers.
 *
 * Redistribution of this file is permitted under
 * the terms of the GNU Public License (GPL) version 2.
 */

/** Tag-route follower. Replays a recorded tag route. */

#include "base/types.h"
#include "base/core.h"
#include "base/display.h"
#include "base/util.h"
#include "base/drivers/avr.h"
#include "base/drivers/systick.h"
#include "base/drivers/motors.h"
#include "base/lib/fs/fs.h"
#include "base/lib/gui/gui.h"
#include "base/lib/rcmd/rcmd.h"

#define ROUTE_FILE "tag.data"

#define TEST_DATA "print hello world\nmove A,B 100 1500\nwait 2000\nplay 1500 1000 sync\nprint done"
#define DATA_SIZE strlen(TEST_DATA)

void record(char *filename);
void replay(char *filename);
void defrag(void);
void stats(void);

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

void replay(char *filename) {
  nx_display_clear();
  nx_rcmd_parse(filename);
}

void defrag(void) {
  nx_display_clear();
  nx_display_string("- Defrag -\n\n");
  nx_fs_defrag_best_overall();
  nx_display_string("Done.\n");

	while (nx_avr_get_button() != BUTTON_CANCEL);
}

void stats(void) {
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

void main(void) {
  char *entries[] = {"Replay", "Record", "Stats", "Defrag", "Halt", NULL};
  gui_text_menu_t menu;
  U8 res;

  /*
  U32 nulldata[EFC_PAGE_WORDS] = {0};
  for (res=128; res<140; res++)
    nx__efc_write_page(nulldata, res);
  */

  menu.entries = entries;
  menu.title = "Tag route";
  menu.active_mark = GUI_DEFAULT_TEXT_MARK;

  while ((res = nx_gui_text_menu(menu)) != 4) {
    switch (res) {
      case 0:
        replay(ROUTE_FILE);
        break;
      case 1:
        record(ROUTE_FILE);
        break;
      case 2:
        stats();
        break;
      case 3:
        defrag();
        break;
      default:
        continue;
        break;
    }

    nx_display_string("\nOk to go back");
    while (nx_avr_get_button() != BUTTON_OK);
    nx_systick_wait_ms(500);
  }

  nx_display_string(">> Halting...");
  nx_systick_wait_ms(1000);
}
