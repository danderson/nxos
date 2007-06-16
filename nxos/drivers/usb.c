/* Driver for the NXT's USB port.
 *
 * This driver drives the onboard USB controller to make the NXT into
 * a functional USB 2.0 peripheral. Note that given the limitations of
 * the controller hardware, the brick cannot function as a host, only
 * as a peripheral.
 */

#include "at91sam7s256.h"

#include "mytypes.h"
#include "interrupts.h"
#include "systick.h"
#include "aic.h"
#include "util.h"
#include "usb.h"

/* TODO: Move to util.h */
#define MIN(x, y) (x < y ? x : y)


/* The USB controller supports up to 4 endpoints. */
#define N_ENDPOINTS 4

/* Maximum data packet sizes. Endpoint 0 is a special case (control
 * endpoint).
 *
 * TODO: Discuss the need/use for separating recv/send.
 */
#define MAX_EP0_SIZE 8
#define MAX_RCV_SIZE 64
#define MAX_SND_SIZE 64


/* Various constants for the setup packets.
 *
 * TODO: clean up these. Most are unused.
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


/* The following definitions are 'raw' USB setup packets. They are all
 * standard responses to various setup requests by the USB host. These
 * packets are all constant, and mostly boilerplate. Don't be too
 * bothered if you skip over these to real code.
 *
 * If you want to understand the full meaning of every bit of these
 * packets, you should refer to the USB 2.0 specifications.
 *
 * One point of interest: the USB device space is partitionned by
 * vendor and product ID. As we are lacking money and real need, we
 * don't have a vendor ID to use. Therefore, we are currently
 * piggybacking on Lego's device space, using an unused product ID.
 */
static const U8 usb_device_descriptor[] = {
  18, USB_DESC_TYPE_DEVICE, /* Packet size and type. */
  0x00, 0x20, /* This packet is USB 2.0. */
  2, /* Class code. */
  0, /* Sub class code. */
  0, /* Device protocol. */
  MAX_EP0_SIZE, /* Maximum packet size for EP0 (control endpoint). */
  0x94, 0x06, /* Vendor ID : LEGO */
  0x00, 0xFF, /* Product ID : NXOS */
  0x00, 0x00, /* Product revision. */
  1, /* Index of the vendor string. */
  2, /* Index of the product string. */
  0, /* Index of the serial number (none for us). */
  1, /* The number of possible configurations. */
};

static const U8 usb_dev_qualifier_desc[] = {
  10, USB_DESC_TYPE_DEVICE_QUALIFIER, /* Packet size and type. */
  0x00, 0x20, /* This packet is USB 2.0. */
  2, /* Class code */
  0, /* Sub class code */
  0, /* Device protocol */
  MAX_EP0_SIZE, /* Maximum packet size for EP0. */
  1, /* The number of possible configurations. */
  0 /* Reserved for future use, must be zero. */
};


static const U8 usb_nxos_full_config[] = {
  0x09, USB_DESC_TYPE_CONFIG, /* Descriptor size and type. */
  0x20, 0x00, /* Total length of the configuration, interface
               * description included.
               */
  1, /* The number of interfaces declared by this configuration. */
  1, /* The ID for this configuration. */
  0, /* Index of the configuration description string (none). */

  /* Configuration attributes bitmap. Bit 7 (MSB) must be 1, bit 6 is
   * 1 because the NXT is self-powered, bit 5 is 0 because the NXT
   * doesn't support remote wakeup, and bits 0-4 are 0 (reserved).
   */
  0x40,
  0, /* Device power consumption, for non self-powered devices. */


  /*
   * This is the descriptor for the interface associated with the
   * configuration.
   */
  0x09, USB_DESC_TYPE_INT, /* Descriptor size and type. */
  0x00, /* Interface index. */
  0x00, /* ID for this interface configuration. */
  0x02, /* The number of endpoints defined by this interface
         * (excluding EP0).
         */
  0xFF, /* Interface class ("Vendor specific"). */
  0xFF, /* Interface subclass (see above). */
  0xFF, /* Interface protocol (see above). */
  0x00, /* Index of the string descriptor for this interface (none). */


  /*
   * Descriptor for EP1.
   */
  7, USB_DESC_TYPE_ENDPT, /* Descriptor length and type. */
  0x1, /* Endpoint number. MSB is zero, meaning this is an OUT EP. */
  0x2, /* Endpoint type (bulk). */
  MAX_RCV_SIZE, 0x00, /* Maximum packet size (64). */
  0, /* EP maximum NAK rate (device never NAKs). */


  /*
   * Descriptor for EP2.
   */
  7, USB_DESC_TYPE_ENDPT, /* Descriptor length and type. */
  0x82, /* Endpoint number. MSB is one, meaning this is an IN EP. */
  0x2, /* Endpoint type (bulk). */
  MAX_RCV_SIZE, 0x00, /* Maximum packet size (64). */
  0, /* EP maximum NAK rate (device never NAKs). */
};


