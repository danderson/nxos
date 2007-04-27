#ifndef __NXTOS_USB_H__
#define __NXTOS_USB_H__

#include "mytypes.h"



void usb_init();
void usb_disable();

void usb_send(const U8 *data, U32 length);

void usb_test();

#endif

