#include "at91sam7s256.h"
#include "types.h"
#include "lock.h"
#include "crt0.h"
#include "aic.h"
#include "sys_timer.h"
#include "twi.h"

#define NXT_AVR_TWI_ADDRESS 0x1
#define NXT_AVR_NUM_OUTPUTS 3
#define NXT_AVR_NUM_INPUTS 4
#define NXT_AVR_PWM_FREQUENCY 8

const char avr_handshake_key[] = "\xCC" "Let's samba nxt arm in arm, (c)LEGO System A/S";

static volatile enum {
  AVR_UNINITIALIZED = 0,
  AVR_INIT_HANDSHAKE = 1,
  AVR_RX_BUSY = 2,
  AVR_WAIT_TX = 3,
  AVR_TX_BUSY = 4,
} current_state = AVR_UNINITIALIZED;

static bool twi_done = FALSE;

/* The state shared between the ARM and AVR. */
static struct {
  enum {
    AVR_RUN = 0,
    AVR_POWER_OFF,
    AVR_RESET_MODE,
  } power_mode;
  S8 output_percent[NXT_AVR_NUM_OUTPUTS];
  enum {
    AVR_MOTOR_BREAK = 0,
    AVR_MOTOR_COAST,
  } output_mode;
  // TODO: Enable controlling of input power. Currently sensors are
  // forced off.
} to_avr = {
  AVR_RUN,
  { 50, 50, 50 },
  AVR_MOTOR_BREAK
};

static struct {
  bool valid; /* Set to FALSE until the first status is actually
                 transferred. */
  U16 sensor_value[NXT_AVR_NUM_INPUTS];
  U8 buttons;
  bool battery_is_aa;
  U16 battery_voltage;
  struct {
    U8 major;
    U8 minor;
  } firmware_version;
} from_avr = {
  FALSE,
  { 0, 0, 0, 0 },
  0,
  FALSE,
  0,
  { 0, 0 }
};

/* Raw data buffers for transfers to/from the AVR. */
static U8 raw_from_avr[(NXT_AVR_NUM_INPUTS * sizeof(U16)) + (5 * sizeof(U8))];
static U8 raw_to_avr[NXT_AVR_NUM_OUTPUTS + 6];

static void
avr_data_recv()
{
  memset(raw_from_avr, 0, sizeof(raw_from_avr));
  twi_read_async(NXT_AVR_TWI_ADDRESS, raw_from_avr,
                 sizeof(raw_from_avr), &twi_done);
  current_state = AVR_RX_BUSY;
}

static void
avr_data_send()
{
  /* First marshal the data into the send buffer. */
  memset(raw_to_avr, 0, sizeof(raw_to_avr));

  switch (to_avr.power_mode)
    {
    case AVR_RUN:
      raw_to_avr[1] = 8;
      break;
    case AVR_POWER_OFF:
      raw_to_avr[0] = 0x5A;
      break;
    case AVR_RESET_MODE:
      raw_to_avr[0] = 0xA5;
      raw_to_avr[1] = 0x5A;
    }
  memcpy(&raw_to_avr[2], (U8 *) to_avr.output_percent, NXT_AVR_NUM_OUTPUTS);
  raw_to_avr[6] = to_avr.output_mode;

  /* Calculate the structure checksum. The checksum value is such that
   * the sum of all byte values in the structure plus the checksum add
   * up to 255.
   */
  {
    U8 i;
    U8 checksum = 0;
    for (i=0; i<sizeof(raw_to_avr)-1; i++)
      checksum += raw_to_avr[i];
    raw_to_avr[8] = ~checksum;
  }

  /* And fire off the data. */
  twi_write_async(NXT_AVR_TWI_ADDRESS, raw_to_avr,
                  sizeof(raw_to_avr), &twi_done);
  current_state = AVR_TX_BUSY;
}

void avr_init_handshake()
{
  twi_write_async(NXT_AVR_TWI_ADDRESS, (U8 *) avr_handshake_key,
                  sizeof(avr_handshake_key)-1, &twi_done);
  current_state = AVR_INIT_HANDSHAKE;
}

void avr_1khz_update()
{
  /* If the AVR is uninitialized and the I2C bus is free, start
     handshaking. */
  switch (current_state) {
  case AVR_UNINITIALIZED:
    if (twi_is_ready())
      avr_init_handshake();
    break;
  case AVR_INIT_HANDSHAKE:
    if (twi_done)
      avr_data_send();
    break;
  case AVR_RX_BUSY:
    if (twi_done) {
      // TODO: unmarshal the data.
      from_avr.valid = FALSE;
      if (twi_is_ready())
        avr_data_send();
      else
        current_state = AVR_WAIT_TX;
    }
    break;
  case AVR_WAIT_TX:
    if (twi_is_ready())
      avr_data_send();
    break;
  case AVR_TX_BUSY:
    if (twi_done && twi_is_ready())
      avr_data_recv();
    break;
  }
}
