
#include "at91sam7s256.h"

#include "aic.h"

#include "systick.h"
#include "display.h"
#include "interrupts.h"

#include "usb.h"


/* used in setup packets : */

#define USB_BMREQUEST_DIR_HOST_TO_DEVICE 0x0
#define USB_BMREQUEST_DIR_DEVICE_TO_HOST 0x1

#define USB_BMREQUEST_TYPE_STANDARD 0x0
#define USB_BMREQUEST_TYPE_CLASS    0x1
#define USB_BMREQUEST_TYPE_VENDOR   0x2
#define USB_BMREQUEST_TYPE_RESERVED 0x3

#define USB_BMREQUEST_REC_DEVICE    0x0
#define USB_BMREQUEST_REC_INTERFACE 0x1
#define USB_BMREQUEST_REC_ENDPOINT  0x2
#define USB_BMREQUEST_REC_OTHER     0x4

#define USB_BREQUEST_SET_INTERFACE   0xB
#define USB_BREQUEST_GET_DESCRIPTOR  0x6



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




typedef struct usb_setup_packet {
  /* bmRequestType is a bit map, so with this union
   * I can easily parse it
   */
  union {
    struct {
      U8  recipient:5;
      U8  type     :2;
      U8  direction:1;
    } details;

    U8 whole;
  } bmRequestType;

  U8  bRequest;      /* request */
  U16 wValue;        /* value */
  U16 wIndex;        /* index */
  U16 wLength;       /* number of bytes to transfer if there is a data phase */
} usb_setup_packet_t;





static volatile struct {
  /* for debug purpose : */
  U8  isr;
  U32 nmb_int;
  U32 last_isr;
  U32 last_udp_isr;
  U32 last_udp_csr0;
  U32 last_udp_csr1;
  U32 x;
  U32 y;

} usb_state;





/* THESE two functions are recommended by the ATMEL doc (34.6.10) */
//! Clear flags of UDP UDP_CSR register and waits for synchronization
static inline void udp_csr_clear_flag(U8 endpoint, U32 flags)
{
  AT91C_UDP_CSR[endpoint] &= ~(flags);
  while (AT91C_UDP_CSR[endpoint] & (flags));
}

//! Set flags of UDP UDP_CSR register and waits for synchronization
static inline void udp_csr_set_flag(U8 endpoint, U32 flags)
{
  AT91C_UDP_CSR[endpoint] |= (flags);
  while ( (AT91C_UDP_CSR[endpoint] & (flags)) != (flags) );
}





static inline void usb_send_null() {
  /* we tell to the controller that we put something in the FIFO
   */
  AT91C_UDP_CSR[0] |= AT91C_UDP_TXPKTRDY;

  while ( !((AT91C_UDP_CSR[0]) & AT91C_UDP_TXCOMP));

  udp_csr_clear_flag(0, AT91C_UDP_TXCOMP);
}


static inline void usb_send_stall() {
  udp_csr_set_flag(0, AT91C_UDP_FORCESTALL);
  udp_csr_clear_flag(0, AT91C_UDP_FORCESTALL | AT91C_UDP_ISOERROR);
}



/**
 * this function is called when
 * we receive a setup packet
 */
void usb_manage_setup_packet() {
  /* strangly, the value RXBYTECNT from the register UDP_CSR0
   * is set to 0, but there are data in the fifo
   */

  usb_setup_packet_t packet;

  /* setup packet are always received
   * on the endpoint 0 */
  packet.bmRequestType.whole = AT91C_UDP_FDR[0];
  packet.bRequest      = AT91C_UDP_FDR[0];
  packet.wValue        = (AT91C_UDP_FDR[0] & 0xFF) | (AT91C_UDP_FDR[0] << 8);
  packet.wIndex        = (AT91C_UDP_FDR[0] & 0xFF) | (AT91C_UDP_FDR[0] << 8);
  packet.wLength       = (AT91C_UDP_FDR[0] & 0xFF) | (AT91C_UDP_FDR[0] << 8);


  udp_csr_clear_flag(0, AT91C_UDP_RXSETUP);

  if (packet.bmRequestType.details.direction    == USB_BMREQUEST_DIR_DEVICE_TO_HOST
      && packet.bmRequestType.details.type      == USB_BMREQUEST_TYPE_STANDARD
      && packet.bmRequestType.details.recipient == USB_BMREQUEST_REC_DEVICE
      && packet.bRequest == USB_BREQUEST_GET_DESCRIPTOR)
    {
      /* TODO */
    }
}




