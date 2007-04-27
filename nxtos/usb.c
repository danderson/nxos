
#include "at91sam7s256.h"

#include "aic.h"

#include "systick.h"
#include "display.h"
#include "interrupts.h"

#include "usb.h"

#define MIN(x, y) (x < y ? x : y)



/* number of endpoints ; there are 4, but we will only
 * use 3 of them */
#define NMB_ENDPOINTS 4


/* max packet size in reception for each endpoint */
#define MAX_RCV_SIZE 64

/* max packet size when we send data */
#define MAX_SND_SIZE 8


/* used in setup packets : */

/* see 'bmRequestType' in the specs of a setup packet
 * H_TO_D == Host to Device
 * D_TO_H == Device to Host
 * STD    == Type : Standart
 * CLS    == Type : Class
 * VDR    == Type : Vendor
 * RSV    == Type : Reserved
 * DEV    == Recipient : Device
 * INT    == Recipient : Interface
 * EPT    == Recipient : Endpoint
 * OTH    == Recipient : Other
 */
#define USB_BMREQUEST_D_TO_H         0x80

#define USB_BMREQUEST_H_TO_D_STD_DEV 0x0
#define USB_BMREQUEST_D_TO_H_STD_DEV 0x80
/* ... */

#define USB_BREQUEST_SET_INTERFACE   0xB
#define USB_BREQUEST_GET_DESCRIPTOR  0x6

#define USB_WVALUE_TYPE        (0xFF << 8)
#define USB_WVALUE_TYPE_DEVICE 1 << 8
#define USB_WVALUE_TYPE_CONFIG 2 << 8
#define USB_WVALUE_INDEX       0xFF


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

  U16 bcdUsb;          /* USB Specification Number which device complies too */
  U8  bDeviceClass;    /* Class Code */
  U8  bDeviceSubClass; /* Sub class code */
  U8  bDeviceProtocol; /* device protocol */
  U8  bMaxPacketSize;  /* max packet size for the endpoint 0 */
  U16 idVendor;
  U16 idProduct;
  U16 bcdDevice;       /* Device release number */
  U8  iManufacturer;   /* Index of manufacturer string descriptor */
  U8  iProduct;        /* Index of Product String Descriptor */
  U8  iSerialNumber;   /* Index of Serial Number String Descriptor */
  U8  bNumConfigurations; /* Number of possible configurations */

} usb_dev_desc = {
  { 18, 1 }, /* header */
  0x0200, /* bcdUsb : USB 2.0 */
  0, /* class code : => specified by the interface  */
  0, /* sub class code */
  0, /* device protocol */
  MAX_RCV_SIZE, /* max packet size for the end point 0 :
		 * all the endpoint of the nxt can take 64 bytes */
  0x0694, /* idVendor : LEGO */
  0xFF00, /* idProduct : NXTOS */
  0, /* bcdDevice */
  0, /* index of manufacturer string */
  0, /* index of product string */
  0, /* index of serial number => none */
  1, /* number of possible configuration */
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
  U8  bmRequestType;  /* bit field : see the specs */
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


  /* ds == Data to send */
  /* ds_data : last position of the data pointer */
  U8 *ds_data[NMB_ENDPOINTS];
  /* ds_length : data remaining to send */
  U32 ds_length[NMB_ENDPOINTS];

} usb_state = {
  0
};





/* These two functions are recommended by the ATMEL doc (34.6.10) */
//! Clear flags of UDP UDP_CSR register and waits for synchronization
static inline void usb_csr_clear_flag(U8 endpoint, U32 flags)
{
  AT91C_UDP_CSR[endpoint] &= ~(flags);
  //while (AT91C_UDP_CSR[endpoint] & (flags));
}

//! Set flags of UDP UDP_CSR register and waits for synchronization
static inline void usb_csr_set_flag(U8 endpoint, U32 flags)
{
  AT91C_UDP_CSR[endpoint] |= (flags);
  //while ( (AT91C_UDP_CSR[endpoint] & (flags)) != (flags) );
}





static void usb_send_data(int endpoint, U8 *ptr, U32 length) {
  U32 packet_size;

  /* we can't send more than MAX_SND_SIZE each time */
  packet_size = MIN(MAX_SND_SIZE, length);

  length -= packet_size;

  /* we put the packet in the fifo */
  while(packet_size--) {
    AT91C_UDP_FDR[0] = *ptr++;
  }

  /* we prepare the next sending */
  usb_state.ds_data[endpoint]   = ptr;
  usb_state.ds_length[endpoint] = length;
  usb_state.y = 1;

  /* and next we tell the controller to send what is in the fifo */
  usb_csr_set_flag(endpoint, AT91C_UDP_TXPKTRDY);
}




/**
 * this function is called when
 * we receive a setup packet
 */
