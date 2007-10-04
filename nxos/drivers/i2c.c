/* Driver for the NXT's I2C-based sensors.
 *
 * SoftMAC I2C driver.
 */

#include "at91sam7s256.h"

#include "mytypes.h"
#include "nxt.h"
#include "interrupts.h"
#include "systick.h"
#include "aic.h"
#include "sensors.h"
#include "i2c.h"

#include "display.h"

/* The base clock frequency of the sensor I2C bus in Hz. */
#define SENSOR_I2C_SPEED 9600

static volatile struct i2c_port {
  enum {
    I2C_OFF = 0, /* Port not initialized in I2C mode. */
    I2C_RECLOCK0,
    I2C_RECLOCK1,
    I2C_IDLE,    /* No transaction in progress. */
    I2C_SEND_START_BIT0,
    I2C_SEND_START_BIT1,
    I2C_SCL_LOW,
    I2C_SAMPLE0,
    I2C_SAMPLE1,
    I2C_SEND_STOP_BIT0,
    I2C_SEND_STOP_BIT1,
  } bus_state;

  /* The connected device address on the I2C bus. */
  U8 device_addr;

  enum {
    TXN_NONE = 0,
    TXN_WAITING,
    TXN_START,
    TXN_TRANSMIT_BYTE,
    TXN_WRITE_ACK,
    TXN_READ_ACK,
    TXN_STOP,
  } txn_state;

  /** Transaction related members. */

  i2c_txn_mode txn_mode;  /* Transaction mode. See i2c.h#i2c_txn_mode. */

  U8 *data;           /* A buffer to hold the data to transmit or
                       * receive.
                       */
  int data_size;      /* Data size, in bytes. */
  int processed;      /* Sent/received bytes. */

  /* The currently transmitted byte, and the
   * position of the bit currently transmitted
   * in this byte.
   */
  U8 current_byte;
  int current_pos;

  i2c_txn_status txn_result;

} i2c_state[NXT_N_SENSORS] = {
  { I2C_OFF, 0, TXN_NONE, TXN_MODE_WRITE, NULL, 0, 0, 0, 0, TXN_STAT_UNKNOWN },
  { I2C_OFF, 0, TXN_NONE, TXN_MODE_WRITE, NULL, 0, 0, 0, 0, TXN_STAT_UNKNOWN },
  { I2C_OFF, 0, TXN_NONE, TXN_MODE_WRITE, NULL, 0, 0, 0, 0, TXN_STAT_UNKNOWN },
  { I2C_OFF, 0, TXN_NONE, TXN_MODE_WRITE, NULL, 0, 0, 0, 0, TXN_STAT_UNKNOWN },
};

/** Initializes the I2C SoftMAC driver, configures the TC (Timer Counter)
 * and set the interrupt handler.
 */
void i2c_init() {
  interrupts_disable();

  /* We need power for both the PIO controller and the first TC (Timer
   * Channel) controller.
   */
  *AT91C_PMC_PCER = (1 << AT91C_ID_PIOA) | (1 << AT91C_ID_TC0);

  /* Configure a timer to trigger interrupts for managing the I2C
   * ports.
   *
   * Disable the counter before reconfiguring it, and mask all
   * interrupt sources. Then wait for the clock to acknowledge the
   * shutdown in its status register. Reading the SR has the
   * side-effect of clearing any pending state in there.
   */
  *AT91C_TC0_CCR = AT91C_TC_CLKDIS;
  *AT91C_TC0_IDR = ~0;
  while (*AT91C_TC0_SR & AT91C_TC_CLKSTA);

  /* Configure the timer to count at a rate of MCLK/2 (24MHz), and to
   * reset on RC compare. This means the clock will be repeatedly
   * counting at 24MHz from 0 to the value in the RC register.
   */
  *AT91C_TC0_CMR = AT91C_TC_CPCTRG;
  *AT91C_TC0_RC = (NXT_CLOCK_FREQ/2)/(4*SENSOR_I2C_SPEED);

  /* Enable the timer. */
  *AT91C_TC0_CCR = AT91C_TC_CLKEN;

  /* Allow the timer to trigger interrupts and register our interrupt
   * handler.
   */
  *AT91C_TC0_IER = AT91C_TC_CPCS;
  aic_install_isr(AT91C_ID_TC0, AIC_PRIO_SOFTMAC, AIC_TRIG_EDGE, i2c_isr);

  /* Softare trigger, to get the counter going. */
  *AT91C_TC0_CCR = AT91C_TC_SWTRG;

  interrupts_enable();
}

