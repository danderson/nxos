#include "at91sam7s256.h"

#include "types.h"
#include "crt0.h"
#include "aic.h"
#include "sys_timer.h"

static enum {
  TWI_UNINITIALIZED = 0,
  TWI_READY,
  TWI_TX_BUSY,
  TWI_RX_BUSY,
  TWI_FAILED
} twi_state;

static struct {
  U32 pending_len;
  U8 *pending_ptr;
  enum {
    IN = 0,
    OUT,
  } pending_direction;
  U8 pending_thread; /* Wake up this thread on pending done. Not
                        implemented yet. */
} twi_current_request;

/* Interrupt service routine for the TWI, used to drive transmission
 * and reception of data.
 */
static void twi_isr() {
}

/* Reset the I2C bus and restart the TWI controller.
 *
 * The NXT's TWI controller has a hardware bug: if the system attempts
 * to initialize it when the I2C bus is *not* idle (either the data or
 * clock line are low), then the TWI controller will crash and become
 * unresponsive until it is reset.
 *
 * To counter this, we shut down the TWI controller, manually take
 * control of the two I2C bus lines, and drive them for up to a full
 * I2C transaction (9 clock line cycles), until we successfully pull
 * the two bus lines high. Then we can give control of the bus back to
 * the TWI controller and initialize it.
 */
void twi_init() {
  unsigned long cycles = 9;

  interrupts_disable();

  /* Disable all TWI interrupts. */
  *AT91C_TWI_IDR = ~0;

  /* Enable power to the I/O and TWI controllers. */
  *AT91C_PMC_PCER = ((1 << AT91C_ID_PIOA) |
                     (1 << AT91C_ID_TWI));

  /* Configure the I/O controller to take control of the clock and
   * data lines of the I2C bus, to work around the bug.
   */
  *AT91C_PIOA_MDER = (1 << 3) | (1 << 4); /* Enable multi-drive. */
  *AT91C_PIOA_PER = (1 << 3) | (1 << 4);  /* Give pin control to the PIO. */
  *AT91C_PIOA_ODR = (1 << 3);             /* The data line is input only. */
  *AT91C_PIOA_OER = (1 << 4);             /* Drive the clock as an output. */

  /* Force-clock the I2C bus until the data line goes high. */
  while (cycles > 0 && !(*AT91C_PIOA_PDSR & (1 << 3))) {
    *AT91C_PIOA_CODR = (1 << 4);
    sys_timer_wait_ns(1500);
    *AT91C_PIOA_SODR = (1 << 4);
    sys_timer_wait_ns(1500);
    cycles--;
  }

  /* At this stage, the I2C bus is in an idle state. Relinquish I/O
   * pin control and assign the pins to the TWI controller.
   */
  *AT91C_PIOA_PDR = (1 << 3) | (1 << 4);
  *AT91C_PIOA_ASR = (1 << 3) | (1 << 4);

  /* Reset the TWI controller into disabled mode. */
  *AT91C_TWI_CR = (1 << 7) | (1 << 3);

  /* Configure the waveform generator to generate a perfect square
   * clock signal, clocked at 380KHz.
   */
  *AT91C_TWI_CWGR = (0x2 << 16) | (0xf << 8) | (0xf << 0);

  /* Enable the TWI controller in master mode. */
  *AT91C_TWI_CR = (1 << 2);

  twi_state = TWI_READY;

  /* Install the TWI controller interrupt service routine. */
  aic_install_isr(AT91C_ID_TWI, twi_isr);
  aic_enable(AT91C_ID_TWI);

  interrupts_enable();
}

int twi_is_ready() {
  return (twi_state & TWI_READY);
}

int twi_is_busy() {
  return ((twi_state & TWI_TX_BUSY) || (twi_state & TWI_RX_BUSY));
}


/* Start a read operation on the I2C bus. This function returns
 * control to the OS immediately.
 *
 * When the read is completed, the given flag address is set to 1.
 */
int twi_read_async(U32 dev_addr, U32 int_addr_bytes, U32 int_addr, U8 *data,
                   U32 len)
{
  /* Wait until the TWI is idle. */
  while (!twi_is_ready());

  
}
