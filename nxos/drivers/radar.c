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
#include "avr.h"
#include "interrupts.h"
#include "systick.h"
#include "sensors.h"
#include "display.h"
#include "i2c.h"
#include "usb.h"

/* As defined in the NXT Hardware Developer Kit, the Ultrasonic sensor
 * has been given address 1 (within a 7 bit context).
 */
#define RADAR_I2C_ADDRESS 0x1

typedef enum {
  RADAR_CMD_READ_VERSION = 0x00,

  RADAR_CMD_READ_PRODUCT_ID = 0x08,

  RADAR_CMD_READ_SENSOR_TYPE = 0x10,
  RADAR_CMD_READ_FACTORY_ZERO,
  RADAR_CMD_READ_FACTORY_SCALE_FACTOR,
  RADAR_CMD_READ_FACTORY_SCALE_DIVISOR,
  RADAR_CMD_READ_MEASUREMENT_UNITS,

  RADAR_CMD_READ_INTERVAL = 0x40,
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
} radar_commands;

extern U32 offset;
extern U8 dump[2048];

void radar_init(U8 sensor)
{
  sensors_i2c_enable(sensor);
  i2c_register(sensor, RADAR_I2C_ADDRESS);
}


void radar_display_lines(U8 sensor)
{
  U32 lines = *AT91C_PIOA_PDSR;

  sensor_pins pins = sensors_get_pins(sensor);
  display_string("[");
  display_uint(lines & pins.sda ? 1 : 0);
  display_string("/");
  display_uint(lines & pins.scl ? 1 : 0);
  display_string("]\n");
}

void radar_test(U8 sensor)
{
  i2c_txn_status status;
  i2c_txn_err err;
  U8 cmd = RADAR_CMD_READ_SENSOR_TYPE;
  U8 buf[8] = { 0x0 };

  /* Send the command READ_SENSOR_TYPE */
  display_clear();
  display_cursor_set_pos(0, 0);
  display_string(">> send command\n");
  err = i2c_start_transaction(sensor, &cmd, 1, TXN_MODE_WRITE);
  if (err != I2C_ERR_OK) {
    display_string("EE txn (");
    display_uint(err);
    display_string(") !\n");
  } else {
    while (i2c_get_txn_status(sensor) == TXN_STAT_IN_PROGRESS);
    systick_wait_ms(50);
    status = i2c_get_txn_status(sensor);
    if (status == TXN_STAT_SUCCESS)
      display_string("OK ");
    else {
      display_string("EE (");
      display_uint(status);
      display_string(") ");
    }
  }

  radar_display_lines(sensor);
  while (avr_get_button() != BUTTON_OK);

  display_clear();
  display_cursor_set_pos(0, 0);

  display_string("<< read result\n");
  err = i2c_start_transaction(sensor, buf, 6, TXN_MODE_READ);
  if (err != I2C_ERR_OK) {
    display_string("EE txn (");
    display_uint(err);
    display_string(") !\n");
  } else {
    while (i2c_get_txn_status(sensor) == TXN_STAT_IN_PROGRESS);
    systick_wait_ms(50);
    status = i2c_get_txn_status(sensor);
    if (status == TXN_STAT_SUCCESS) {
      display_string("OK ");
    }

    else {
      display_string("EE (");
      display_uint(status);
      display_string(") ");
    }
  }

  radar_display_lines(sensor);

  display_string("  -+- \"");
  display_string((char *)buf);
  display_string("\" -+-\n");

  while(avr_get_button() != BUTTON_CANCEL);
}
