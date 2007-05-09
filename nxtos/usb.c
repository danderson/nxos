
#include "at91sam7s256.h"

#include "aic.h"

#include "systick.h"
#include "display.h"
#include "interrupts.h"
#include "util.h"

#include "usb.h"

#define MIN(x, y) (x < y ? x : y)



/* number of endpoints ; there are 4, but we will only
 * use 3 of them */
#define NMB_ENDPOINTS 4
#define NMB_USED_ENDPOINTS 3

/* maximum packet size for the endpoint 0 */
#define MAX_ENDPT0_SIZE 8

/* max packet size in reception for each endpoint */
#define MAX_RCV_SIZE 64

/* max packet size when we send data */
#define MAX_SND_SIZE 64


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
#define USB_BMREQUEST_DIR            0x80
#define USB_BMREQUEST_H_TO_D         0x0
#define USB_BMREQUEST_D_TO_H         0x80

#define USB_BMREQUEST_RCPT           0x0F
#define USB_BMREQUEST_RCPT_DEV       0x0 /* device */
#define USB_BMREQUEST_RCPT_INT       0x1 /* interface */
#define USB_BMREQUEST_RCPT_EPT       0x2 /* endpoint */
#define USB_BMREQUEST_RCPT_OTH       0x3 /* other */


#define USB_BREQUEST_GET_STATUS      0x0
#define USB_BREQUEST_CLEAR_FEATURE   0x1
#define USB_BREQUEST_SET_FEATURE     0x3
#define USB_BREQUEST_SET_ADDRESS     0x5
#define USB_BREQUEST_GET_DESCRIPTOR  0x6
#define USB_BREQUEST_SET_DESCRIPTOR  0x7
#define USB_BREQUEST_GET_CONFIG      0x8
#define USB_BREQUEST_SET_CONFIG      0x9
#define USB_BREQUEST_GET_INTERFACE   0xA
#define USB_BREQUEST_SET_INTERFACE   0xB


#define USB_WVALUE_TYPE        (0xFF << 8)
#define USB_DESC_TYPE_DEVICE           1
#define USB_DESC_TYPE_CONFIG           2
#define USB_DESC_TYPE_STR              3
#define USB_DESC_TYPE_INT              4
#define USB_DESC_TYPE_ENDPT            5
#define USB_DESC_TYPE_DEVICE_QUALIFIER 6

#define USB_WVALUE_INDEX       0xFF



/*** OUTPUT DATA (from the NXT PoV) ****/





/*
 * device descriptor
 */
static const U8 usb_dev_desc[] = {
  18, USB_DESC_TYPE_DEVICE, /* header :
			     * packet size: 18 ;
			     * type : device description */
  0x00, 0x20, /* bcd_usb :
	       * USB Specification Number which device complies to
	       * Here : USB 2.0 */
  2, /* Class code */
  0, /* Sub class code */
  0, /* Device protocol */
  MAX_ENDPT0_SIZE, /* max packet size for the end point 0 */
  0x94, 0x06, /* id_vendor : LEGO */
  0x00, 0xFF, /* id_product : NXTOS */
  0, 0, /* bcd_device (~ revision of the product) */
  1, /* i_manufacturer : index of manufacturer string */
  2, /* i_product : index of product string */
  0, /* i_serial_number : index of serial number (here : none) */
  1, /* b_num_configurations : number of possible config */
};



static const U8 usb_dev_qualifier_desc[] = {
  10, /* b_length */
  USB_DESC_TYPE_DEVICE_QUALIFIER, /* b_descriptor_type */
  0x00, 0x20, /* bcd_usb :
	       * USB Specification Number which device complies to
	       * Here : USB 2.0 */
  2, /* Class code */
  0, /* Sub class code */
  0, /* Device protocol */
  MAX_ENDPT0_SIZE, /* max packet size for the end point 0 */
  1, /* b_num_configurations : number of possible config */
  0 /* reserved : must be 0 */
};



