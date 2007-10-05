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

/** Reads a value at 'internal_address' in the memory unit on the
 * given sensor port and returns it in the given buffer. Size is the
 * expected returned data size in bytes. The buffer 'buf' should be
 * pre-allocated by the caller.
 */
void i2c_memory_read(U8 sensor, U8 internal_address, U8 *buf, U8 size)
{

}
