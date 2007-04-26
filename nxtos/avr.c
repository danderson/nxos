#include "mytypes.h"
#include "nxt.h"
#include "systick.h"
#include "twi.h"
#include "avr.h"
#include "util.h"

#define AVR_ADDRESS 1

const char avr_brainwash_string[] =
  "\xCC" "Let's samba nxt arm in arm, (c)LEGO System A/S";

static volatile U32 avr_initialised;
static volatile bool tx_done = FALSE;

static volatile struct {
  enum {
    AVR_RUN = 0,
    AVR_POWER_OFF,
    AVR_RESET_MODE,
  } power_mode;
  S8 motor_speed[NXT_N_MOTORS];
  U8 motor_brake;

  /* TODO: enable controlling of input power. Currently everything is
   * forced off.
   */
} to_avr = { AVR_RUN, { 0, 0, 0 }, 0 };

static volatile struct {
  U16 adc_value[NXT_N_SENSORS];
  U16 buttons;
  U16 battery_is_AA;
  U16 battery_mV;
  U8 avr_fw_version_major;
  U8 avr_fw_version_minor;
} io_from_avr;

static U8 data_from_avr[(2 * NXT_N_SENSORS) + 5];

static U8 raw_to_avr[1 + /* Power mode    */
                     1 + /* PWM frequency */
                     4 + /* output % for the 4 (?!)  motors */
                     1 + /* Output modes (brakes) */
                     1 + /* Input modes (sensor power) */
                     1]; /* Checksum */


static void
avr_start_read() {
  memset(data_from_avr, 0, sizeof(data_from_avr));
  twi_read_async(AVR_ADDRESS, data_from_avr,
                 sizeof(data_from_avr), (bool*)&tx_done);
}

static void
avr_start_send() {
  int i;
  U8 checksum = 0;

  memset(raw_to_avr, 0, sizeof(raw_to_avr));

  /* Marshal the power mode configuration. */
  switch (to_avr.power_mode) {
  case AVR_RUN:
    /* Normal operating mode. First byte is 0, and the second (PWM
     * frequency is set to 8.
     */
    raw_to_avr[1] = 8;
    break;
  case AVR_POWER_OFF:
    /* Tell the AVR to shut us down. */
    raw_to_avr[0] = 0x5A;
    raw_to_avr[1] = 0;
    break;
  case AVR_RESET_MODE:
    /* Tell the AVR to boot SAM-BA. */
    raw_to_avr[0] = 0x5A;
    raw_to_avr[1] = 0xA5;
  }

  /* Marshal the motor speed settings. */
  raw_to_avr[2] = to_avr.motor_speed[0];
  raw_to_avr[3] = to_avr.motor_speed[1];
  raw_to_avr[4] = to_avr.motor_speed[2];

  /* raw_to_avr[5] is the value for the 4th motor, which doesn't
   * exist. This is probably a bug in the AVR firmware, but it is
   * required. So we just latch the value to zero.
   */

  /* Marshal the motor brake settings. */
  raw_to_avr[6] = to_avr.motor_brake;

  /* Calculate the checksum. */
  for (i=0; i<(sizeof(raw_to_avr)-1); i++)
    checksum += raw_to_avr[i];
  raw_to_avr[sizeof(raw_to_avr)-1] = ~checksum;

  twi_write_async(AVR_ADDRESS, raw_to_avr, sizeof(raw_to_avr), (bool*)&tx_done);
}

void
avr_power_down() {
  while (1)
    to_avr.power_mode = AVR_POWER_OFF;
}


void
avr_firmware_update_mode() {
  while (1)
    to_avr.power_mode = AVR_RESET_MODE;
}

void
avr_link_init() {
  twi_write_async(AVR_ADDRESS, (U8*)avr_brainwash_string,
                  sizeof(avr_brainwash_string)-1, (bool*)&tx_done);
}


static U16
Unpack16(const U8 *x)
{
  U16 retval;

  retval = (((U16) (x[0])) & 0xff) | ((((U16) (x[1])) << 8) & 0xff00);
  return retval;
}

static void
avr_unpack()
{
  U8 check_sum;
  U8 *p;
  U16 buttonsVal;
  U32 voltageVal;
  int i;

  p = data_from_avr;

  for (check_sum = i = 0; i < sizeof(data_from_avr); i++) {
    check_sum += *p;
    p++;
  }

  if (check_sum != 0xff) {
    return;
  }

  p = data_from_avr;

  // Marshall
  for (i = 0; i < NXT_N_SENSORS; i++) {
    io_from_avr.adc_value[i] = Unpack16(p);
    p += 2;
  }

  buttonsVal = Unpack16(p);
  p += 2;


  io_from_avr.buttons = 0;

  if (buttonsVal > 1023) {
    io_from_avr.buttons |= 1;
    buttonsVal -= 0x7ff;
  }

  if (buttonsVal > 720)
    io_from_avr.buttons |= 0x08;
  else if (buttonsVal > 270)
    io_from_avr.buttons |= 0x04;
  else if (buttonsVal > 60)
    io_from_avr.buttons |= 0x02;

  voltageVal = Unpack16(p);

  io_from_avr.battery_is_AA = (voltageVal & 0x8000) ? 1 : 0;
  io_from_avr.avr_fw_version_major = (voltageVal >> 13) & 3;
  io_from_avr.avr_fw_version_minor = (voltageVal >> 10) & 7;


  // Figure out voltage
  // The units are 13.848 mV per bit.
  // To prevent fp, we substitute 13.848 with 14180/1024

  voltageVal &= 0x3ff;		// Toss unwanted bits.
  voltageVal *= 14180;
  voltageVal >>= 10;
  io_from_avr.battery_mV = voltageVal;

}


void
avr_init()
{
  twi_init();

  avr_initialised = 1;
}

static U32 update_count;
static U32 link_running;

void
avr_1kHz_update()
{
  if (!avr_initialised)
    return;

  if (!link_running) {
    memset(data_from_avr, 0, sizeof(data_from_avr));
    link_running = 1;
    avr_link_init();
    update_count = 0;
    return;
  }

  if (!tx_done)
    return;

  if (twi_ready()) {
    if (update_count & 1) {
      avr_start_read();
    } else {
      avr_unpack();
      avr_start_send();
    }
    update_count++;
  }
}

U32
avr_buttons_get()
{
  return io_from_avr.buttons;
}

U32
avr_battery_voltage()
{
  return io_from_avr.battery_mV;
}

U32
avr_sensor_adc(U32 n)
{
  if (n < 4)
    return io_from_avr.adc_value[n];
  else
    return 0;
}


void
avr_set_motor(U32 n, int power_percent, int brake)
{
  if (n < NXT_N_MOTORS) {
    to_avr.motor_speed[n] = power_percent;
    if (brake)
      to_avr.motor_brake |= (1 << n);
    else
      to_avr.motor_brake &= ~(1 << n);
  }
}
