/* Additional abstraction level for I2C sensors acting as I2C remote
 * memory units.
 *
 * Reading a value in memory consists in two I2C transactions:
 *   1. a) Send device address in write mode
 *      b) Send internal address
 *   2. a) Send device address in read mode
 *      b) Read N bytes of data
 *
 * Writing a value to the memory is a bit simpler and consists in only
 * one transaction:
 *   1. a) Send device address in write mode
 *      b) Send internal address
 *      c) Send value
 */

#include "at91sam7s256.h"

#include "mytypes.h"
#include "nxt.h"
#include "sensors.h"
#include "i2c.h"

/** Initializes a remote memory unit of address 'address' on the given
 * sensor port.
 */
void i2c_memory_init(U8 sensor, U8 address)
{
  i2c_register(sensor, address);
}

static void i2c_memory_txn(U8 sensor, U8 *data, U8 size, i2c_txn_mode mode,
                           bool restart)
{
  i2c_txn_err err;
  i2c_txn_status status;

  err = i2c_start_transaction(sensor, data, size, mode, restart);
  if (err != I2C_ERR_OK) {
    i2c_log(mode == TXN_MODE_WRITE ? "> " : "< ");
    i2c_log("TXN error (");
    i2c_log_uint(err);
    i2c_log(") !\n");
  } else {
    /* Wait for the transaction to complete.
     * TODO: change from active wait to passive for scheduling.
     */
    while (i2c_busy(sensor));

    status = i2c_get_txn_status(sensor);
    if (status != TXN_STAT_SUCCESS) {
      i2c_log(mode == TXN_MODE_WRITE ? "> " : "< ");
      i2c_log("DATA error (");
      i2c_log((char *)status);
      i2c_log(")\n");
    }
  }
}


/** Reads a value at 'internal_address' in the memory unit on the
 * given sensor port and returns it in the given buffer. Size is the
 * expected returned data size in bytes. The buffer 'buf' should be
 * pre-allocated by the caller.
 */
void i2c_memory_read(U8 sensor, U8 internal_address, U8 *buf, U8 size)
{
  if (!buf || size >= I2C_MAX_TXN_SIZE)
    return;

  i2c_memory_txn(sensor, &internal_address, 1, TXN_MODE_WRITE, FALSE);
  i2c_memory_txn(sensor, buf, size, TXN_MODE_READ, TRUE);
}

void i2c_memory_write(U8 sensor, U8 internal_address, U8 *data, U8 size)
{
  // TODO
}
