/*
 * Handling of the NXT's Two Wire (I2C) Interface, which is used for
 * communication with the NXT's sensors and AVR coprocessor.
 */

#ifndef __ESTORM_TWI_H__
#define __ESTORM_TWI_H__

#include "types.h"

/* Initialize the TWI. */
void twi_init();
bool twi_is_ready();
bool twi_is_busy();
bool twi_read_async(U32 dev_id, U8 *data, U32 len, bool *done_flag);
void twi_write_async(U32 dev_id, U8 *data, U32 len, bool *done_flag);

#endif
