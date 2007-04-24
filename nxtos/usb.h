#ifndef __NXTOS_USB_H__
#define __NXTOS_USB_H__

#include "mytypes.h"


/* see atmel documentation, p448 */
#define MAX_ENDPOINT_SIZE 64

void usb_init();
void usb_disable();
void usb_test();

#endif