static void usb_manage_setup_packet() {
  usb_setup_packet_t packet;

  /* setup packet are always received
   * on the endpoint 0 */
  packet.bmRequestType = AT91C_UDP_FDR[0];
  packet.bRequest      = AT91C_UDP_FDR[0];
  packet.wValue        = (AT91C_UDP_FDR[0] & 0xFF) | (AT91C_UDP_FDR[0] << 8);
  packet.wIndex        = (AT91C_UDP_FDR[0] & 0xFF) | (AT91C_UDP_FDR[0] << 8);
  packet.wLength       = (AT91C_UDP_FDR[0] & 0xFF) | (AT91C_UDP_FDR[0] << 8);


  usb_csr_clear_flag(0, AT91C_UDP_RX_DATA_BK0);
  usb_csr_clear_flag(0, AT91C_UDP_RXSETUP);

  if (packet.bmRequestType == USB_BMREQUEST_D_TO_H) {
    usb_csr_set_flag(0, AT91C_UDP_DIR); /* we change the direction */
  }



  if (packet.bmRequestType == USB_BMREQUEST_D_TO_H_STD_DEV
      && packet.bRequest == USB_BREQUEST_GET_DESCRIPTOR)
    {

      if ((packet.wValue & USB_WVALUE_TYPE) == USB_WVALUE_TYPE_DEVICE) {
	usb_send_data(0, (U8 *)(&usb_dev_desc),
		      MIN(sizeof(usb_dev_desc), packet.wLength));

      } else if ((packet.wValue & USB_WVALUE_TYPE) == USB_WVALUE_TYPE_CONFIG) {

	usb_state.x = packet.bmRequestType;
	usb_state.y = 1;

      }

    }
  else
    {
      //usb_state.x = packet.bmRequestType;
      //usb_state.y = packet.bRequest;
    }

}





static void usb_isr() {
  U8 endpoint = 127;

  usb_state.nmb_int++;
  usb_state.last_isr      =  systick_get_ms();
  usb_state.last_udp_isr  = *AT91C_UDP_ISR;
  usb_state.last_udp_csr0 =  AT91C_UDP_CSR[0];
  usb_state.last_udp_csr1 =  AT91C_UDP_CSR[1];


  if (*AT91C_UDP_ISR & AT91C_UDP_ENDBUSRES) {

    /* we ack all these interruptions */
    *AT91C_UDP_ICR = AT91C_UDP_ENDBUSRES;
    *AT91C_UDP_ICR = AT91C_UDP_RXSUSP; /* suspend */
    *AT91C_UDP_ICR = AT91C_UDP_RXRSM; /* resume */

    /* we reset the end points */
    *AT91C_UDP_RSTEP = ~0;
    *AT91C_UDP_RSTEP = 0;

    /* we activate the "function" (?!) */
    *AT91C_UDP_FADDR = AT91C_UDP_FEN;

    /* then we activate the irq for the end point 0 (and only for this one) */
    *AT91C_UDP_IDR = ~0;
    *AT91C_UDP_IER |= 0x1;

    /* we must activate the end point 0  & configure it */
    AT91C_UDP_CSR[0] = AT91C_UDP_EPEDS | AT91C_UDP_EPTYPE_CTRL;


    return;
  }


  if (*AT91C_UDP_ISR & AT91C_UDP_RXSUSP) {
    *AT91C_UDP_ICR = AT91C_UDP_RXSUSP;
  }

  if (*AT91C_UDP_ISR & AT91C_UDP_RXRSM) {
    *AT91C_UDP_ICR = AT91C_UDP_RXRSM;
  }


  for (endpoint = 0; endpoint < NMB_ENDPOINTS ; endpoint++) {
    if (*AT91C_UDP_ISR & (1 << endpoint))
      break;
  }


  if (endpoint == 0) {
    *AT91C_UDP_ICR = AT91C_UDP_EPINT0;

    if (AT91C_UDP_CSR[0] & AT91C_UDP_RXSETUP) {
      usb_manage_setup_packet();
      return;
    }
  }


  if (endpoint < NMB_ENDPOINTS) { /* if an endpoint was specified */
    if (AT91C_UDP_CSR[endpoint] & AT91C_UDP_TXCOMP) {
      /* then it means that we sent a data and the host has acknowledged it */

      /* so first we will reset this flag */
      usb_csr_clear_flag(endpoint, AT91C_UDP_TXCOMP);

      /* and we will send the following data */
      if (usb_state.ds_length > 0)
	usb_send_data(endpoint, usb_state.ds_data[endpoint],
		      usb_state.ds_length[endpoint]);

      return;
    }

  }


  /* if we are here, it means we don't know what to do
   * with the interruption, so we simply ack it */
  *AT91C_UDP_ICR = *AT91C_UDP_ISR; /* << this is evil :P */
}



void usb_send(const U8 *data, U32 length) {


}




void usb_disable() {

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
   * this interruption is always emit (can't be disable with UDP_IER)
   */
  /* other interruptions will be enabled when needed */
  aic_install_isr(AT91C_ID_UDP, AIC_PRIO_DRIVER,
		  AIC_TRIG_EDGE, usb_isr);


  interrupts_enable();


  /* Enable the UDP pull up by outputting a zero on PA.16 */
  /* Enabling the pull up will tell to the host (the computer) that
   * we are ready for a communication
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