static const U8 usb_nxtos_full_config[] = {
  /* configuration packet :
   * describe a configuration
   * only one is used with nxtos
   */
  0x09, /* b_length : size of the configuration packet */
  0x02, /* b_descriptor_type : config packet */
  0x20, 0x00, /* w_total_length
	       * (here, 9 + 9 + 2*7 = 32)
	       * sizeof(config packet) +
	       * nmb interfaces sizeof(interface packet) +
	       * nmb endpoints * sizeof(endpoint packet)
	       * Note: if the total length exceed 0xFF, you must adapt
	       *       the function usb_manage_setup_packet
	       */
  0x01, /* b_num_interfaces : number of interfaces */
  0x01, /* b_configuration_value :
	 * Value to use as an argument to select this
	 * configuration will be used by the computer to select this config
	 */
  0x00, /* Index of String Descriptor describing this
	 * configuration */


  /* bm_attributes:
   * Bitmap :
   *  D7 Reserved, set to 1. (USB 1.0 Bus Powered)
   *  D6 Self Powered
   *  D5 Remote Wakeup
   *  D4..0 Reserved, set to 0.
   */
#define BM_ATTR_RESERVED_7    0x80
#define BM_ATTR_SELF_POWERED  0x40
#define BM_ATTR_REMOTE_WAKEUP 0x20
#define BM_ATTR_RESERVED_4_0  0x00
  BM_ATTR_RESERVED_7 | BM_ATTR_SELF_POWERED | BM_ATTR_RESERVED_4_0,
  0, /* b_max_power : max power consumption (unit: 2mA) : 0 for the nxt */


  /*
   * interface descriptor
   */
  0x09, /* b_length */
  0x04, /* b_descriptor_type */
  0x00, /* b_interface_number */
  0x00, /* b_alternate_settings */
  0x02, /* b_num_endpoints : number of endpoints for this interface
	 * (end point 0 not counted) */
  0xFF, /* b_interface_class (see the device descriptor) */
  0xFF, /* b_interface_sub_class (see the device descriptor) */
  0xFF, /* b_interface_protocol (see the device descriptor) */
  0x0,  /* i_interface : Index of String Descriptor describing this interface */


  /*
   * endpoint 1 descriptor
   */
  7, /* b_length */
  USB_DESC_TYPE_ENDPT, /* desc type */

  /* b_endpoint_address:
   * bitmap :
   * Bits 0..3b Endpoint Number.
   * Bits 4..6b Reserved. Set to Zero
   * Bits 7 Direction 0 = Out, 1 = In (Ignored for Control Endpoints)
   */
#define B_ENDPOINT_ADDR_DIR_IN       0x80
#define B_ENDPOINT_ADDR_RESERVED_6_4 0x00
  B_ENDPOINT_ADDR_RESERVED_6_4 | 0x1,


  /* bm_attributes:
   * bitmap again:
   * Bits 0..1 Transfer Type
   *  00 = Control ; 01 = Isochronous ; 10 = Bulk ; 11 = Interrupt
   *
   * Bits 2..7 are reserved, but if Isochronous endpoint:
   * - Bits 3..2 = Synchronisation Type (Iso Mode):
   *   00 = No Synchonisation;01 = Asynchronous;10 = Adaptive;11 = Synchronous
   * - Bits 5..4 = Usage Type (Iso Mode):
   *   00 = Data Endpoint;01 = Feedback Endpoint;
   */
#define BM_ATTR_ENDPT_CONTROL     0x00
#define BM_ATTR_ENDPT_ISOCHRONOUS 0x01
#define BM_ATTR_ENDPT_BULK        0x02
#define BM_ATTR_ENDPT_INTERRUPT   0x03
  BM_ATTR_ENDPT_BULK,
  MAX_RCV_SIZE, 0x00,      /* w_max_packet_size (64) : Max packet size */
  0,                  /* b_interval */


  /*
   * endpoint 2 descriptor
   */
  7, /* b_length */
  USB_DESC_TYPE_ENDPT, /* desc type */
  /* b_endpoint_address: */
  B_ENDPOINT_ADDR_DIR_IN | B_ENDPOINT_ADDR_RESERVED_6_4 | 0x2,
  /* bm_attributes: */
  BM_ATTR_ENDPT_BULK,
  MAX_RCV_SIZE, 0x00,      /* w_max_packet_size (64) : Max packet size */
  0
};