static void usb_isr() {
  U8 endpoint = 127;


  usb_state.nmb_int++;
  usb_state.last_isr = systick_get_ms();
  usb_state.last_udp_isr = *AT91C_UDP_ISR;
  usb_state.last_udp_csr0 = AT91C_UDP_CSR[0];
  usb_state.last_udp_csr1 = AT91C_UDP_CSR[1];



  if (*AT91C_UDP_ISR & AT91C_UDP_EPINT0) {
    endpoint = 0;
  } else if (*AT91C_UDP_ISR & AT91C_UDP_EPINT1) {
    endpoint = 1;
  } else if (*AT91C_UDP_ISR & AT91C_UDP_EPINT2) {
    endpoint = 2;
  } else if (*AT91C_UDP_ISR & AT91C_UDP_EPINT3) {
    endpoint = 3;
  }


  if (*AT91C_UDP_ISR & AT91C_UDP_ENDBUSRES) {
    /* we reset the end points */
    *AT91C_UDP_RSTEP = ~0;
    *AT91C_UDP_RSTEP = 0;

    /* we activate the 'function' ?! */
    *AT91C_UDP_FADDR = AT91C_UDP_FEN;

    /* then we activate the irq for the end point 0 */
    *AT91C_UDP_IER |= 0x1;

    /* we must activate the end point 0  & parameter it */
    AT91C_UDP_CSR[0] = AT91C_UDP_EPEDS | AT91C_UDP_EPTYPE_CTRL;

    /* we ack the endbusres interruption */
    *AT91C_UDP_ICR = AT91C_UDP_ENDBUSRES;

    return;
  }



  if (endpoint == 0) {
    *AT91C_UDP_ICR = AT91C_UDP_EPINT0;

    if (AT91C_UDP_CSR[0] & AT91C_UDP_RXSETUP) {
      usb_manage_setup_packet();
    }

    return;
  }

  /* if we are here, it means we don't know what to do
   * with the interruption, so we simply ack it */
  *AT91C_UDP_ICR = *AT91C_UDP_ISR; /* << this is evil :P */

}


void usb_disable() {
  usb_state.nmb_int = 0;
  usb_state.last_udp_isr  = 0;
  usb_state.last_udp_csr0 = 0;
  usb_state.last_udp_csr1 = 0;
  usb_state.x = 0;
  usb_state.y = 0;
}


void usb_init() {

  usb_disable();

  interrupts_disable();



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

  /* the first interruption we will get is an ENDBUSRES
   * this interruption is always emit (can't be disable with UDP_IER
   */
  /* other interruptions will be enabled when needed */
  aic_clear(AT91C_ID_UDP);
  aic_install_isr(AT91C_ID_UDP, AIC_PRIO_DRIVER,
                  AIC_TRIG_LEVEL, usb_isr);


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
    display_string("nmb isr : ");
    display_uint(usb_state.nmb_int);

    display_cursor_set_pos(0, 1);
    display_string("ISR: 0x");
    display_hex(usb_state.last_udp_isr);

    display_cursor_set_pos(0, 2);
    display_string("CSR0:0x");
    display_hex(usb_state.last_udp_csr0);

    display_cursor_set_pos(0, 3);
    display_string("CSR1:0x");
    display_uint(usb_state.last_udp_csr1);

    display_cursor_set_pos(0, 4);
    display_string("Last:0x");
    display_hex(usb_state.last_isr);
    display_string("/0x");
    display_hex(systick_get_ms());

    display_cursor_set_pos(0, 5);
    display_string("X   :0x");
    display_hex(usb_state.x);

    display_cursor_set_pos(0, 6);
    display_string("Y   :0x");
    display_hex(usb_state.y);

    systick_wait_ms(250);
  }

}