/** Register a remote device (by its address) on the given sensor. */
void i2c_register(U8 sensor, U8 address) {
  if (sensor >= NXT_N_SENSORS || address <= 0)
    return;

  i2c_state[sensor].bus_state = I2C_IDLE;
  i2c_state[sensor].txn_state = TXN_NONE;
  i2c_state[sensor].device_addr = address;

  display_string("sensor  : ");
  display_uint(sensor);
  display_end_line();

  display_string("i2c addr: ");
  display_uint(i2c_state[sensor].device_addr);
  display_end_line();
}

/** Start an I2C transaction.
 *
 * For a write transaction, data should point to the bytes to send to
 * the sensor. For a read transaction, data should be pre-allocated
 * with enough space to receive the data from the sensor.
 *
 * Returns non-zero if an error occured.
 */
i2c_txn_err i2c_start_transaction(U8 sensor, U8 *data, int size,
                                  i2c_txn_mode mode)
{
  if (sensor >= NXT_N_SENSORS)
    return I2C_ERR_UNKNOWN_SENSOR;

  if (i2c_state[sensor].bus_state != I2C_IDLE)
    return I2C_ERR_NOT_READY;

  if (!data || !size)
    return I2C_ERR_DATA;

  /* The bus is ready to perform a transaction. Initialize the sensor
   * bus data structure.
   */
  i2c_state[sensor].txn_state = TXN_WAITING;
  i2c_state[sensor].txn_result = TXN_STAT_IN_PROGRESS;
  i2c_state[sensor].txn_mode = mode;
  i2c_state[sensor].data = data;
  i2c_state[sensor].data_size = size;

  i2c_state[sensor].processed = 0;
  i2c_state[sensor].current_byte = 0;
  i2c_state[sensor].current_pos = 0;

  return I2C_ERR_OK;
}

/** Retrieve the transaction status for the given sensor.
 */
i2c_txn_status i2c_get_txn_status(U8 sensor)
{
  if (sensor >= NXT_N_SENSORS)
    return TXN_STAT_UNKNOWN;

  return i2c_state[sensor].txn_result;
}

