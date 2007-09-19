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

/* The base clock frequency of the sensor I2C bus in Hz. */
#define SENSOR_I2C_SPEED 9600

static volatile struct i2c_port {
  enum {
    I2C_OFF = 0, /* Port not initialized in I2C mode. */
    I2C_IDLE,    /* No transaction in progress. */
  } bus_state;

  /* The connected device address on the I2C bus. */
  U8 device_addr;
  
} i2c_state[NXT_N_SENSORS] = {
  { I2C_OFF, 0 },
  { I2C_OFF, 0 },
  { I2C_OFF, 0 },
  { I2C_OFF, 0 },
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
  
  i2c_state[sensor].device_addr = address;
}

/** Interrupt handler. */
void i2c_isr() {
  volatile struct i2c_port *p;
  U32 lines = *AT91C_PIOA_PDSR;
  short i;
  
  for (i=0; i<NXT_N_SENSORS; i++) {
    volatile sensor_pins pins = sensors_get_pins(i);
    p = &i2c_state[i];

    switch (p->bus_state) {
      case I2C_OFF:
        /* Port is OFF, do nothing. */
        break;
      
      case I2C_IDLE:
        if (lines & pins.sda) {
          /* Something's happening. */
        }
        
        break;
    }
  }
}
