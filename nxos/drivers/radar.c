/* Driver for the NXT ultrasonic radar.
 *
 * This driver provides a high level interface to the NXT ultrasonic
 * radar. The radar is the first digital sensor for the NXT, and thus
 * makes use of the I2C communication protocol over the two wire
 * interface of DIGIxI0 + DIGIxI1. See drivers/i2c.{c,h}.
 *
 * Among the functionnalities provided by this sensor, this driver
 * supports the following features:
 */

#include "at91sam7s256.h"

#include "mytypes.h"
#include "nxt.h"
#include "interrupts.h"
#include "systick.h"
#include "sensors.h"
#include "display.h"
#include "i2c.h"

/* As defined in the NXT Hardware Developer Kit, the Ultrasonic sensor
 * has been given address 1 (within a 7 bit context).
 */
#define RADAR_I2C_ADDRESS 0x1
#define RADAR_N_COMMANDS 29

typedef enum {
  RADAR_CMD_READ_VERSION = 0,
  RADAR_CMD_READ_PRODUCT_ID,
  RADAR_CMD_READ_SENSOR_TYPE,
  RADAR_CMD_READ_FACTORY_ZERO,
  RADAR_CMD_READ_FACTORY_SCALE_FACTOR,
  RADAR_CMD_READ_FACTORY_SCALE_DIVISOR,
  RADAR_CMD_READ_MEASUREMENT_UNITS,

  RADAR_CMD_READ_INTERVAL,
  RADAR_CMD_READ_OP_MODE,
  RADAR_CMD_READ_R0,
  RADAR_CMD_READ_R1,
  RADAR_CMD_READ_R2,
  RADAR_CMD_READ_R3,
  RADAR_CMD_READ_R4,
  RADAR_CMD_READ_R5,
  RADAR_CMD_READ_R6,
  RADAR_CMD_READ_R7,
  RADAR_CMD_READ_CURRENT_ZERO,
  RADAR_CMD_READ_CURRENT_SCALE_FACTOR,
  RADAR_CMD_READ_CURRENT_SCALE_DIVISOR,

  RADAR_CMD_OFF,
  RADAR_CMD_SINGLE_SHOT,
  RADAR_CMD_CONTINUOUS,
  RADAR_CMD_EVENT_CAPTURE,
  RADAR_CMD_RESET,
  RADAR_CMD_SET_INTERVAL,
  RADAR_CMD_SET_ACTUAL_ZERO,
  RADAR_CMD_SET_ACTUAL_SCALE_FACTOR,
  RADAR_CMD_SET_ACTUAL_SCALE_DIVISOR,
} radar_cmd;

static volatile struct radar_command
{
  U8 reg;
  U8 addr;
  U8 val;

  int result_size;
} commands[RADAR_N_COMMANDS] = {
  /* Constants */
  { 0x02, 0x00, 0x03, 8 },
  { 0x02, 0x08, 0x03, 8 },
  { 0x02, 0x10, 0x03, 8 },
  { 0x02, 0x11, 0x03, 1 },
  { 0x02, 0x12, 0x03, 1 },
  { 0x02, 0x13, 0x03, 1 },
  { 0x02, 0x14, 0x03, 7 },

  /* Variables (readings, ...) */
  { 0x02, 0x40, 0x03, 1 },
  { 0x02, 0x41, 0x03, 1 },
  { 0x02, 0x42, 0x03, 1 },
  { 0x02, 0x43, 0x03, 1 },
  { 0x02, 0x44, 0x03, 1 },
  { 0x02, 0x45, 0x03, 1 },
  { 0x02, 0x46, 0x03, 1 },
  { 0x02, 0x47, 0x03, 1 },
  { 0x02, 0x48, 0x03, 1 },
  { 0x02, 0x49, 0x03, 1 },
  { 0x02, 0x50, 0x03, 1 },
  { 0x02, 0x51, 0x03, 1 },
  { 0x02, 0x52, 0x03, 1 },

  /* Variables (readings, ...) */
  { 0x02, 0x41, 0x00, 0 },
  { 0x02, 0x41, 0x01, 0 },
  { 0x02, 0x41, 0x02, 0 },
  { 0x02, 0x41, 0x03, 0 },
  { 0x02, 0x41, 0x04, 0 },

  /* Note: in these last three commands, the 'val' field must be set
   * before issuing the command.
   */
  { 0x02, 0x40, 0x00, 0 },
  { 0x02, 0x50, 0x00, 0 },
  { 0x02, 0x51, 0x00, 0 },
  { 0x02, 0x52, 0x00, 0 },
};


void radar_init(U8 sensor)
{
  sensors_i2c_enable(sensor);
  i2c_register(sensor, RADAR_I2C_ADDRESS);
}

void radar_test(U8 sensor)
{
  struct radar_command cmd = commands[RADAR_CMD_READ_SENSOR_TYPE];
  U8 data[3] = { cmd.reg, cmd.addr, cmd.val };
  U8 buf[8] = { 0x0 };

  display_string("cmd: ");
  display_hex(data[0]);
  display_string(".");
  display_hex(data[1]);
  display_string(".");
  display_hex(data[2]);
  display_end_line();

  if (i2c_start_transaction(sensor, data, 3, TXN_MODE_WRITE) != I2C_ERR_OK) {
    display_string("err cmd");
    display_end_line();
  }

  systick_wait_ms(3000);

  display_string("res: ");
  display_uint(i2c_get_txn_status(sensor));
  display_end_line();

  if (i2c_start_transaction(sensor, buf, 8, TXN_MODE_READ) != I2C_ERR_OK) {
    display_string("err read");
    display_end_line();
  }

  systick_wait_ms(3000);

  display_string("Sensor: ");
  display_string((char *)buf);
  display_end_line();

  systick_wait_ms(2000);
}