/*
 * string descriptor
 * explain to the host that we only speak english
 */
static const U8 usb_string_desc[] = {
  4, /* b_length */
  0x03, /* b_descriptor_type */
  0x09, 0x04 /* English (US) */
};

/*
 * b_length = 2 (header) + (2*strlen(str))
 * type = 0x03
 */
static const U8 usb_lego_str[] =
  { 2+(2*4)+2, USB_DESC_TYPE_STR,
    'L', 0,
    'E', 0,
    'G', 0,
    'O', 0,
    '\0', '\0'};

static const U8 usb_nxt_str[] =
  { 2+(2*3)+2, USB_DESC_TYPE_STR,
    'N', 0,
    'X', 0,
    'T', 0,
    '\0', '\0'};


static const U8 *usb_strings[] = {
  usb_lego_str,
  usb_nxt_str
};



/** INPUT DATA (from the NXT PoV) ****/


typedef struct usb_setup_packet {
  U8  bm_request_type;  /* bit field : see the specs */
  U8  b_request;      /* request */
  U16 w_value;        /* value */
  U16 w_index;        /* index */
  U16 w_length;       /* number of bytes to transfer if there is a data phase */
} usb_setup_packet_t;





/** INTERNAL **/

static volatile struct {
  /* for debug purpose : */
  S8 usb_status;
  U8 has_sent_stall;
  U32 stalling_isr;

  U32 debug; /* various informations depending of the error code */


  U8 current_config; /* 0 (none) or 1 (the only config) */
  U8 current_rx_bank;
  U8 is_suspended;   /* true or false */


  /* ds == Data to send */
  /* ds_data : last position of the data pointer */
  U8 *ds_data[NMB_ENDPOINTS];
  /* ds_length : data remaining to send */
  U32 ds_length[NMB_ENDPOINTS];


  /* dr == Data received */
  /* The first buffer is where the interruption
   *  function will write
   * The second buffer is where the user app read
   * The first buffer is put into the second
   * one when the application flush the second buffer
   */
  U8  dr_buffer[2][USB_BUFFER_SIZE+1];
  U16 dr_buffer_used[2]; /* data size waiting in the buffer */
  U8  dr_overflowed;

} usb_state = {
  0
};





/***** FUNCTIONS ******/



/* These two functions are recommended by the ATMEL doc (34.6.10) */
//! Clear flags of UDP UDP_CSR register and waits for synchronization
static inline void usb_csr_clear_flag(U8 endpoint, U32 flags)
{
  AT91C_UDP_CSR[endpoint] &= ~(flags);
  while (AT91C_UDP_CSR[endpoint] & (flags));
}

//! Set flags of UDP UDP_CSR register and waits for synchronization
static inline void usb_csr_set_flag(U8 endpoint, U32 flags)
{
  AT91C_UDP_CSR[endpoint] |= (flags);
  while ( (AT91C_UDP_CSR[endpoint] & (flags)) != (flags) );
}





static void usb_send_data(int endpoint, const U8 *ptr, U32 length) {
  U32 packet_size;

  /* we can't send more than MAX_SND_SIZE each time */
  if (endpoint == 0) {
    packet_size = MIN(MAX_ENDPT0_SIZE, length);
  } else {
    packet_size = MIN(MAX_SND_SIZE, length);
  }

  length -= packet_size;

  /* we put the packet in the fifo */
  while(packet_size) {
    AT91C_UDP_FDR[endpoint] = *ptr;
    ptr++;
    packet_size--;
  }

  if (endpoint > 0)
    usb_state.usb_status = USB_STATUS_WRITED_SOMETHING;

  /* we prepare the next sending */
  usb_state.ds_data[endpoint]   = length ? (U8 *)ptr : NULL;
  usb_state.ds_length[endpoint] = length;

  /* and next we tell the controller to send what is in the fifo */
  usb_csr_set_flag(endpoint, AT91C_UDP_TXPKTRDY);

}



