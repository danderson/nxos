/* Copyright (c) 2009 the NxOS developers
 *
 * See AUTHORS for a full list of the developers.
 *
 * Redistribution of this file is permitted under
 * the terms of the GNU Public License (GPL) version 2.
 */

/* Display example.
 *
 * This example demonstrates the use of the NxOS display
 * driver. Because displaying information is so useful, you've
 * probably seen a couple of its functions in use in other
 * examples. Here, we'll go into more detail, covering the whole
 * display API.
 */

#include "base/display.h"
#include "base/drivers/systick.h" /* for nx_systick_wait_ms */

void main() {
  /* The display is already initialized at this point, so let's
   * display the classic string.
   */
  nx_display_string("Hello world!");

  nx_systick_wait_ms(1000);

  /* The display works with a cursor analogy: whenever you call a
   * display function, the output is at the current location of the
   * cursor, and the cursor is advanced to point just beyond what you
   * displayed.
   *
   * You can also move the cursor by hand to a location, if you want
   * to display at a given position. Let's move down to the start of
   * the third line of the display.
   */
  nx_display_cursor_set_pos(2, 0);

  /* We can display numbers as well as strings, either in decimal or
   * hexadecimal base. We also have a function to insert a line feed,
   * to move to the next line of the display.
   */
  nx_display_uint(42);
  nx_display_end_line();
  nx_display_hex(42);

  nx_systick_wait_ms(1000);

  /* If you set the cursor over some existing output, whatever you
   * display next will overwrite just what it needs. This can be
   * useful, but you'll probably also want to clear the whole display
   * at some point.
   */
  nx_display_clear();

  /* The display is double-buffered. That is, you output data into one
   * buffer, and then transfer the contents of that buffer into the
   * LCD memory, where it actually gets displayed.
   *
   * By default, the screen is in auto-refresh mode: whenever you
   * output something to the display, the driver will make sure that
   * it is sent to the LCD screen. Essentially, you shouldn't even
   * realize that the screen is double-buffered.
   *
   * If you want more control over when the display is refreshed, you
   * can disable auto-refresh mode.
   */
  nx_display_auto_refresh(FALSE);

  /* The display is now in manual refresh mode. We can output anything
   * we want, but it won't get displayed until we explicitely request
   * a screen refresh.
   */
  nx_display_string("Hello again!");

  /* Nothing displayed for 1s... */
  nx_systick_wait_ms(1000);

  /* We request a refresh, and the message appears. */
  nx_display_refresh();

  nx_systick_wait_ms(1000);
}
