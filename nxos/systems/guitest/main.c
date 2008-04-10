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
  char *entries[3] = {"Browse", "Sysinfo", "Halt"};
  gui_text_menu_t menu;
  U8 res;

  menu.entries = entries;
  menu.title = "Home menu";
  menu.active_mark = ">";
  menu.count = 3;

  res = nx_gui_text_menu(menu);
  nx_display_end_line();

  switch (res) {
    case 2:
      nx_display_string("Halting...");
      nx_systick_wait_ms(1000);
      return;
      break;
    default:
      nx_display_string(entries[res]);
      break;
  }

  while (nx_avr_get_button() != BUTTON_CANCEL);
}