static void usb_read_data(int endpoint) {
  U8 buf;
  U16 i;
  U16 total;



  if (endpoint == 1) {

    total = ((AT91C_UDP_CSR[endpoint] & AT91C_UDP_RXBYTECNT) >> 16) & 0x7FF;

    if (total > 0) {
	usb_state.usb_status = USB_STATUS_READ_SOMETHING;
    }

    /* by default we use the buffer for the interruption function */
    /* except if the buffer for the user application is already free */

    if (usb_state.dr_buffer_used[1] == 0) /* if the user buffer is free */
      buf = 1;
    else {
      if (usb_state.dr_buffer_used[0] > 0) /* if the isr buffer is already used */
	usb_state.dr_overflowed = 1;
      buf = 0;
    }

    usb_state.dr_buffer_used[buf] = total;

    /* we read the data, and put them in the buffer */
    for (i = 0 ; i < total; i++)
      usb_state.dr_buffer[buf][i] = AT91C_UDP_FDR[1];

    usb_state.dr_buffer[buf][i+1] = '\0';

    /* and then we tell the controller that we read the FIFO */
    usb_csr_clear_flag(1, usb_state.current_rx_bank);

    /* we switch on the other bank */
    if (usb_state.current_rx_bank == AT91C_UDP_RX_DATA_BK0)
      usb_state.current_rx_bank = AT91C_UDP_RX_DATA_BK1;
    else
      usb_state.current_rx_bank = AT91C_UDP_RX_DATA_BK0;

  } else {

    /* we ignore */
    usb_csr_clear_flag(endpoint, AT91C_UDP_RX_DATA_BK0 | AT91C_UDP_RX_DATA_BK1);

  }
}



/*
 * when the nxt doesn't understand something from the host
 * it must send a "stall"
 */
static void usb_send_stall(S8 reason) {
  usb_state.usb_status = reason;

  if (usb_state.has_sent_stall == 0)
    usb_state.has_sent_stall = 1;

  usb_csr_set_flag(0, AT91C_UDP_FORCESTALL);
}


/*
 * when we receive a setup packet
 * we must sometimes answer with a null packet
 */
static void usb_send_null() {
  usb_send_data(0, NULL, 0);
}




/**
 * this function is called when
 * we receive a setup packet
 */
