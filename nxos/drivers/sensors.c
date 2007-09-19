/* Driver for the NXT's sensors.
 *
 * This driver provides the basic building blocks for the specialized
 * sensor drivers: Initial conversion of the A/D values read from the
 * sensors, a SoftMAC I2C driver for the sensor digital I/O, and ways
 * to register logic for each sensor port.
 */

#include "at91sam7s256.h"

#include "mytypes.h"
#include "nxt.h"
#include "interrupts.h"
#include "systick.h"
#include "aic.h"
#include "avr.h"
#include "sensors.h"


static volatile struct {
  /* The mode of the sensor port. */
  enum {
    OFF = 0, /* Unused. */
    LEGACY,  /* Legacy RCX sensor support (Not currently supported). */
    ANALOG,  /* NXT sensor in analog mode. */
    DIGITAL, /* NXT sensor in digital (i2c) mode. */
  } mode;

  /* Pins wired to this sensor. */
  sensor_pins pins;
} sensors_state[NXT_N_SENSORS] = {
  { OFF, { AT91C_PIO_PA23, AT91C_PIO_PA18 }},
  { OFF, { AT91C_PIO_PA28, AT91C_PIO_PA19 }},
  { OFF, { AT91C_PIO_PA29, AT91C_PIO_PA20 }},
  { OFF, { AT91C_PIO_PA30, AT91C_PIO_PA2  }},
};


void sensors_init() {
  U32 sensor_sda_mask = 0;
  U32 sensor_scl_mask = 0;
  int i;

  for (i=0; i<NXT_N_SENSORS; i++) {
    sensor_sda_mask |= sensors_state[i].pins.sda;
    sensor_scl_mask |= sensors_state[i].pins.scl;
  }

  /* Disable output on all DIGI0/1 pins, which will set the lines high
   * due to the internal PIO pull-up resistor. We will keep the lines
   * in this idle state until a sensor driver tells us what to do with
   * the lines.
   */
  *AT91C_PIOA_PER = sensor_sda_mask | sensor_scl_mask;
  *AT91C_PIOA_ODR = sensor_sda_mask | sensor_scl_mask;
}

/** Returns a pointer to this sensor's pins info structure. */
sensor_pins sensors_get_pins(U8 sensor) {
  if (sensor >= NXT_N_SENSORS) {
    sensor_pins empty = { 0, 0 };
    return empty;
  }
  
  return sensors_state[sensor].pins;
}

void sensors_analog_digi_set(U8 sensor, sensor_data_pin pin) {
  /* The DIGI pins can be manually controlled only when in analog
   * mode.
   */
  if (sensor >= NXT_N_SENSORS || sensors_state[sensor].mode != ANALOG)
    return;

  *AT91C_PIOA_SODR = (pin == DIGI1 ? sensors_state[sensor].pins.sda :
                      sensors_state[sensor].pins.scl);
}


void sensors_analog_digi_clear(U8 sensor, sensor_data_pin pin) {
  /* The DIGI pins can be manually controlled only when in analog
   * mode.
   */
  if (sensor >= NXT_N_SENSORS || sensors_state[sensor].mode != ANALOG)
    return;

  *AT91C_PIOA_CODR = (pin == DIGI1 ? sensors_state[sensor].pins.sda :
                      sensors_state[sensor].pins.scl);
}


U32 sensors_analog_get(U8 sensor) {
  if (sensor >= NXT_N_SENSORS || sensors_state[sensor].mode != ANALOG)
    return 0;

  return avr_get_sensor_value(sensor);
}

void sensors_analog_enable(U8 sensor) {
  if (sensor >= NXT_N_SENSORS)
    return;

  if (sensors_state[sensor].mode != OFF)
    sensors_disable(sensor);

  sensors_state[sensor].mode = ANALOG;

  /* In analog mode, the DIGI outputs are driven low. */
  *AT91C_PIOA_OER = (sensors_state[sensor].pins.sda |
                     sensors_state[sensor].pins.scl);
  *AT91C_PIOA_CODR = (sensors_state[sensor].pins.sda |
                      sensors_state[sensor].pins.scl);
}

/** Enables the given sensor in digital, I2C mode. */
void sensors_i2c_enable(U8 sensor) {
  if (sensor >= NXT_N_SENSORS)
    return;

  if (sensors_state[sensor].mode != OFF)
    sensors_disable(sensor);
  
  sensors_state[sensor].mode = DIGITAL;

  /* No register black magic needs to be done here, both lines
   * are already pulled up.
   */
}

void sensors_disable(U8 sensor) {
  if (sensor >= NXT_N_SENSORS)
    return;

  switch (sensors_state[sensor].mode) {
  case OFF:
  case LEGACY:
    break;
  case ANALOG:
  case DIGITAL:
    /* Disable output on the DIGI pins to return to the idle state. */
    *AT91C_PIOA_SODR = (sensors_state[sensor].pins.sda |
                        sensors_state[sensor].pins.scl);
    *AT91C_PIOA_ODR = (sensors_state[sensor].pins.sda |
                       sensors_state[sensor].pins.scl);
    break;
  }
}
