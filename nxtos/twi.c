#include "at91sam7s256.h"
#include "types.h"
#include "lock.h"
#include "crt0.h"
#include "aic.h"
#include "sys_timer.h"

static spinlock twi_busy = SPINLOCK_INIT_LOCKED;

static volatile enum {
  TWI_UNINITIALIZED = 0,
  TWI_READY,
  TWI_TX_BUSY,
  TWI_RX_BUSY,
  TWI_FAILED,
} current_state;

static volatile struct {
  U32 len;
  U8 *ptr;
  U8 *flag; /* Set *flag to 1 when the pending request is complete. */
} current_request;

/* Interrupt service routine for the TWI, used to drive transmission
 * and reception of data.
 */
static void twi_isr() {
  U32 status = *AT91C_TWI_SR;

  /* We received the data we were expecting. */
  if (current_state == TWI_RX_BUSY && (status & AT91C_TWI_RXRDY)) {
    if (current_request.len > 0) {
      *(current_request.ptr) = *AT91C_TWI_RHR;
      current_request.ptr++;
      current_request.len--;
    }

    /* Only one byte left to receive, we tell the TWI to send STOP
     * after this. */
    if (current_request.len == 1)
      *AT91C_TWI_CR = AT91C_TWI_STOP;

    /* Receive complete, return to idle. */
    if (current_request.len == 0) {
      *AT91C_TWI_IDR = ~0;
      current_state = TWI_READY;
      *(current_request.flag) = TRUE;
      spinlock_release(twi_busy);
    }
  }

  /* We finished sending a byte. */
  if (current_state == TWI_TX_BUSY && (status & AT91C_TWI_TXRDY)) {
    if (current_request.len > 0) {
      /* There is still some data buffered that needs to be sent. */
      *AT91C_TWI_CR = AT91C_TWI_START;
      if (current_request.len == 1)
        *AT91C_TWI_CR = AT91C_TWI_STOP;
      *AT91C_TWI_THR = *(current_request.ptr);
      current_request.ptr++;
      current_request.len--;
    } else {
      /* All data sent. */
      *AT91C_TWI_IDR = ~0;
      current_state = TWI_READY;
      *(current_request.flag) = TRUE;
      spinlock_release(twi_busy);
    }
  }

  /* TODO: Handle NAK, which might be either a crash or a reinit... */
}

/* Reset the I2C bus and start the TWI controller.
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
  U8 cycles = 9;

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

  /* Install the TWI controller interrupt service routine. */
  aic_install_isr(AT91C_ID_TWI, twi_isr);
  aic_enable(AT91C_ID_TWI);

  /* Let the world make requests on the TWI bus. */
  current_state = TWI_READY;
  spinlock_release(twi_busy);

  interrupts_enable();
}

bool twi_is_ready() {
  return !twi_busy;
}

bool twi_is_busy() {
  return twi_busy;
}

/* Start a read operation on the I2C bus. This function returns
 * control to the OS immediately.
 *
 * When the read is completed, the given flag is set to TRUE.
 */
void twi_read_async(U32 dev_id, U8 *data, U32 len, bool *done_flag)
{
  /* The value for the mode register. This sets the 7-bit device
     address and read mode. */

  U32 mode =
    ((dev_id << 16) & AT91C_TWI_DADR) | AT91C_TWI_IADRSZ_NO | AT91C_TWI_MREAD;


  /* Wait until the TWI is idle. */
  spinlock_acquire(twi_busy);
  current_state = TWI_RX_BUSY;

  current_request.len = len;
  current_request.ptr = data;
  current_request.flag = done_flag;
  *done_flag = FALSE;

  /* Disable all interrupt signalling in the TWI */
  *AT91C_TWI_IDR = ~0;

  /* Set the mode to the value defined above and start the read. */
  *AT91C_TWI_MMR = mode;
  *AT91C_TWI_CR = AT91C_TWI_START;

  /* Tell the TWI to send an interrupt when a byte is received, or
   * when there is a NAK (error) condition. */
  *AT91C_TWI_IER = AT91C_TWI_RXRDY | AT91C_TWI_NACK;
}

/* Start a write operation on the I2C bus. This function returns
 * control to the OS immediately.
 *
 * When the write is completed, the given flag is set to TRUE.
 */
void twi_write_async(U32 dev_id, U8 *data, U32 len, bool *done_flag)
{
  /* The value for the mode register. This sets the 7-bit device
     address and read mode. */
  U32 mode =
    ((dev_id << 16) & AT91C_TWI_DADR) | AT91C_TWI_IADRSZ_NO;

  /* Wait until the TWI is idle. */
  spinlock_acquire(twi_busy);
  current_state = TWI_TX_BUSY;

  current_request.len = len;
  current_request.ptr = data;
  current_request.flag = done_flag;
  *done_flag = FALSE;

  /* Disable all interrupt signalling in the TWI. */
  *AT91C_TWI_IDR = ~0;

  /* Set the mode to the value defined above and start the write. */
  *AT91C_TWI_MMR = mode;
  *AT91C_TWI_CR = AT91C_TWI_START;

  /* Tell the TWI to send an interrupt when a byte is sent, or
   * when there is a NAK (error) condition. */
  *AT91C_TWI_IER = AT91C_TWI_TXRDY | AT91C_TWI_NACK;
}