static void usb_manage_setup_packet() {
  usb_setup_packet_t packet;
  U16 value16;
  U32 size;
  U8 index;


  /* setup packet are always received
   * on the endpoint 0 */
  packet.bm_request_type = AT91C_UDP_FDR[0];
  packet.b_request       = AT91C_UDP_FDR[0];
  packet.w_value         = (AT91C_UDP_FDR[0] & 0xFF) | (AT91C_UDP_FDR[0] << 8);
  packet.w_index         = (AT91C_UDP_FDR[0] & 0xFF) | (AT91C_UDP_FDR[0] << 8);
  packet.w_length        = (AT91C_UDP_FDR[0] & 0xFF) | (AT91C_UDP_FDR[0] << 8);


  if ((packet.bm_request_type & USB_BMREQUEST_DIR) == USB_BMREQUEST_D_TO_H) {
    usb_csr_set_flag(0, AT91C_UDP_DIR); /* we change the direction */
  }

  usb_csr_clear_flag(0, AT91C_UDP_RXSETUP);


  value16 = 0;


  /* let's see what the host want from us */

  switch (packet.b_request)
    {
    case (USB_BREQUEST_GET_STATUS):

      switch (packet.bm_request_type & USB_BMREQUEST_RCPT)
	{
	case (USB_BMREQUEST_RCPT_DEV):
	  value16 = 1; /* self powered but can't wake up the host */
	  break;
	case (USB_BMREQUEST_RCPT_INT):
	  value16 = 0;
	  break;
	case (USB_BMREQUEST_RCPT_EPT):
	  value16 = 0; /* endpoint not halted */
	  /* TODO : Check what the host has sent ! */
	default:
	  break;
	}

      usb_send_data(0, (U8 *)&value16, 2);

      break;

    case (USB_BREQUEST_CLEAR_FEATURE):
    case (USB_BREQUEST_SET_INTERFACE):
    case (USB_BREQUEST_SET_FEATURE):
      /* ni ! */
      /* we send null to not be bothered by the host */
      usb_send_null();
      break;

    case (USB_BREQUEST_SET_ADDRESS):
      /* we ack */
      usb_send_null();

      /* we must be sure that the ack was sent & received */
      while(!(AT91C_UDP_CSR[0] & AT91C_UDP_TXCOMP));
      usb_csr_clear_flag(0, AT91C_UDP_TXCOMP);

      /* we set the specified usb address in the controller */
      *AT91C_UDP_FADDR    = AT91C_UDP_FEN | packet.w_value;
      /* and we tell the controller that we are in addressed mode now */
      *AT91C_UDP_GLBSTATE = packet.w_value > 0 ? AT91C_UDP_FADDEN : 0;
      break;

    case (USB_BREQUEST_GET_DESCRIPTOR):
      /* the host want some informations about us */
      index = (packet.w_value & USB_WVALUE_INDEX);

      switch ((packet.w_value & USB_WVALUE_TYPE) >> 8)
	{
	case (USB_DESC_TYPE_DEVICE):
	  /* it wants infos about the device */
	  size = usb_dev_desc[0];
	  usb_send_data(0, usb_dev_desc,
			MIN(size, packet.w_length));
	  break;

	case (USB_DESC_TYPE_CONFIG):
	  /* it wants infos about a specific config */
	  /* we have only one configuration so ... */
	  usb_send_data(0, usb_nxtos_full_config,
			MIN(usb_nxtos_full_config[2], packet.w_length));
	  if (usb_nxtos_full_config[2] < packet.w_length)
	    usb_send_null();
	  break;

	case (USB_DESC_TYPE_STR):
	  if ((packet.w_value & USB_WVALUE_INDEX) == 0) {
	    /* the host want to know want language we support */
	    usb_send_data(0, usb_string_desc,
			  MIN(usb_string_desc[0], packet.w_length));
	  } else {
	    /* the host want a specific string */
	    /* TODO : Check it asks an existing string ! */
	    usb_send_data(0, usb_strings[index-1],
			  MIN(usb_strings[index-1][0],
			      packet.w_length));
	  }
	  break;

	case (USB_DESC_TYPE_DEVICE_QUALIFIER):
	  size = usb_dev_qualifier_desc[0];
	  usb_send_data(0, usb_dev_qualifier_desc,
	  		MIN(size, packet.w_length));
	  break;

	default:
	  usb_state.debug=packet.w_value;
	  usb_send_stall(USB_STATUS_UNKNOWN_DESCRIPTOR);
	  break;
      }

      break;

    case (USB_BREQUEST_GET_CONFIG):
      usb_send_data(0, (U8 *)&(usb_state.current_config), 1);
      break;

    case (USB_BREQUEST_SET_CONFIG):
      usb_state.usb_status = USB_STATUS_INIT_DONE;
      usb_state.current_config = packet.w_value;

      /* we ack */
      usb_send_null();

      /* we set the register in configured mode */
      *AT91C_UDP_GLBSTATE = packet.w_value > 0 ?
	(AT91C_UDP_CONFG | AT91C_UDP_FADDEN)
	:AT91C_UDP_FADDEN;

      break;

    case (USB_BREQUEST_GET_INTERFACE):
    case (USB_BREQUEST_SET_DESCRIPTOR):
    default:
      usb_send_stall(USB_STATUS_UNMANAGED_REQUEST);
      break;
    }

}





