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
#define MARKER 0x42
#define BLEH "test"

void test_prog_u32(U32 page, U32 value);
void test_read_all(void);
void test_file_operations(void);

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
    if (((marker & 0xFF000000) >> 24) == MARKER) {
      nx_display_string("-> ");
      nx_display_uint(page);
      nx_display_end_line();
    }
  }

  nx_display_uint(nx_systick_get_ms() - start);
  nx_display_string(" ms.\n");
}

void test_file_operations(void) {
  fs_err_t err;
  fs_fd_t fd;

  nx_display_string("Creating...\n");

  err = nx_fs_open("test", FS_FILE_MODE_CREATE, &fd);
  if (err != FS_ERR_NO_ERROR) {
    nx_display_string("Error: ");
    nx_display_uint(err);
    nx_display_end_line();
    return;
  }
  nx_fs_close(fd);
  
  err = nx_fs_open("ecoume", FS_FILE_MODE_CREATE, &fd);
  if (err != FS_ERR_NO_ERROR) {
    nx_display_string("Error: ");
    nx_display_uint(err);
    nx_display_end_line();
    return;
  }
  nx_fs_close(fd);
  
  nx_display_string("done.");
  while (nx_avr_get_button() != BUTTON_RIGHT);
  nx_systick_wait_ms(500);
  nx_display_clear();
  
  nx_display_string("Opening...\n");

  err = nx_fs_open("ecoume", FS_FILE_MODE_OPEN, &fd);
  if (err != FS_ERR_NO_ERROR) {
    nx_display_string("Not found?: ");
    nx_display_uint(err);
    nx_display_end_line();
    return;
  }
  
  nx_display_string("Success? ");
  nx_display_uint(fd);
  nx_display_end_line();
  
  err = nx_fs_open("test", FS_FILE_MODE_OPEN, &fd);
  if (err != FS_ERR_NO_ERROR) {
    nx_display_string("Not found?: ");
    nx_display_uint(err);
    nx_display_end_line();
    return;
  }
  
  nx_display_string("Success? ");
  nx_display_uint(fd);
  nx_display_end_line();  
  nx_display_hex(FLASH_BASE_PTR[129*EFC_PAGE_WORDS+2]);
  nx_display_end_line();
}

void main(void) {
  U16 i;
  
  nx__efc_init();
  nx_fs_init();
  
  U32 nulldata[EFC_PAGE_WORDS] = {0};
  
  for (i=128; i<140; i++)
    nx__efc_write_page(nulldata, i);

  nx_systick_wait_ms(500);
    
  test_file_operations();

  nx_systick_wait_ms(500);
  while (nx_avr_get_button() != BUTTON_OK);
  nx_display_clear();
  test_read_all();
  
  while (nx_avr_get_button() != BUTTON_CANCEL);
}