/** Interrupt handler. */
void i2c_isr()
{
  volatile struct i2c_port *p;
  U32 dummy;
  U32 lines = *AT91C_PIOA_PDSR;
  U32 codr = 0;
  U32 sodr = 0;
  short i;

  /* Read the TC0 status register to ack the TC0 timer and allow this
   * interrupt handler to be called again.
   */
  dummy = *AT91C_TC0_SR;

  for (i=0; i<NXT_N_SENSORS; i++) {
    volatile sensor_pins pins = sensors_get_pins(i);
    p = &i2c_state[i];

    switch (p->bus_state)
      {
      case I2C_OFF:
        /* Port is OFF, do nothing. */
        break;

      case I2C_RECLOCK0:
        /* First step of reclocking: pull SCL low. */
        codr |= pins.scl;
        p->bus_state = I2C_RECLOCK1;
        break;

      case I2C_RECLOCK1:
        /* Second and last step of reclocking: set SCL high again, and
         * retry transaction.
         */
        sodr |= pins.scl;

        if (p->txn_state == TXN_READ_ACK ||
            p->txn_state == TXN_WRITE_ACK) {
          p->bus_state = I2C_SCL_LOW;
          p->txn_state = TXN_TRANSMIT_BYTE;
        } else {
          p->bus_state = I2C_SEND_START_BIT0;
        }

        break;

      case I2C_IDLE:
        if (p->txn_state == TXN_WAITING) {
          /* A transaction is waiting. Set both pins high, just to be
           * sure, and proceed to SEND_START_BIT.
           */
          sodr |= pins.sda | pins.scl;

          p->txn_state = TXN_START;
          p->bus_state = I2C_SEND_START_BIT0;
        }

        break;

      case I2C_SEND_START_BIT0:
        if (lines & pins.sda) {
          /* Pull SDA low. */
          codr |= pins.sda;
          p->bus_state = I2C_SEND_START_BIT1;
        } else {
          /* Something is holding SDA low. Reclock until we get our data
           * line back.
           */
          p->bus_state = I2C_RECLOCK0;
        }

        break;

      case I2C_SEND_START_BIT1:
        /* Pull SCL low. */
        codr |= pins.scl;

        /* After the start bit has been sent, switch to byte sending
         * mode with the device address + mode.
         */
        p->current_byte = (p->device_addr << 1) | p->txn_mode;
        p->current_pos = 0;
        p->processed = 0;

        p->bus_state = I2C_SCL_LOW;
        p->txn_state = TXN_TRANSMIT_BYTE;
        break;

      case I2C_SCL_LOW:
        /* SCL is low. */

        switch (p->txn_state) {
        case TXN_TRANSMIT_BYTE:
          /* In write mode, it's time to set SDA to the bit
           * value we want. In read mode, let the remote device set
           * SDA.
           */
          if (p->txn_mode == TXN_MODE_WRITE) {
            if ((p->current_byte & (1 << p->current_pos))) {
              sodr |= pins.sda;
            } else {
              codr |= pins.sda;
            }

            p->current_pos++;
          }

          p->bus_state = I2C_SAMPLE0;
          break;

        case TXN_WRITE_ACK:
          if (lines & pins.sda) {
            /* SDA is high: the slave has released SDA. Pull it low
             * and reclock.
             */
            codr |= pins.sda;
            p->bus_state = I2C_RECLOCK0;
          }

          /* Stay in the same state until the slave release SDA. */
          break;

        case TXN_READ_ACK:
          if (lines & pins.sda) {
            /* SDA is still high, this is a ACK fault. Setting
             * transaction status to TXN_STAT_FAILED and sending stop
             * bit.
             */
            p->txn_result = TXN_STAT_FAILED;
            p->bus_state = I2C_SEND_STOP_BIT0;
            p->txn_state = TXN_STOP;
          } else {
            /* Otherwise, reclock to make the slave release SDA. */
            p->bus_state = I2C_RECLOCK0;
          }

          break;

        default:
          break;
        }

        break;

      case I2C_SAMPLE0:
        /* Start sampling, rising SCL. */
        sodr |= pins.scl;
        p->bus_state = I2C_SAMPLE1;
        break;

      case I2C_SAMPLE1:
        /* End sampling.  In write mode, let the remote device read
         * the bit set in I2C_SCL_LOW. In read mode, retrieve SDA
         * value and store it.
         */
        if (p->txn_mode == TXN_MODE_READ) {
          U8 value = lines & pins.sda;
          p->data[p->processed - 1] |= (value << p->current_pos);
          p->current_pos++;
        }

        if (p->current_pos == 8) {
          /* Note: processed goes from 1 to data_size and not to 0 to
           * data_size - 1 (because of the address being sent before
           * the data.
           */

          p->processed++;
          p->current_pos = 0;

          if (p->processed == p->data_size) {
            p->txn_result = TXN_STAT_SUCCESS;
            p->txn_state = TXN_STOP;
          } else {
            if (p->txn_mode == TXN_MODE_READ) {
              p->txn_state = TXN_WRITE_ACK;
            } else {
            /* Update the current_byte being processed. */
            p->current_byte = p->data[p->processed - 1];
            p->txn_state = TXN_READ_ACK;
            }
          }
        }

        /* Finally, pull SCL low. */
        codr |= pins.scl;

        p->bus_state = I2C_SCL_LOW;
        break;

      case I2C_SEND_STOP_BIT0:
        /* First, rise SCL. */
        sodr |= pins.scl;
        p->bus_state = I2C_SEND_STOP_BIT1;
        break;

      case I2C_SEND_STOP_BIT1:
        /* Finally, rise SDA. */
        sodr |= pins.sda;
        p->bus_state = I2C_IDLE;
        p->txn_state = TXN_NONE;

        display_string("txn stop");
        display_end_line();

        break;
      }

    /** Update CODR and SODR to reflect changes for this sensor's
     * pins. */
    if (codr)
      *AT91C_PIOA_CODR = codr;
    if (sodr)
      *AT91C_PIOA_SODR = sodr;
  }
}
