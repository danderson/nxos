#ifndef __NXTOS_USB_H__
#define __NXTOS_USB_H__

#include "mytypes.h"

/******
 * The NXT can only work as a device, not as host.
 * Here, the goal of this driver is to provide a simplified mechanism
 * to exchange data over usb :
 *  - Endpoint 0 is used by the driver to control the usb connection
 *  - Endpoint 1 is used for uploads
 *  - Endpoint 2 is used for downloads
 *
 * usb_send():
 *  - If the end point is free, the function is non-blocking
 *  - If there are already some data waiting for being sent,
 *    then this function will be blocking.
 *
 * usb_can_send():
 *  - Allow to anticipate if usb_send() will be blocking or not.
 *  - Returns 1 if no, 0 if yes.
 *
 * usb_has_data():
 *  - Return the number of bytes waiting in the buffer
 *
 * usb_get_buffer():
 *  - Return a pointer to the user app buffer
 *  - This pointer is constant
 *
 * usb_flush_buffer():
 *  - Flush the current buffer
 *
 * usb_overflowed():
 *  - Data were lost since the last flush
 ******/

#define USB_BUFFER_SIZE 64 /* usb packet size */

#define USB_STATUS_NULL             0
#define USB_STATUS_INIT_STARTED     1
#define USB_STATUS_INIT_DONE        2
#define USB_STATUS_WRITED_SOMETHING 3
#define USB_STATUS_READ_SOMETHING   4
#define USB_STATUS_CRASHED         -1


void usb_init();
void usb_disable();

U8   usb_can_send();
void usb_send(U8 *data, U32 length);

U16 usb_has_data();
U8 *usb_get_buffer();
void usb_flush_buffer();
U8 usb_overloaded();

U8 usb_status();

void usb_display_debug();


#endif

