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
#include "base/drivers/systick.h"
#include "base/drivers/usb.h"
#include "base/lib/fs/fs.h"
#include "base/lib/rcmd/rcmd.h"

#include "main.h"

static void usb_readline(U8 *buf) {
  size_t len;
  int i = 0;

  nx_usb_read(buf, RCMD_BUF_LEN*sizeof(char));
  for (i=0; i<10 && !nx_usb_data_read(); i++) {
    nx_systick_wait_ms(200);
  }

  if (i >= 10) {
    return;
  }

  len = nx_usb_data_read();
  if (len+1 < RCMD_BUF_LEN) {
    buf[len+1] = '\0';
  } else {
    buf[RCMD_BUF_LEN-1] = '\0';
  }
}

void usb_recv(void) {
  U8 buf[RCMD_BUF_LEN];

  nx_display_clear();
  nx_display_string("<< ");

  do {
    memset(buf, 0, RCMD_BUF_LEN);
    usb_readline(buf);

    if (!*buf) {
      continue;
    }

    nx_display_string((char *)buf);
    nx_display_end_line();

    nx_rcmd_do((char *)buf);
    nx_systick_wait_ms(50);
    nx_display_string("<< ");
  } while (!streq((char *)buf, "end"));
}

fs_err_t usb_recv_to(fs_fd_t fd) {
  U8 buf[RCMD_BUF_LEN];
  fs_err_t err;
  size_t i;

  do {
    memset(buf, 0, RCMD_BUF_LEN);
    usb_readline(buf);

    if (!*buf) {
      continue;
    }

    for (i=0; i<strlen((char *)buf); i++) {
      err = nx_fs_write(fd, buf[i]);
      if (err != FS_ERR_NO_ERROR) {
        return err;
      }
    }

    err = nx_fs_write(fd, (U8)'\n');
    if (err != FS_ERR_NO_ERROR) {
      return err;
    }

    nx_rcmd_do((char *)buf);
    nx_systick_wait_ms(50);
  } while (!streq((char *)buf, "end"));

  return FS_ERR_NO_ERROR;
}


