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
#define MARKER 42

void test_prog_u32(U32 page, U32 value);
void test_read_all(void);

void test_prog_u32(U32 page, U32 value) {
  U32 data[EFC_PAGE_WORDS];
  data[0] = value;

  nx_display_clear();

  nx_display_string("Progr. page #");
  nx_display_uint(page);
  nx_display_end_line();

  nx_display_string("<< ");
  nx_display_uint(FLASH_BASE_PTR[page*EFC_PAGE_WORDS]);
  nx_display_end_line();

  nx_display_string("|= ");
  nx_display_uint(value);
  nx_display_end_line();

  if (!nx__efc_write_page(data, page)) {
    nx_display_string("Failed!\nPress CANCEL to halt");
    while (nx_avr_get_button() != BUTTON_CANCEL);
    return;
  }

  nx_display_string(">> ");
  nx_display_uint(FLASH_BASE_PTR[page*EFC_PAGE_WORDS]);
  nx_display_end_line();
  
  nx_display_string("Press >>");
  while (nx_avr_get_button() != BUTTON_RIGHT);
  nx_systick_wait_ms(500);
}

void test_read_all(void) {
  U32 page, marker;
  U32 start;

  nx_display_clear();
  nx_display_string("Searching...\n");

  start = nx_systick_get_ms();
  
  for (page=0; page<EFC_PAGES; page++) {
    marker = FLASH_BASE_PTR[page*EFC_PAGE_WORDS];
    if (marker == MARKER) {
      nx_display_string("-> ");
      nx_display_uint(page);
      nx_display_end_line();
    }
  }

  nx_display_uint(nx_systick_get_ms() - start);
  nx_display_string(" ms.\n");
}

void main(void) {
  nx__efc_init();

  /*
  test_prog_u32(128, MARKER);
  test_prog_u32(200, MARKER);
  test_prog_u32(841, MARKER);
  test_prog_u32(1020, MARKER);
  */
  
  test_read_all();

  while (nx_avr_get_button() != BUTTON_CANCEL);
}

