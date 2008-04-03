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

/* HACK */
#include "base/drivers/_efc.h"

#define ROUTE_FILE "tag_route.data"

void main(void) {
  U32 data[EFC_PAGE_WORDS] = {0}, num = 128;
  U32 t = (EFC_WRITE_KEY << 24) + ((num & 0x000003FF) << 8) + EFC_CMD_WP;

  nx__efc_init();

  nx_display_clear();

  nx_display_string("Progr. page #");
  nx_display_uint(num);
  nx_display_end_line();

  nx_display_string("<< ");
  nx_display_uint(FLASH_BASE_PTR[num*EFC_PAGE_WORDS]);
  nx_display_end_line();

  data[0] = FLASH_BASE_PTR[num*EFC_PAGE_WORDS]+1;
  nx_display_string("data[0] = ");
  nx_display_uint(data[0]);
  nx_display_end_line();

  nx_display_string("CMD: ");
  nx_display_hex(t);
  nx_display_end_line();
  
  if (!nx__efc_write_page(data, num)) {
    nx_display_string("Failed!\nPress CANCEL to halt");
    while (nx_avr_get_button() != BUTTON_CANCEL);
    return;
  }

  nx_display_string("Press OK to read.\n");

  while (nx_avr_get_button() != BUTTON_OK);

  nx_display_string(">> ");
  nx_display_uint(FLASH_BASE_PTR[num*EFC_PAGE_WORDS]);
  nx_display_end_line();

  while (nx_avr_get_button() != BUTTON_CANCEL);
}

