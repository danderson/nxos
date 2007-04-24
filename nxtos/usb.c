
#include "at91sam7s256.h"

#include "aic.h"

#include "systick.h"
#include "display.h"


#include "usb.h"



/* to have more informations about the descriptors,
 * see http://www.beyondlogic.org/usbnutshell/usb5.htm
 */


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
typedef struct usb_dev_desc {
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
  U16 idProduct;       /* LEGO : 0x0002 ; NXTOS : 0x00FF */
  U16 bcdDevice;       /* Device release number : NXTOS : 0x0000 */
  U8  iManufacturer;   /* Index of the manufacturer string descriptor : 0x01 */
  U8  iProduct;        /* Index of Product String Descriptor : 0x02 */
  U8  iSerialNumber;   /* Index of Serial Number String Descriptor : 0x03 */
  U8  bNumConfigurations; /* Number of possible configurations : 0x01 */

} usb_dev_desc_t;


/*
 * describe a configuration
 * only one is used with nxtos
 * bLength = 9
 * bDescriptorType = 0x02
 */
typedef struct usb_config_desc {
  usb_desc_header_t header;

  U16 wTotalLength;   /* Total length in bytes of data returned ; TO COMPUTE */
  U8  bNumInterfaces; /* Number of Interfaces ; nxtos use only 1 */
  U8  bConfigurationValue; /* Value to use as an argument to select this
			    * configuration will be used by the computer to select this config */
  U8  iConfiguration; /* Index of String Descriptor describing this
		       * configuration */
  U8  bmAttributes;   /* */
  U8  bMaxPower;      /* max power consumption (unit: 2mA) : 0 for the nxt */

  /* From http://www.beyondlogic.org/usbnutshell/usb5.htm :
   * When the configuration descriptor is read, it returns the entire
   * configuration hierarchy which includes all related interface and endpoint
   * descriptors. The wTotalLength field reflects the number of bytes in the
   * hierarchy.
   */
} usb_config_desc_t;


/*
 * interface descriptor
 * bLength = 9 bytes
 * bDescriptorType = 0x04
 */
typedef struct usb_int_desc {
  usb_desc_header_t header;

  
} usb_int_desc_t;




static struct {
  /*
   * 0 == not initialized
   * 1 == initialized but no communication
   * 2 == initialized and something was received
   */
  U32 status;

  /*
   * number of bytes read since the beginning
   */
  U32 nmb_bytes_read;

  /*
   * last endpoint from where came a comm
   */
  S8 endpoint;

  /*
   * set to 1 while in the interruption
   * set to 2 if error
   */
  U32 reading;
} usb;






/* these two functions are recommanded by the ATMEL doc (34.6.10) */
//! Clear flags of UDP UDP_CSR register and waits for synchronization
static __inline void udp_ep_clr_flag(U8 endpoint, U32 flags)
{
  while (AT91C_UDP_CSR[endpoint] & (flags))
    AT91C_UDP_CSR[endpoint] &= ~(flags);
}

//! Set flags of UDP UDP_CSR register and waits for synchronization
static __inline void udp_ep_set_flag(U8 endpoint, U32 flags)
{
  while ( (AT91C_UDP_CSR[endpoint] & (flags)) != (flags) )
    AT91C_UDP_CSR[endpoint] |= (flags);
}





static void usb_isr() {
  /* number of bytes to read from the fifo */
  U16 nmb_bytes, i;

  /* message read */
  U8 msg[MAX_ENDPOINT_SIZE];

  usb.status = 2;
  usb.reading = 1;

  if ((*AT91C_UDP_ISR & 0xF) == 0) { /* Hu ? */
    usb.endpoint = 125;
    usb.reading = 0;
    return;
  }

 *AT91C_UDP_ISR;


  if (*AT91C_UDP_ISR & 1) /* endpoint 0 */
    usb.endpoint = 0;
  else if (*AT91C_UDP_ISR & (1 << 1)) /* endpoint 1 */
    usb.endpoint = 1;
  else if (*AT91C_UDP_ISR & (1 << 2)) /* endpoint 2 */
    usb.endpoint = 2;
  else if (*AT91C_UDP_ISR & (1 << 3)) /* endpoint 3 */
    usb.endpoint = 3;


  /** number of bytes to read */
  nmb_bytes = (AT91C_UDP_CSR[usb.endpoint] & AT91C_UDP_RXBYTECNT) >> 16;

  for (i = 0 ; i < nmb_bytes ; i++) {
    msg[i] = AT91C_UDP_FDR[usb.endpoint] & 0xFF;
    usb.nmb_bytes_read++;
  }

  /* notify that the data have been read */
  udp_ep_clr_flag(usb.endpoint, AT91C_UDP_RX_DATA_BK0 & AT91C_UDP_RXSETUP);

  usb.reading = 0;
}


void usb_disable() {
  usb.status = 0;

  *AT91C_PIOA_PER = (1 << 16);
  *AT91C_PIOA_SODR = (1 << 16);
  *AT91C_PIOA_OER = (1 << 16);
}


void usb_init() {
  usb_disable();

  usb.endpoint = -1;
  usb.nmb_bytes_read = 0;
  usb.status = 1;

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
}


void usb_test() {
  int i;


  display_clear();


  for (i = 0 ; i < 100 ; i++) {
    systick_wait_ms(250);

    display_cursor_set_pos(0, 0);

    if (usb.status < 2)
      display_string("Jflesch 0-USB 1");
    else {
      display_string("Jflesch 1-USB 0");
    }

    if (usb.endpoint >= 0) {
      display_cursor_set_pos(0, 1);
      display_uint(usb.endpoint);
    }

    display_cursor_set_pos(0, 2);
    display_uint(usb.nmb_bytes_read);

    display_cursor_set_pos(0, 3);
    display_uint(usb.reading);

    systick_wait_ms(250);
  }

}
