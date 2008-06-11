/* Copyright (c) 2008 the NxOS developers
 *
 * See AUTHORS for a full list of the developers.
 *
 * Redistribution of this file is permitted under
 * the terms of the GNU Public License (GPL) version 2.
 */

#include "base/types.h"
#include "base/display.h"
#include "base/util.h"
#include "base/drivers/usb.h"
#include "base/lib/rcmd/rcmd.h"

#include "main.h"

static void usb_readline(char *buf) {
  int i=0;

  do {
    nx_usb_read((U8 *)(buf+i), 1);
  } while (buf[i] != '\0' || buf[i] != '\n');
}

void usb_recv(void) {
  char buf[RCMD_BUF_LEN];

  nx_display_clear();
  nx_display_string("Waiting...\n");

  do {
    memset(buf, 0, RCMD_BUF_LEN);
    usb_readline(buf);
    nx_display_string(buf);
  } while (!streq(buf, "end"));

}


