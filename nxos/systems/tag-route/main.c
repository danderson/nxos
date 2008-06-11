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
#include "base/drivers/avr.h"
#include "base/drivers/systick.h"
#include "base/lib/gui/gui.h"

#include "main.h"

void main(void) {
  char *entries[] = {"Replay", "Record", "From USB", "Halt", NULL};
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

  while (TRUE) {
    res = nx_gui_text_menu(menu);

    switch (res) {
      case 0:
        replay(ROUTE_FILE);
        break;
      case 1:
        record(ROUTE_FILE);
        break;
      case 2:
        usb_recv();
        break;
      case 3:
        return;
        break;
      default:
        continue;
        break;
    }

    nx_display_string("\nOk to go back");
    while (nx_avr_get_button() != BUTTON_OK);
    nx_systick_wait_ms(500);
  }
}

