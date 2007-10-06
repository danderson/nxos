#ifndef __NXOS_I2C_H__
#define __NXOS_I2C_H__

#include "mytypes.h"

typedef enum {
  I2C_ERR_OK = 0,
  I2C_ERR_UNKNOWN_SENSOR,
  I2C_ERR_NOT_READY,
  I2C_ERR_DATA,
} i2c_txn_err;

typedef enum
{
  TXN_MODE_WRITE = 0,
  TXN_MODE_READ,
} i2c_txn_mode;

typedef enum
{
  TXN_STAT_SUCCESS = 0,
  TXN_STAT_UNKNOWN,
  TXN_STAT_IN_PROGRESS,
  TXN_STAT_FAILED,
} i2c_txn_status;

void i2c_init();
void i2c_register(U8 sensor, U8 address);

i2c_txn_err i2c_start_transaction(U8 sensor, U8 *data, int size,
                                  i2c_txn_mode mode, bool restart);
                                  
i2c_txn_status i2c_get_txn_status(U8 sensor);
bool i2c_busy(U8 sensor);

void i2c_isr();

#endif