static void usb_isr() {
  U8 endpoint = 127;
  U32 csr, isr;

  isr = *AT91C_UDP_ISR;


  if (AT91C_UDP_CSR[0] & AT91C_UDP_ISOERROR /* == STALLSENT */) {
    /* then it means that we sent a stall, and the host has ack the stall */

    if (usb_state.has_sent_stall < 2)
      usb_state.has_sent_stall = 2;

    usb_csr_clear_flag(0, AT91C_UDP_FORCESTALL | AT91C_UDP_ISOERROR);
  }


  if (isr & AT91C_UDP_ENDBUSRES) {
    usb_state.usb_status = USB_STATUS_INIT_STARTED;

    /* we ack all these interruptions */
    *AT91C_UDP_ICR = AT91C_UDP_ENDBUSRES;
    *AT91C_UDP_ICR = AT91C_UDP_RXSUSP; /* suspend */
    *AT91C_UDP_ICR = AT91C_UDP_RXRSM; /* resume */
    *AT91C_UDP_ICR = AT91C_UDP_SOFINT;
    *AT91C_UDP_ICR = AT91C_UDP_WAKEUP;

    /* we reset the end points */
    *AT91C_UDP_RSTEP = ~0;
    *AT91C_UDP_RSTEP = 0;


    /* we activate the function (i.e. us),
     * and set the usb address 0 */
    *AT91C_UDP_FADDR = AT91C_UDP_FEN | 0x0;

    usb_state.current_config  = 0;
    usb_state.current_rx_bank = 0;
    usb_state.is_suspended    = 0;

    /* then we activate the irq for the end points 0, 1 and 2 */
    /* and for the suspend / resume */
    *AT91C_UDP_IDR = ~0;
    *AT91C_UDP_IER |= 0x7 /* endpts */ | (0x3 << 8) /* suspend / resume */;

    usb_state.current_rx_bank = AT91C_UDP_RX_DATA_BK0;

    /* we redefine how the endpoints must work */
    AT91C_UDP_CSR[0] = AT91C_UDP_EPEDS | AT91C_UDP_EPTYPE_CTRL;
    AT91C_UDP_CSR[1] = AT91C_UDP_EPEDS | AT91C_UDP_EPTYPE_BULK_OUT;
    AT91C_UDP_CSR[2] = AT91C_UDP_EPEDS | AT91C_UDP_EPTYPE_BULK_IN;
    AT91C_UDP_CSR[3] = 0;

    //*AT91C_UDP_ICR = 0xFFFFFFFF;

    return;
  }

  if (isr & AT91C_UDP_WAKEUP) {
    *AT91C_UDP_ICR = AT91C_UDP_WAKEUP;
    isr &= ~AT91C_UDP_WAKEUP;
  }


  if (isr & AT91C_UDP_SOFINT) {
    *AT91C_UDP_ICR = AT91C_UDP_SOFINT;
    isr &= ~AT91C_UDP_SOFINT;
  }


  if (isr & AT91C_UDP_RXSUSP) {
    *AT91C_UDP_ICR = AT91C_UDP_RXSUSP;
    isr &= ~AT91C_UDP_RXSUSP;
    usb_state.is_suspended = 1;
  }

  if (isr & AT91C_UDP_RXRSM) {
    *AT91C_UDP_ICR = AT91C_UDP_RXRSM;
    isr &= ~AT91C_UDP_RXRSM;
    usb_state.is_suspended = 0;
  }


  for (endpoint = 0; endpoint < NMB_ENDPOINTS ; endpoint++) {
    if (isr & (1 << endpoint))
      break;
  }


  if (endpoint == 0) {

    if (AT91C_UDP_CSR[0] & AT91C_UDP_RXSETUP) {
      usb_manage_setup_packet();
      return;
    }
  }


  if (endpoint < NMB_ENDPOINTS) { /* if an endpoint was specified */
    csr = AT91C_UDP_CSR[endpoint];

    if (csr & AT91C_UDP_RX_DATA_BK0
	|| csr & AT91C_UDP_RX_DATA_BK1) {
      usb_read_data(endpoint);

      return;
    }

    if (csr & AT91C_UDP_TXCOMP) {

      /* then it means that we sent a data and the host has acknowledged it */

      /* so first we will reset this flag */
      usb_csr_clear_flag(endpoint, AT91C_UDP_TXCOMP);

      /* and we will send the following data */
      if (usb_state.ds_length[endpoint] > 0
	  && usb_state.ds_data[endpoint] != NULL) {

	usb_send_data(endpoint, usb_state.ds_data[endpoint],
		      usb_state.ds_length[endpoint]);
      }

      return;
    }

  }


  /* We clear also the unused bits,
   * just "to be sure" */
  if (isr) {
    *AT91C_UDP_ICR = 0xFFFFC4F0;
  }
}






