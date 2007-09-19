#ifndef __NXOS_I2C_H__
#define __NXOS_I2C_H__

#include "mytypes.h"

void i2c_init();
void i2c_register(U8 sensor, U8 address);
void i2c_isr();

#endif
