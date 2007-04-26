/* Driver for the NXT's sensors.
 *
 * This driver provides the basic building blocks for the specialized
 * sensor drivers: Initial conversion of the A/D values read from the
 * sensors, a SoftMAC I2C driver for the sensor digital I/O, and ways
 * to register logic for each sensor port.
 */

#include "at91sam7s256.h"

#include "mytypes.h"
#include "interrupts.h"
#include "systick.h"
#include "aic.h"
#include "sensors.h"

/* Mapping of the clock and data lines for each of the 4 sensor ports'
 * i2c busses. */
static const struct {
  U32 scl;
  U32 sda;
} i2c_bus[4] = {
  { AT91C_PIO_PA23, AT91C_PIO_PA18 },
  { AT91C_PIO_PA28, AT91C_PIO_PA19 },
  { AT91C_PIO_PA29, AT91C_PIO_PA20 },
  { AT91C_PIO_PA30, AT91C_PIO_PA2 },
};


static volatile struct {
  /* The mode of the sensor port. */
  enum {
    OFF = 0, /* Unused. */
    ANALOG,  /* Active in analog mode. */
    DIGITAL, /* Active in digital (i2c) mode. */
  } mode;

  /* A port in digital mode requires the storage of additional state
   * for the i2c driver.
   */
  struct {
    /* TODO: Make digital ports work. */
  } digital;
} sensors_state[NXT_N_SENSORS] = {
  { OFF, {} },
  { OFF, {} },
  { OFF, {} },
  { OFF, {} },
};


void sensors_init() {

}