static const U8 usb_string_desc[] = {
  4, USB_DESC_TYPE_STR, /* Descriptor length and type. */
  0x09, 0x04, /* Supported language ID (US English). */
};

static const U8 usb_lego_str[] = {
  12, USB_DESC_TYPE_STR,
  'L', 0,
  'E', 0,
  'G', 0,
  'O', 0,
  '\0', '\0', /* TODO: USB spec p274 sez this is useless. */
};

static const U8 usb_nxt_str[] = {
  12, USB_DESC_TYPE_STR,
  'N', 0,
  'x', 0,
  'O', 0,
  'S', 0,
  '\0', '\0', /* TODO: USB spec p274 sez this is useless. */
};


/* Internal lookup table mapping string descriptors to their indices
 * in the USB string descriptor table.
 */
static const U8 *usb_strings[] = {
  usb_lego_str,
  usb_nxt_str,
};


/*
 * The USB device state. Contains the current USB state (selected
 * configuration, etc.) and transitory state for data transfers.
 */
static volatile struct {
  /* The current state of the device. */
  enum usb_status {
    USB_UNINITIALIZED = 0,
    USB_READY,
    USB_BUSY,
    USB_SUSPENDED,
  } status;
  /* TODO: above state replaces is_suspended and is_waiting_ack. */

  /* Holds the status the bus was in before entering suspend. */
  enum usb_status pre_suspend_status;

  /* When the host gives us an address, we must send a null ACK packet
   * back before actually changing addresses. This field stores the
   * address that should be set once the ACK is sent.
   */
  U32 new_device_address;

  /* The currently selected USB configuration. */
  U8 current_config;

  /* Holds the state of the data transmissions on both EP0 and
   * EP1. This only gets used if the transmission needed to be split
   * into several USB packets.
   */
  U8 *tx_data[N_ENDPOINTS]; /* TODO: Switch to 2, memory waste. */
  U32 tx_len[N_ENDPOINTS];

  /* Holds received data shifted from the controller. Receiving is
   * double-buffered, and the reader must flush the current buffer to
   * gain access to the other buffer.
   */
  U8 rx_buffer[2][USB_BUFFER_SIZE+1];
  U16 rx_buffer_size[2]; /* data size waiting in the buffer */
  bool rx_overflow;

  /* The USB controller has two hardware input buffers. This remembers
   * the one currently in use.
   */
  U8 current_rx_bank;
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
  while ( (AT91C_UDP_CSR[endpoint] & (flags)) != (flags));
}





