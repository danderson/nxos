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

#define ROUTE_FILE "tag_route.data"

void main(void) {
  U32 fd;

  nx_display_clear();
  nx_fs_init();

  if (nx_fs_open(ROUTE_FILE, &fd) != FS_ERR_NO_ERROR) {
    nx_display_string("Error opening data file!");
    return;
  }
  
  nx_display_string("File opened (fd:");
  nx_display_uint(fd);
  nx_display_end_line();
}

