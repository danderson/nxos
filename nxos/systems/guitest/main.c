/** Tag-route follower.
 *
 * Replays a recorded tag route.
 */

#include "base/types.h"
#include "base/core.h"
#include "base/display.h"
#include "base/drivers/avr.h"
#include "base/drivers/systick.h"
#include "base/lib/gui/gui.h"

void main(void) {
  char *entries[] = {"Browse", "Sysinfo", "Settings", "Format", "Item5", "Item6", "Item7", "Halt", NULL};
  gui_text_menu_t menu;
  U8 res;

  menu.entries = entries;
  menu.title = "Home menu";
  menu.active_mark = "> ";

  while (TRUE) {
    res = nx_gui_text_menu(menu);
    nx_display_end_line();

    switch (res) {
      case 7:
        nx_display_clear();
        nx_display_string(">> Halting...");
        nx_systick_wait_ms(1000);
        return;
        break;
      default:
        nx_display_clear();

        nx_display_string("You pressed:\n");
        nx_display_string(entries[res]);
        nx_display_end_line();

        nx_display_string("\nOk to go back");
        while (nx_avr_get_button() != BUTTON_OK);

        break;
    }
  }
}

