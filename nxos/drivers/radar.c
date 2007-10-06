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
#include "i2c_memory.h"
#include "usb.h"

/* As defined in the NXT Hardware Developer Kit, the Ultrasonic sensor
 * has been given address 1 (within a 7 bit context).
 */
#define RADAR_I2C_ADDRESS 0x1

/** Radar's internal memory addresses.
 *
 * This enum contains the radar's internal memory addresses of the
 * radar parameters and readings.
 */
typedef enum {
  RADAR_VERSION = 0x00,

  RADAR_PRODUCT_ID = 0x08,

  RADAR_SENSOR_TYPE = 0x10,
  RADAR_FACTORY_ZERO,
  RADAR_FACTORY_SCALE_FACTOR,
  RADAR_FACTORY_SCALE_DIVISOR,
  RADAR_MEASUREMENT_UNITS,

  RADAR_INTERVAL = 0x40,
  RADAR_OP_MODE,
  RADAR_R0,
  RADAR_R1,
  RADAR_R2,
  RADAR_R3,
  RADAR_R4,
  RADAR_R5,
  RADAR_R6,
  RADAR_R7,
  RADAR_CURRENT_ZERO,
  RADAR_CURRENT_SCALE_FACTOR,
  RADAR_CURRENT_SCALE_DIVISOR,
} radar_commands;

extern U32 offset;
extern U8 dump[1024];
extern bool record;

/** Initializes the radar sensor. */
void radar_init(U8 sensor)
{
  i2c_memory_init(sensor, RADAR_I2C_ADDRESS);
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

void radar_txn(U8 sensor, U8 *data, U8 size, i2c_txn_mode mode, bool restart)
{
  i2c_txn_err err;
  i2c_txn_status status;

  err = i2c_start_transaction(sensor, data, size, mode, restart);
  if (err != I2C_ERR_OK) {
    display_string(mode == TXN_MODE_WRITE ? "> " : "< ");
    display_string("TXN error (");
    display_uint(err);
    display_string(") !\n");
  } else {
    while (i2c_busy(sensor));

    status = i2c_get_txn_status(sensor);
    if (status != TXN_STAT_SUCCESS) {
      display_string(mode == TXN_MODE_WRITE ? "> " : "< ");
      display_string("DATA error (");
      display_uint(status);
      display_string(")\n");
    }
  }
}

void radar_send_dump() {
  display_string("dumping... ");

  usb_send((U8 *) (&offset), 4);
  while (usb_can_send());

  usb_send(dump, offset);
  while (usb_can_send());

  display_string("done.\n");
}

void radar_test(U8 sensor)
{
  display_clear();
  display_cursor_set_pos(0, 0);
  display_string("I2C Radar Test\n");

  U8 cmd;
  U8 product_id[8] = { 0x00 };
  U8 sensor_type[8] = { 0x00 };
  U8 version[8] = { 0x00 };

  display_string("Reading info...\n");

  /* Read product ID */
  cmd = RADAR_PRODUCT_ID;
  radar_txn(sensor, &cmd, 1, TXN_MODE_WRITE, FALSE);
  radar_txn(sensor, product_id, 8, TXN_MODE_READ, TRUE);

  /* Read sensor type */
  cmd = RADAR_SENSOR_TYPE;
  radar_txn(sensor, &cmd, 1, TXN_MODE_WRITE, FALSE);
  radar_txn(sensor, sensor_type, 8, TXN_MODE_READ, TRUE);

  /* Read sensor type */
  cmd = RADAR_VERSION;
  radar_txn(sensor, &cmd, 1, TXN_MODE_WRITE, FALSE);
  radar_txn(sensor, version, 8, TXN_MODE_READ, TRUE);

  display_string((char *)product_id);
  display_string(" ");
  display_string((char *)sensor_type);
  display_string(" ");
  display_string((char *)version);
  display_end_line();

  display_string("Done.\n");
}
