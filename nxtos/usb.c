
#include "at91sam7s256.h"

#include "aic.h"

#include "systick.h"
#include "display.h"


#include "usb.h"


/*
 * 0 == not initialized
 * 1 == initialized but no communication
 * 2 == initialized and communication running
 */
volatile U32 g_status;


static void usb_isr() {
  /* endpoint */
  U8 endpoint;

  /* number of bytes to read from the fifo */
  U16 nmb_bytes, i;

  /* message read */
  U8 msg[MAX_ENDPOINT_SIZE];

  g_status = 2;


  if ((*AT91C_UDP_ISR & 0x1) == 0x1) /* endpoint 0 */
    endpoint = 0;

  if ((*AT91C_UDP_ISR & 0x2) == 0x2) /* endpoint 1 */
    endpoint = 1;

  if ((*AT91C_UDP_ISR & 0x2) == 0x3) /* endpoint 2 */
    endpoint = 2;

  if ((*AT91C_UDP_ISR & 0x2) == 0x4) /* endpoint 3 */
    endpoint = 3;


  if (endpoint == 127) /* unknown endpoint ?! */
    return;


  nmb_bytes = AT91C_UDP_CSR[endpoint] & AT91C_UDP_RXBYTECNT;


  for (i = 0 ; i < nmb_bytes ; i++)
    msg[i] = AT91C_UDP_FDR[endpoint] & 0xFF;


  AT91C_UDP_CSR[endpoint] = 0; /* clear RXSETUP to say we read the FIFO */
}


void usb_disable() {
  g_status = 0;

  *AT91C_PIOA_PER = (1 << 16);
  *AT91C_PIOA_SODR = (1 << 16);
  *AT91C_PIOA_OER = (1 << 16);
}


void usb_init() {
  usb_disable();

  /* usb pll was already set in init.S */

  /* enable peripheral clock */
  *AT91C_PMC_PCER = (1 << AT91C_ID_UDP);

  /* enable system clock */
  *AT91C_PMC_SCER = AT91C_PMC_UDP;

  /* disable all the interruptions */
  *AT91C_UDP_IDR = 0xFFFFFFFF;

  /* mask all the interruptions */
  *AT91C_UDP_IMR = 0x00000000;

  /* reset all the endpoints */
  *AT91C_UDP_RSTEP = 0x0F;
  *AT91C_UDP_RSTEP = 0x00;


  /* Install the interrupt routine */
  aic_install_isr(AT91C_ID_UDP, AIC_INT_LEVEL_NORMAL, usb_isr);


  /* enable interruption for the endpoint 0, 1, 2, 3 */
  *AT91C_UDP_IER = 0x0F;

  /* unmask the corresponding interruptions */
  *AT91C_UDP_IMR = 0x0F;


  /* Enable the UDP pull up by outputting a zero on PA.16 */
  /* Enabling the pull up will tell to the host (the computer) that
   * we are ready for a communication (are we ?)
   * How do I know it's PA.16 who must be used ? I don't, I looked at Lejos
   * source code (and they probably looked at Lego source code ... :)
   */
  *AT91C_PIOA_PER = (1 << 16);
  *AT91C_PIOA_CODR = (1 << 16);
  *AT91C_PIOA_OER = (1 << 16);

  g_status = 1;
}


void usb_test() {
  int i;


  display_clear();


  for (i = 0 ; i < 40 ; i++) {
    systick_wait_ms(250);

    display_cursor_set_pos(0, 2);

    if (g_status < 2)
      display_string("Jflesch 0 - USB 1");
    else
      display_string("Jflesch 1 - USB 0");

    systick_wait_ms(250);
  }

}