static void usb_send_data(int endpoint, const U8 *ptr, U32 length) {
  U32 packet_size;

  /* we can't send more than MAX_SND_SIZE each time */
  if (endpoint == 0) {
    packet_size = MIN(MAX_EP0_SIZE, length);
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

  /* we prepare the next sending */
  usb_state.tx_data[endpoint]   = length ? (U8 *)ptr : NULL;
  usb_state.tx_len[endpoint] = length;

  /* and next we tell the controller to send what is in the fifo */
  usb_state.status = USB_BUSY;
  usb_csr_set_flag(endpoint, AT91C_UDP_TXPKTRDY);

}



static void usb_read_data(int endpoint) {
  U8 buf;
  U16 i;
  U16 total;



  if (endpoint == 1) {

    total = ((AT91C_UDP_CSR[endpoint] & AT91C_UDP_RXBYTECNT) >> 16) & 0x7FF;

    /* by default we use the buffer for the interruption function */
    /* except if the buffer for the user application is already free */

    if (usb_state.rx_buffer_size[1] == 0) /* if the user buffer is free */
      buf = 1;
    else {
      if (usb_state.rx_buffer_size[0] > 0) /* if the isr buffer is already used */
	usb_state.rx_overflow = TRUE;
      buf = 0;
    }

    usb_state.rx_buffer_size[buf] = total;

    /* we read the data, and put them in the buffer */
    for (i = 0 ; i < total; i++)
      usb_state.rx_buffer[buf][i] = AT91C_UDP_FDR[1];

    usb_state.rx_buffer[buf][i+1] = '\0';

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
static void usb_send_stall(S8 reason) { /* TODO: remove reason. */
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
static U32 usb_manage_setup_packet() {
  /* The structure of a USB setup packet. */
  struct {
    U8  request_attrs; /* Request characteristics. */
    U8  request; /* Request type. */
    U16 value; /* Request-specific value. */
    U16 index; /* Request-specific index. */
    U16 length; /* The number of bytes transferred in the (optional)
                   * second phase of the control transfer. */
  } packet;
  U16 value16;
  U32 size;
  U8 index;


  /* setup packet are always received
   * on the endpoint 0 */
  packet.request_attrs = AT91C_UDP_FDR[0];
  packet.request       = AT91C_UDP_FDR[0];
  packet.value         = (AT91C_UDP_FDR[0] & 0xFF) | (AT91C_UDP_FDR[0] << 8);
  packet.index         = (AT91C_UDP_FDR[0] & 0xFF) | (AT91C_UDP_FDR[0] << 8);
  packet.length        = (AT91C_UDP_FDR[0] & 0xFF) | (AT91C_UDP_FDR[0] << 8);


  if ((packet.request_attrs & USB_BMREQUEST_DIR) == USB_BMREQUEST_D_TO_H) {
    usb_csr_set_flag(0, AT91C_UDP_DIR); /* we change the direction */
  }

  usb_csr_clear_flag(0, AT91C_UDP_RXSETUP);


  value16 = 0;


  /* let's see what the host want from us */

  switch (packet.request)
    {
    case (USB_BREQUEST_GET_STATUS):

      switch (packet.request_attrs & USB_BMREQUEST_RCPT)
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
      usb_state.new_device_address = packet.value;

      /* we ack */
      usb_send_null();

      /* we will wait for an interruption telling us that TXCOMP is
       * set to 1 now.
       * so for now, that's all.
       */

      /* if the address must be reset to 0, we do it immediatly,
       * because this driver is not made to manage this kind of
       * situation else
       */
      if (usb_state.new_device_address == 0) {
	/* we set the specified usb address in the controller */
	*AT91C_UDP_FADDR    = AT91C_UDP_FEN | 0;
	/* and we tell the controller that we are not in addressed mode anymore  */
	*AT91C_UDP_GLBSTATE = 0;
      }

      break;

    case (USB_BREQUEST_GET_DESCRIPTOR):
      /* the host want some informations about us */
      index = (packet.value & USB_WVALUE_INDEX);

      switch ((packet.value & USB_WVALUE_TYPE) >> 8)
	{
	case (USB_DESC_TYPE_DEVICE):
	  /* it wants infos about the device */
	  size = usb_device_descriptor[0];
	  usb_send_data(0, usb_device_descriptor,
			MIN(size, packet.length));
	  break;

	case (USB_DESC_TYPE_CONFIG):
	  /* it wants infos about a specific config */
	  /* we have only one configuration so ... */
	  usb_send_data(0, usb_nxos_full_config,
			MIN(usb_nxos_full_config[2], packet.length));
	  if (usb_nxos_full_config[2] < packet.length)
	    usb_send_null();
	  break;

	case (USB_DESC_TYPE_STR):
	  if ((packet.value & USB_WVALUE_INDEX) == 0) {
	    /* the host want to know want language we support */
	    usb_send_data(0, usb_string_desc,
			  MIN(usb_string_desc[0], packet.length));
	  } else {
	    /* the host want a specific string */
	    /* TODO : Check it asks an existing string ! */
	    usb_send_data(0, usb_strings[index-1],
			  MIN(usb_strings[index-1][0],
			      packet.length));
	  }
	  break;

	case (USB_DESC_TYPE_DEVICE_QUALIFIER):
	  size = usb_dev_qualifier_desc[0];
	  usb_send_data(0, usb_dev_qualifier_desc,
	  		MIN(size, packet.length));
	  break;

	default:
	  usb_send_stall(USB_STATUS_UNKNOWN_DESCRIPTOR);
	  break;
      }

      break;

    case (USB_BREQUEST_GET_CONFIG):
      usb_send_data(0, (U8 *)&(usb_state.current_config), 1);
      break;

    case (USB_BREQUEST_SET_CONFIG):
      usb_state.status = USB_READY;
      usb_state.current_config = packet.value;

      /* we ack */
      usb_send_null();

      /* we set the register in configured mode */
      *AT91C_UDP_GLBSTATE = packet.value > 0 ?
	(AT91C_UDP_CONFG | AT91C_UDP_FADDEN)
	:AT91C_UDP_FADDEN;

      AT91C_UDP_CSR[1] = AT91C_UDP_EPEDS | AT91C_UDP_EPTYPE_BULK_OUT;
      AT91C_UDP_CSR[2] = AT91C_UDP_EPEDS | AT91C_UDP_EPTYPE_BULK_IN;
      AT91C_UDP_CSR[3] = 0;
      break;

    case (USB_BREQUEST_GET_INTERFACE):
    case (USB_BREQUEST_SET_DESCRIPTOR):
    default:
      usb_send_stall(USB_STATUS_UNMANAGED_REQUEST);
      break;
    }

  return packet.request;
}



static void usb_isr() {
  U8 endpoint = 127;
  U32 csr, isr;

  isr = *AT91C_UDP_ISR;

  if (AT91C_UDP_CSR[0] & AT91C_UDP_ISOERROR /* == STALLSENT */) {
    /* then it means that we sent a stall, and the host has ack the stall */

    usb_csr_clear_flag(0, AT91C_UDP_FORCESTALL | AT91C_UDP_ISOERROR);
  }


  if (isr & AT91C_UDP_ENDBUSRES) {
    usb_state.status = USB_UNINITIALIZED;

    /* we ack all these interruptions */
    *AT91C_UDP_ICR = AT91C_UDP_ENDBUSRES;
    *AT91C_UDP_ICR = AT91C_UDP_RXSUSP; /* suspend */
    *AT91C_UDP_ICR = AT91C_UDP_RXRSM; /* resume */
    *AT91C_UDP_ICR = AT91C_UDP_SOFINT;
    *AT91C_UDP_ICR = AT91C_UDP_WAKEUP;

    /* we reset the end points */
    *AT91C_UDP_RSTEP = ~0;
    *AT91C_UDP_RSTEP = 0;

    usb_state.current_rx_bank = AT91C_UDP_RX_DATA_BK0;
    usb_state.current_config  = 0;

    /* we redefine how the endpoints must work */
    AT91C_UDP_CSR[0] = AT91C_UDP_EPEDS | AT91C_UDP_EPTYPE_CTRL;

    /* then we activate the irq for the end points 0, 1 and 2 */
    /* and for the suspend / resume */
    *AT91C_UDP_IDR = ~0;
    *AT91C_UDP_IER |= 0x7 /* endpts */ | (0x3 << 8) /* suspend / resume */;


    usb_state.current_rx_bank = AT91C_UDP_RX_DATA_BK0;

    /* we activate the function (i.e. us),
     * and set the usb address 0 */
    *AT91C_UDP_FADDR = AT91C_UDP_FEN | 0x0;

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
    usb_state.pre_suspend_status = usb_state.status;
    usb_state.status = USB_SUSPENDED;
  }

  if (isr & AT91C_UDP_RXRSM) {
    *AT91C_UDP_ICR = AT91C_UDP_RXRSM;
    isr &= ~AT91C_UDP_RXRSM;
    usb_state.status = usb_state.pre_suspend_status;
  }



  for (endpoint = 0; endpoint < N_ENDPOINTS ; endpoint++) {
    if (isr & (1 << endpoint))
      break;
  }


  if (endpoint == 0) {

    if (AT91C_UDP_CSR[0] & AT91C_UDP_RXSETUP) {
      csr = usb_manage_setup_packet();
      return;
    }
  }


  if (endpoint < N_ENDPOINTS) { /* if an endpoint was specified */
    csr = AT91C_UDP_CSR[endpoint];

    if (csr & AT91C_UDP_RX_DATA_BK0
	|| csr & AT91C_UDP_RX_DATA_BK1) {
      usb_read_data(endpoint);
      return;
    }

    if (csr & AT91C_UDP_TXCOMP) {

      /* then it means that we sent a data and the host has acknowledged it */
      usb_state.status = USB_READY; /* TODO: check for race with
                                       usb_send_data further down. */
      /* so first we will reset this flag */
      usb_csr_clear_flag(endpoint, AT91C_UDP_TXCOMP);

      if (usb_state.new_device_address > 0) {
	/* the previous message received was SET_ADDR */
	/* now that the computer ACK our send_null(), we can
	 * set this address for real */

	/* we set the specified usb address in the controller */
	*AT91C_UDP_FADDR    = AT91C_UDP_FEN | usb_state.new_device_address;
	/* and we tell the controller that we are in addressed mode now */
	*AT91C_UDP_GLBSTATE = AT91C_UDP_FADDEN;
	usb_state.new_device_address = 0;
      }


      /* and we will send the following data */
      if (usb_state.tx_len[endpoint] > 0
	  && usb_state.tx_data[endpoint] != NULL) {
	usb_send_data(endpoint, usb_state.tx_data[endpoint],
		      usb_state.tx_len[endpoint]);
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

  //  usb_enable();

  *AT91C_UDP_ICR = 0xFFFFFFFF;

  /* Install the interruption routine */

  /* the first interruption we will get is an ENDBUSRES
   * this interruption is always emit (can't be disable with UDP_IER)
   */
  /* other interruptions will be enabled when needed */
  aic_install_isr(AT91C_ID_UDP, AIC_PRIO_DRIVER,
		  AIC_TRIG_LEVEL, usb_isr);


  interrupts_enable();

  usb_enable();
}


bool usb_can_send() {
  return (usb_state.status == USB_READY);
}


void usb_send(U8 *data, U32 length) {
  if (usb_state.status != USB_READY)
    return;

  /* start sending the data */
  usb_send_data(2, data, length);
}

bool usb_is_connected() {
  return (usb_state.status != USB_UNINITIALIZED);
}


U16 usb_has_data() {
  return usb_state.rx_buffer_size[1];
}


void *usb_get_buffer() {
  return (usb_state.rx_buffer[1]);
}


bool usb_overflowed() {
  return usb_state.rx_overflow;
}

void usb_flush_buffer() {
  usb_state.rx_overflow = FALSE;

  if (usb_state.rx_buffer_size[0] > 0)
    memcpy(usb_state.rx_buffer[1], usb_state.rx_buffer[0],
	   usb_state.rx_buffer_size[0]);

  usb_state.rx_buffer_size[1] = usb_state.rx_buffer_size[0];
  usb_state.rx_buffer_size[0] = 0;
}


U8 usb_status() { /* TODO: remove this, internal state leakage. */
  return usb_state.status;
}



void usb_display_debug_info() { /* TODO: Remove this. */
}
