/** Tag-route follower.
 *
 * Replays a recorded tag route.
 */

#include "base/types.h"
#include "base/core.h"
#include "base/display.h"
#include "base/fs.h"
#include "base/drivers/avr.h"
#include "base/drivers/systick.h"
#include "base/drivers/motors.h"
#include "base/lib/gui/gui.h"

#define ROUTE_FILE "tag_route.data"

void record(char *filename);
void replay(char *filename);

void record(char *filename) {
  nx_display_string(filename);
}

void replay(char *filename) {
  fs_fd_t fd;
  fs_err_t err;
  
  err = nx_fs_open(filename, FS_FILE_MODE_OPEN, &fd);
  if (err != FS_ERR_NO_ERROR) {
    nx_display_string("Can't open file!\n");
    return;
  }
  
  nx_display_string("Replaying...\n");
}

void main(void) {
  char *entries[] = {"Record", "Replay", "--", "Halt", NULL};
  gui_text_menu_t menu;
  U8 res;
  
  menu.entries = entries;
  menu.title = "Tag route";
  menu.active_mark = "> ";
  
  do {
    res = nx_gui_text_menu(menu);

    switch (res) {
      case 0:
        record(ROUTE_FILE);
        break;
      case 1:
        replay(ROUTE_FILE);
        break;
      default:
        break;
    }
    
    nx_display_string("\nOk to go back");
    while (nx_avr_get_button() != BUTTON_OK);
    nx_systick_wait_ms(500);
  } while (res != 3);
  
  nx_display_string(">> Halting...");
  nx_systick_wait_ms(1000);
}
