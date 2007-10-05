#ifndef __NXOS_I2C_MEMORY_H__
#define __NXOS_I2C_MEMORY_H__

#include "mytypes.h"

void i2c_memory_init(U8 sensor, U8 address);
void i2c_memory_read(U8 sensor, U8 internal_address, U8 *buf, U8 size);

#endif
