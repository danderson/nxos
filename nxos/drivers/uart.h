#ifndef __NXOS_UART_H__
#define __NXOS_UART_H__

#include "mytypes.h"

/**
 * This driver only support the uart where the bluetooth chip is connected.
 */


/*
 * called back each time something has been readed.
 * The buffer where the read stuffs are is given in parameter.
 */
typedef void (*uart_read_callback_t)(U8 *buffer);


/*
 * the driver using an uart port must configure itself the PIO. The
 * callback will be called by the interruption function of the uart
 * driver, so it must do its work as fast as possible.
 */
void uart_init(uart_read_callback_t callback);

/*
 * Will send the data asap.
 * If the two slot in the pdc are used, this function will be blocking.
 */
void uart_write(void *data, U16 lng);

bool uart_can_write();

#endif