void usb_disable() {
  *AT91C_PIOA_PER = (1 << 16);
  *AT91C_PIOA_OER = (1 << 16);
  *AT91C_PIOA_SODR = (1 << 16);
}


static inline void usb_enable() {
  /* Enable the UDP pull up by outputting a zero on PA.16 */
  /* Enabling the pull up will tell to the host (the computer) that
   * we are ready for a communication
   */
  *AT91C_PIOA_PER = (1 << 16);
  *AT91C_PIOA_OER = (1 << 16);
  *AT91C_PIOA_CODR = (1 << 16);
}


void usb_init() {

  usb_disable();

  systick_wait_ms(200);

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

  usb_enable();

  *AT91C_UDP_ICR = 0xFFFFFFFF;

  /* Install the interruption routine */

  /* the first interruption we will get is an ENDBUSRES
   * this interruption is always emit (can't be disable with UDP_IER)
   */
  /* other interruptions will be enabled when needed */
  aic_install_isr(AT91C_ID_UDP, AIC_PRIO_DRIVER,
		  AIC_TRIG_LEVEL, usb_isr);


  interrupts_enable();
}


U8 usb_can_send() {
  return (!usb_state.is_suspended
	  && usb_state.ds_length[1] > 0);
}


void usb_send(U8 *data, U32 length) {
  /* wait until the end point is free */
  while(usb_state.is_suspended
	|| usb_state.ds_length[2] > 0);

  /* start sending the data */
  usb_send_data(2, data, length);
}



U16 usb_has_data() {
  return usb_state.dr_buffer_used[1];
}


U8 *usb_get_buffer() {
  return (usb_state.dr_buffer[1]);
}


U8 usb_overflowed() {
  return usb_state.dr_overflowed;
}

void usb_flush_buffer() {
  usb_state.dr_overflowed = 0;

  if (usb_state.dr_buffer_used[0] > 0)
    memcpy(usb_state.dr_buffer[1], usb_state.dr_buffer[0],
	   usb_state.dr_buffer_used[0]);

  usb_state.dr_buffer_used[1] = usb_state.dr_buffer_used[0];
  usb_state.dr_buffer_used[0] = 0;
}


U8 usb_status() {
  return usb_state.usb_status;
}



void usb_display_debug() {
  display_clear();
  display_cursor_set_pos(0, 0);
  display_string("--- USB infos --\n"
		 "----------------");
  display_string("\nStat.:0x");
  display_hex(usb_state.usb_status);
  display_string("\nOverflowed: ");
  display_uint(usb_state.dr_overflowed);
  display_string("\nStalled: ");
  display_uint(usb_state.has_sent_stall);
  display_string("\nDebug: 0x");
  display_hex(usb_state.debug);
}
