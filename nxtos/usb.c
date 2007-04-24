
#include "at91sam7s256.h"

#include "aic.h"

#include "systick.h"
#include "display.h"
#include "interrupts.h"

#include "usb.h"



/*
 * header common to all the usb descriptors (device, configuration, interface,
 * endpoint)
 */
typedef struct usb_desc_header {
  U8 bLength; /* length of the descriptor ; this whole header is counted in the
	       * size ! */
  U8 bDescriptionType; /* type of the descriptor */

  /* next follow the parameters */
} usb_desc_header_t;


/*
 * device descriptor
 * only one is used by nxtos
 * bLength = 18;
 * bDescriptionType = 0x1;
 */
static const struct {
  usb_desc_header_t header;

  U16 bcdUsb;          /* USB Specification Number which device complies too ;
			* 0x0200 (usb 2) */
  U8  bDeviceClass;    /* Class Code ; lejOS : 0 here => specified by the
			* interface */
  U8  bDeviceSubClass; /* Sub class code ; lejOS : 0 */
  U8  bDeviceProtocol; /* (lejOS : 0x00) */
  U8  bMaxPacketSize;  /* max packet size for the endpoint 0 ;
			* for the NXT, max = 64 */
  U16 idVendor;        /* LEGO : 0x0694 */
  U16 idProduct;       /* LEGO : 0x0002 ; NXTOS : 0xFF00 */
  U16 bcdDevice;       /* Device release number : NXTOS : 0x0000 */
  U8  iManufacturer;   /* Index of the manufacturer string descriptor : 0x01 */
  U8  iProduct;        /* Index of Product String Descriptor : 0x02 */
  U8  iSerialNumber;   /* Index of Serial Number String Descriptor : 0x03 */
  U8  bNumConfigurations; /* Number of possible configurations : 0x01 */

} usb_dev_desc = {
  { 0 },
  0
};


/*
 * describe a configuration
 * only one is used with nxtos
 * bLength = 9
 * bDescriptorType = 0x02
 */
static const struct {
  usb_desc_header_t header;

  U16 wTotalLength;   /* Total length in bytes of data returned */
  U8  bNumInterfaces; /* Number of Interfaces ; nxtos use only 1 */
  U8  bConfigurationValue; /* Value to use as an argument to select this
			    * configuration will be used by the computer to select this config */
  U8  iConfiguration; /* Index of String Descriptor describing this
		       * configuration */
  U8  bmAttributes;   /* */
  U8  bMaxPower;      /* max power consumption (unit: 2mA) : 0 for the nxt */

  /* the config descriptor is followed by all the other descriptors */
} usb_config_desc = {
  {0},
  0
};


/*
 * interface descriptor
 * bLength = 9 bytes
 * bDescriptorType = 0x04
 */
static const struct {
  usb_desc_header_t header;


} usb_int_desc = {
  {0}
};




static volatile struct {
  /* for debug purpose : */
  U8  isr;
  U32 last_udp_isr;
  U32 last_udp_csr0;
  U32 last_udp_csr1;

} usb_state;






/* THESE two functions are recommended by the ATMEL doc (34.6.10) */
//! Clear flags of UDP UDP_CSR register and waits for synchronization
static inline void udp_ep_clr_flag(U8 endpoint, U32 flags)
{
  while (AT91C_UDP_CSR[endpoint] & (flags))
    AT91C_UDP_CSR[endpoint] &= ~(flags);
}

//! Set flags of UDP UDP_CSR register and waits for synchronization
static inline void udp_ep_set_flag(U8 endpoint, U32 flags)
{
  while ( (AT91C_UDP_CSR[endpoint] & (flags)) != (flags) )
    AT91C_UDP_CSR[endpoint] |= (flags);
}





static void usb_isr() {
  U8 endpoint = 127;

  usb_state.isr = 1;

  usb_state.last_udp_isr = *AT91C_UDP_ISR;
  usb_state.last_udp_csr0 = AT91C_UDP_CSR[0];
  usb_state.last_udp_csr1 = AT91C_UDP_CSR[1];

  if (*AT91C_UDP_ISR & 1) /* endpoint 0 */
    endpoint = 0;
  else if (*AT91C_UDP_ISR & (1 << 1)) /* endpoint 1 */
    endpoint = 1;
  else if (*AT91C_UDP_ISR & (1 << 2)) /* endpoint 2 */
    endpoint = 2;
  else if (*AT91C_UDP_ISR & (1 << 3)) /* endpoint 3 */
    endpoint = 3;



  /* notify that the data have been read */
  if (endpoint != 127)
    udp_ep_clr_flag(endpoint, AT91C_UDP_RX_DATA_BK0 & AT91C_UDP_RXSETUP);

  usb_state.isr = 0;
}


void usb_disable() {
  usb_state.last_udp_isr  = 0;
  usb_state.last_udp_csr0 = 0;
  usb_state.last_udp_csr1 = 0;
}


void usb_init() {

  usb_disable();

  interrupts_disable();

  usb_state.isr = 0;
  usb_state.last_udp_isr = 0;
  usb_state.last_udp_csr0 = 0;
  usb_state.last_udp_csr1 = 0;

  /* usb pll was already set in init.S */

  /* enable peripheral clock */
  *AT91C_PMC_PCER = (1 << AT91C_ID_UDP);

  /* enable system clock */
  *AT91C_PMC_SCER = AT91C_PMC_UDP;

  /* disable all the interruptions */
  *AT91C_UDP_IDR = ~0;

  /* reset all the endpoints */
  *AT91C_UDP_RSTEP = 0xF;
  *AT91C_UDP_RSTEP = 0;


  /* Install the interruption routine */
  aic_clear(AT91C_ID_UDP);
  aic_install_isr(AT91C_ID_UDP, AIC_INT_LEVEL_NORMAL, usb_isr);


  /* enable interruption for the endpoint 0, 1, 2, 3 */
  *AT91C_UDP_IER = ~0; //0x0F;

  interrupts_enable();


  /* Enable the UDP pull up by outputting a zero on PA.16 */
  /* Enabling the pull up will tell to the host (the computer) that
   * we are ready for a communication (are we ?)
   * How do I know it's PA.16 who must be used ? I don't, I looked at Lejos
   * source code (and they probably looked at Lego source code ... :)
   */
  *AT91C_PIOA_PER = (1 << 16);
  *AT91C_PIOA_OER = (1 << 16);
  *AT91C_PIOA_CODR = (1 << 16);
}


void usb_test() {
  int i;


  display_clear();


  for (i = 0 ; i < 40 ; i++) {
    systick_wait_ms(250);

    display_cursor_set_pos(0, 0);
    display_string("In isr ? : ");
    display_uint(usb_state.isr);

    display_cursor_set_pos(0, 1);
    display_string("ISR:  0x");
    display_hex(usb_state.last_udp_isr);

    display_cursor_set_pos(0, 2);
    display_string("CSR0: 0x");
    display_hex(usb_state.last_udp_csr0);

    display_cursor_set_pos(0, 3);
    display_string("CSR1: 0x");
    display_uint(usb_state.last_udp_csr1);

    systick_wait_ms(250);
  }

}
