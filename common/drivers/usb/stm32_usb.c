/**********     INCLUDES        **********/
#include "stm32_usb.h"
#include "string.h"
#include "assert.h"

/**********     TYPEDEFS         **********/
typedef enum
{
	USB_PACKET_STS_OUT_NAK = 1,
	USB_PACKET_STS_OUT_PCKT_RECIEVED = 2,
	USB_PACKET_STS_OUT_TX_COMPLETED = 3,
	USB_PACKET_STS_SETUP_COMPLETED = 4,
	USB_PACKET_STS_SETUP_RECIEVED = 6,
}usb_packet_sts_t;

typedef enum
{
	USB_DATA_PID_DATA0,
	USB_DATA_PID_DATA2,		//No this is not a typo, 2 comes before 1. RM0399 Rev 4 pg 3556.
	USB_DATA_PID_DATA1,
	USB_DATA_PID_MDATA,
}usb_data_pid_t;

/* bRequest type is a field in the setup packet. */
typedef enum
{
	USB_BREQUEST_GET_STATUS			= 0,
	USB_BREQUEST_CLEAR_FEATURE		= 1,
	USB_BREQUEST_SET_FEATURE		= 3,
	USB_BREQUEST_SET_ADDRESS		= 5,
	USB_BREQUEST_GET_DESCRIPTOR		= 6,
	USB_BREQUEST_SET_DESCRIPTOR		= 7,
	USB_BREQUEST_GET_CONFIGURATION	= 8,
	USB_BREQUEST_SET_CONFIGURATION	= 9,
	USB_BREQUEST_GET_INTERFACE		= 10,
	USB_BREQUEST_SET_INTERFACE		= 11,
	USB_BREQUEST_SYNCH_FRAME		= 12

}usb_bRequest_t;

typedef enum
{
	USB_DESC_TYPE_DEVICE 			= 1,
	USB_DESC_TYPE_CONFIGURATION		= 2,
	USB_DESC_TYPE_STRING			= 3,
	USB_DESC_TYPE_INTERFACE			= 4,
	USB_DESC_TYPE_ENDPOINT			= 5,
	USB_DESC_TYPE_DEVICE_QUALIFIER	= 6,
	USB_DESC_TYPE_OTHER_SPEED_CONFIGURATION		= 7,
	USB_DESC_TYPE_INTERFACE_POWER	= 8,
}usb_desc_types_t;

typedef struct
{
	union
	{
		struct
		{
			uint8_t reciepient : 5;
			uint8_t type : 2;
			uint8_t direction : 1;			//1 for device to host, 0 for host to device.
		}bit;
	}bmRequestType;

	/*usb_bRequest_t*/uint8_t bRequest;
	uint16_t wValue;
	union
	{
		struct
		{
			uint8_t endpoint_number : 4;
			uint8_t : 3;	//Reserved.
			uint8_t direction : 1;		//Zero indicated OUT endpoint, 1 indicates IN endpoint.
			uint16_t : 8;	//Reserved.
		}bit;
	}wIndex;	//The lower 8 bits are used to specify an interface. Only use the specific bits when specifying an endpoint.

	uint16_t wLength;
} usb_setup_packet_t;

typedef struct 
{
	uint8_t bLength;			//I think this is 0x40, but the setup packet specifies how long it should be.
	uint8_t bDescriptorType;	//"DEVICE descriptor type", 0x01 for device descriptor i think
	uint16_t bcdUSB;			//0x0110 for version 1.1.0
	uint8_t bDeviceClass;		//Will have to look at standard. 0x08 for mass storage.
	uint8_t bDeviceSubClass;	// i think just 0.
	uint8_t bDeviceProtocol;	//I think just zero again.
	uint8_t bMaxPacketSize0;	//Depends on my configuration of OTG_DOEPCTL0
	uint16_t idVendor;			//dunno, assigned by USB-IF
	uint16_t idProduct;			//dunno, assigned by MFG.
	uint16_t bcdDevice;			//device release number in binary coded decimal.
	uint8_t iManufacturer;		//Index
	uint8_t iProduct;			//Index
	uint8_t iSerialNumber;		//Index
	uint8_t bNumConfigurations;		
}usb_dev_descriptor_t;

typedef struct __attribute__((packed))
{
	uint8_t bLength;			//0xFF i think, double check.
	uint8_t bDescriptorType;	//2 for config descriptor.
	uint16_t wTotalLength;		//Total length of data returned for the entire configuration.
	uint8_t bNumInterfaces;		//Number of interfaces supported by this configuration.
	uint8_t bConfigurationValue;	//Value to use as an arguement to the SetConfiguration() request to select this configuration.
	uint8_t iConfiguration;		//Index of string descriptor describing this configuration.
	uint8_t bmAttributes;		//D6-Self powered; D5-remote wakeup; D7,D4-0-reserved.
	uint8_t bMaxPower;			//Expressed in 2x mA (50 = 100mA).
}usb_config_descriptor_t ;



/**********		DEFINES		**********/
#define USB_FS			USB2_OTG_FS								//Bc typing out all that is getting to be a pain in the ass.
#define USB_FS_DEVICE  	((USB_OTG_DeviceTypeDef *) 0x40080800)	//Bc this didnt even exist.

/* Better named defines that what STM provides. */
#define USB_DCFG_DSPD_HS					0x0 << USB_OTG_DCFG_DSPD_Pos
#define USB_DCFG_DSPD_FS_USING_HS			0x1 << USB_OTG_DCFG_DSPD_Pos
#define USB_DCFG_DSPD_FS_INTERNAL			0x3 << USB_OTG_DCFG_DSPD_Pos

#define USB_FS_RX_FIFO_SIZE_WORDS			128							//In terms of 32 bit words, mult by 4 for size in bytes.
#define USB_NON_PRDC_TX_FIFO_SIZE			64							//Non periodic TX fifo size in terms of words.
#define USB_NON_PRDC_TX_FIFO_START_ADDR		USB_FS_RX_FIFO_SIZE_WORDS
#define USB_DIEPTX_FIFO_0_SIZE				128
#define USB_DIEPTX_FIFO_0_START_ADDR		(USB_NON_PRDC_TX_FIFO_START_ADDR + USB_NON_PRDC_TX_FIFO_SIZE)

#define USB_OTG_PCGCCTL    					*(__IO uint32_t *)((uint32_t)USB2_OTG_FS_PERIPH_BASE + USB_OTG_PCGCCTL_BASE)
#define USBx_INEP(i)    					((USB_OTG_INEndpointTypeDef *)(USB2_OTG_FS_PERIPH_BASE\
                                                       + USB_OTG_IN_ENDPOINT_BASE + ((i) * USB_OTG_EP_REG_SIZE)))

#define USBx_OUTEP(i)   					((USB_OTG_OUTEndpointTypeDef *)(USB2_OTG_FS_PERIPH_BASE\
                                                        + USB_OTG_OUT_ENDPOINT_BASE + ((i) * USB_OTG_EP_REG_SIZE)))
#define USB_DFIFO(i)   						(uint32_t*)(USB2_OTG_FS_PERIPH_BASE + USB_OTG_FIFO_BASE + ((i) * USB_OTG_FIFO_SIZE))

#define usb_clear_gintsts_bit(msk)					USB_FS->GINTSTS = msk
#define prv_clear_doepintx_bit(ep, msk)				USBx_OUTEP(ep)->DOEPINT = msk
#define prv_clear_diepintx_bit(ep, msk)				USBx_OUTEP(ep)->DIEPINT = msk
#define usb_set_gintmsk()							USB_FS->GAHBCFG |= USB_OTG_GAHBCFG_GINT_Msk
#define usb_clear_gintmsk()							USB_FS->GAHBCFG &= ~(USB_OTG_GAHBCFG_GINT_Msk)
/**********		EXTERNAL VARIABLE DEFINITIONS		**********/

/**********		STATIC VARIABLES		**********/
static usb_setup_packet_t usb_setup_struct;
static uint16_t usb_device_status = 0;
static usb_dev_descriptor_t usb_device_descriptor = 
{
	.bLength = 0x12,
	.bDescriptorType = 0x01,
	.bcdUSB = 0x011,
	.bDeviceClass = 0x08,
	.bDeviceSubClass = 0x00,
	.bDeviceProtocol = 0x00,
	.bMaxPacketSize0 = 64,
	.idVendor = 0x0000,
	.idProduct = 0xa5a5,
	.bcdDevice = 0x001,
	.iManufacturer = 0x00,
	.iProduct = 0x00,
	.iSerialNumber = 0x00,
	.bNumConfigurations = 1,
};

static usb_config_descriptor_t usb_configuration_descriptor =
{
	.bLength = 0x9,
	.bDescriptorType = 0x2,
	.wTotalLength = 9,
	.bNumInterfaces = 1,
	.bConfigurationValue = 1,
	.iConfiguration = 0x00,
	.bmAttributes = (1 << 6) | (1 << 7),
	.bMaxPower = 150,
};

/**********		STATIC FUNCTION DECLRATIONS		**********/
void prv_usb_write(volatile uint32_t* fifo, void* data, uint8_t len);
void prv_wait_for_tx_fifo_flush();
void prv_wait_for_idle();
void usb_ep_out_int_handler(uint32_t ir);
void usb_ep_in_int_handler(uint32_t ir);

/**
 * this handles an interrupt for a new RX.
 */
void usb_rx_fifo_handler(uint32_t grxstsp);

/**
 * usb_reset_handler:
 * 		desc: handles an interrupt for a USB reset.
 */
void usb_reset_handler();
/**********		STATIC FUNCTION DEFINITIONS		**********/
void prv_usb_write(volatile uint32_t* fifo, void* data, uint8_t len)
{
	/* We're only using EP0 right now so set up that endpoint to transmit. */
	USBx_INEP(0)->DIEPTSIZ = (1 << USB_OTG_DIEPTSIZ_PKTCNT_Pos) | len;
	USBx_INEP(0)->DIEPCTL |= USB_OTG_DIEPCTL_EPENA | USB_OTG_DIEPCTL_CNAK;

	uint32_t fifoWord;
    uint32_t* buffer = (uint32_t*)data;
    uint8_t remains = len;
    for (uint8_t idx = 0; idx < len; idx += 4, remains -= 4, buffer++)
    {
        switch (remains)
        {
            case 0:
                break;
            case 1:
                fifoWord = *buffer & 0xFF;
                *fifo = fifoWord;
                break;
            case 2:
                fifoWord = *buffer & 0xFFFF;
                *fifo = fifoWord;
                break;
            case 3:
                fifoWord = *buffer & 0xFFFFFF;
                *fifo = fifoWord;
                break;
            default:
                *fifo = *buffer;
                break;
        }
    }
}
void prv_wait_for_tx_fifo_flush()
{
	while ((USB_FS->GRSTCTL & USB_OTG_GRSTCTL_TXFFLSH) != 0) {}
}
void prv_wait_for_idle()
{
	while ((USB_FS->GRSTCTL & USB_OTG_GRSTCTL_AHBIDL) == 0) {}
}

void usb_ep_out_int_handler(uint32_t ir)
{
	//"OUT EP: %x\nRequest Type: %d\n",ir,usb_setup_struct.bRequest
	/****** SETUP phase complete. ******/
	if ((ir & USB_OTG_DOEPINT_STUP) == USB_OTG_DOEPINT_STUP)
	{
		if (usb_setup_struct.bmRequestType.bit.direction == 1)		//Device to host.
		{
			if (usb_setup_struct.bRequest == USB_BREQUEST_GET_DESCRIPTOR)
			{
				usb_desc_types_t desc_type = usb_setup_struct.wValue >> 8;		//Get the upper 8 bits (USB2.0, 9.4.3).
				if (desc_type == USB_DESC_TYPE_DEVICE)
				{
					prv_usb_write(USB_DFIFO(0), (void*)&usb_device_descriptor, 0x12);
				}
				if (desc_type == USB_DESC_TYPE_CONFIGURATION)
				{
					prv_usb_write(USB_DFIFO(0), (void*)&usb_configuration_descriptor, 0x9);
				}
			}
			if (usb_setup_struct.bRequest == USB_BREQUEST_GET_STATUS)
			{
				prv_usb_write(USB_DFIFO(0), (void*)&usb_device_status, 2);
			}
		}
		else	//Host to device.
		{
			if (usb_setup_struct.bRequest == USB_BREQUEST_SET_ADDRESS)
			{
				USB_FS_DEVICE->DCFG &= ~(USB_OTG_DCFG_DAD);		//Clear the bits.
				USB_FS_DEVICE->DCFG |= usb_setup_struct.wValue << USB_OTG_DCFG_DAD_Pos;	//Set the bits.
				
				prv_usb_write(USB_DFIFO(0), 0, 0);
			}
		}
		memset(&usb_setup_struct, 0, sizeof(usb_setup_packet_t));
	}

	/****** Status phase receieved. ******/
	if ((ir & USB_OTG_DOEPMSK_OTEPSPRM) == USB_OTG_DOEPMSK_OTEPSPRM)
	{
		USBx_OUTEP(0)->DOEPTSIZ |= (1 << USB_OTG_DOEPTSIZ_PKTCNT_Pos);		//Maybe need to increase transfer size here too?
        USBx_OUTEP(0)->DOEPCTL |= USB_OTG_DOEPCTL_CNAK | USB_OTG_DOEPCTL_EPENA;
	}

	/* OUT token recieved on a disabled endpoint. */
	if ((ir & USB_OTG_DOEPMSK_OTEPDM) == USB_OTG_DOEPMSK_OTEPDM)
	{
		//Not sure what to do here.
		USBx_OUTEP(0)->DOEPINT;
	}

	if ((ir & USB_OTG_DOEPMSK_XFRCM) == USB_OTG_DOEPMSK_XFRCM)
	{
		//The example just does the same thing as for status phase done.
		USBx_OUTEP(0)->DOEPTSIZ |= (1 << USB_OTG_DOEPTSIZ_PKTCNT_Pos);		//Maybe need to increase transfer size here too?
        USBx_OUTEP(0)->DOEPCTL |= USB_OTG_DOEPCTL_CNAK | USB_OTG_DOEPCTL_EPENA;
	}

	USBx_OUTEP(0)->DOEPINT = ir;		//Clear the interrupts.
}

void usb_ep_in_int_handler(uint32_t ir)
{
	//"IN EP: %x\n",ir
	if ((ir & USB_OTG_DIEPINT_XFRC) == USB_OTG_DIEPINT_XFRC)
	{
		USBx_OUTEP(0)->DOEPTSIZ |= (1 << USB_OTG_DOEPTSIZ_PKTCNT_Pos);
        USBx_OUTEP(0)->DOEPCTL |= USB_OTG_DOEPCTL_CNAK | USB_OTG_DOEPCTL_EPENA;
	}

	USBx_INEP(0)->DIEPINT = ir;
}

void usb_rx_fifo_handler(uint32_t grxstsp)
{
	bool status_phase_start = grxstsp & (1 << 27); //WTF ST no macro for USB_OTG_GRXSTSP_STSPHST???
	uint32_t frame_number = (grxstsp >> 21) & 0xF;	//Again no macro for this, what am i missing?
	usb_packet_sts_t packet_status = (grxstsp & USB_OTG_GRXSTSP_PKTSTS) >> USB_OTG_GRXSTSP_PKTSTS_Pos;
	usb_data_pid_t data_pid = (grxstsp & USB_OTG_GRXSTSP_DPID) >> USB_OTG_GRXSTSP_DPID_Pos; 
	uint32_t byte_count = (grxstsp & USB_OTG_GRXSTSP_BCNT) >> USB_OTG_GRXSTSP_BCNT_Pos;
	uint8_t end_pt_number = grxstsp & USB_OTG_GRXSTSP_EPNUM;
	uint32_t buf[64];	//Reading into this for the sake of debugging for now.

	//Dynamic printf here: "Byte Cnt: %d\nPacket Sts: %d\n", byte_count, packet_status

	/* Read all the bytes into the temp buffer from the FIFO. */
	uint32_t* dest_addr = (uint32_t*)&usb_setup_struct;
	//for (uint32_t i = 0; i < byte_count / 4; i++)
	//{
	//	*dest_addr = *USB_DFIFO(end_pt_number);
		//Dynamic printf here: "Data: %x",*dest_addr
	//	dest_addr++;
	//}

	/* If it's a setup packet, move it to the setup struct. */
	if(packet_status == USB_PACKET_STS_SETUP_RECIEVED)
	{
		uint32_t* struct_addr = &usb_setup_struct;
		*struct_addr = *USB_DFIFO(0);
		struct_addr++;
			//Dynamic printf here: "Data: %x",*dest_addr
		*struct_addr = *USB_DFIFO(0);
	}

	//Dynamic printf here: "\n"

}

void usb_reset_handler()
{
    USB_FS_DEVICE->DCTL &= ~USB_OTG_DCTL_RWUSIG;	//Clearing the remote wakeup signaling bit.
	
	/* Flush all the TX FIFOs. */
	prv_wait_for_idle();
	USB_FS->GRSTCTL = (USB_OTG_GRSTCTL_TXFFLSH | (15 << 6));

	/* Endpoint interrupts. */
	USBx_INEP(0)->DIEPINT = 0xFB7F;						//Clears all the IN endpoint interrupts.
	USBx_OUTEP(0)->DOEPINT = 0xFB7F;					//Clears all the OUT endpoint interrupts.
	USB_FS_DEVICE->DAINTMSK = 1 | (1 << 16);			//Enable interrupts for IN EP0 and OUT EP0.
	USB_FS_DEVICE->DOEPMSK = USB_OTG_DOEPMSK_STUPM		//SETUP received.
							| USB_OTG_DOEPMSK_XFRCM		//Transfer complete.
							| USB_OTG_DOEPMSK_OTEPDM	//OUT token received with endpoint disabled.
							| USB_OTG_DOEPMSK_OTEPSPRM;	//Status phase received.

	USB_FS_DEVICE->DIEPMSK = USB_OTG_DIEPMSK_XFRCM;		//Transfer complete.

	/* Configure the OUT EP0 for setup packets. */
	USBx_OUTEP(0U)->DOEPTSIZ = 0U;
  	USBx_OUTEP(0U)->DOEPTSIZ |= (USB_OTG_DOEPTSIZ_PKTCNT & (1U << 19));		//Packet count.
  	//USBx_OUTEP(0U)->DOEPTSIZ |= (3U * 8U);									//Transfer size 24 bytes.
  	USBx_OUTEP(0U)->DOEPTSIZ |=  1 << USB_OTG_DOEPTSIZ_STUPCNT_Pos;					//Setup packet size count = 3.
    USBx_OUTEP(0)->DOEPCTL |= USB_OTG_DOEPCTL_SNAK;		//I think this means any write to this OUT endpoint will be NAK'd.

	/* Set Default Address to 0 */
    USB_FS_DEVICE->DCFG &= ~USB_OTG_DCFG_DAD;
}

/**********		GLOBAL FUNCTION DEFINITIONS		**********/
void usb_init()
{
	/*Enable voltage detector.*/
	PWR->CR3 |= PWR_CR3_USB33DEN;
	
	/* Configure the IO pins. */
	io_set_pin_mux(GPIOA, GPIO_PIN10_Msk, GPIO_AFR_AF10);
	io_set_pin_mux(GPIOA, GPIO_PIN11_Msk, GPIO_AFR_AF10);
	io_set_pin_mux(GPIOA, GPIO_PIN12_Msk, GPIO_AFR_AF10);

	io_set_output_speed(GPIOA, GPIO_PIN10_Msk, GPIO_OSPEEDR_VERY_HIGH);
	io_set_output_speed(GPIOA, GPIO_PIN11_Msk, GPIO_OSPEEDR_VERY_HIGH);
	io_set_output_speed(GPIOA, GPIO_PIN12_Msk, GPIO_OSPEEDR_VERY_HIGH);

	/* Turn on the clocks. */
	RCC->CR |= RCC_CR_HSI48ON;
	while ((RCC->CR & RCC_CR_HSI48RDY) == 0) {}
	RCC->D2CCIP2R |= 0x3 << RCC_D2CCIP2R_USBSEL_Pos;	//Enable kernel clock.

	/* Note: Enabling ULPIEN makes it so the internal PHY wont work. */
	RCC->AHB1ENR |= RCC_AHB1ENR_USB2OTGFSEN;// | RCC_AHB1ENR_USB2OTGFSULPIEN;	//Enable PHY and peripheral clocks.

	/* Peripheral interrupt init, HAL calls this here but dont know if I want or need to yet. */
	//NVIC_EnableIRQ(OTG_FS_IRQn);
}

void usb_core_reset()
{

	/* Select embedded PHY (vs external PHY). */
    USB_FS->GUSBCFG |= USB_OTG_GUSBCFG_PHYSEL;	

	/* Wait for AHB master IDLE state. */
	while (( USB_OTG_FS->GRSTCTL & USB_OTG_GRSTCTL_AHBIDL) == 0U) {}
	/* Core Soft Reset */

	assert(RCC->CR & RCC_CR_HSI48RDY);
	USB_OTG_FS->GRSTCTL |= USB_OTG_GRSTCTL_CSRST;
	while (( USB_OTG_FS->GRSTCTL & USB_OTG_GRSTCTL_CSRST) == USB_OTG_GRSTCTL_CSRST) {}
}

void usb_init_core()
{
    /* Activate the USB Transceiver */
    USB_FS->GCCFG |= USB_OTG_GCCFG_PWRDWN;

	/* Sets force device mode. */
	USB_FS->GUSBCFG &= ~(USB_OTG_GUSBCFG_FHMOD | USB_OTG_GUSBCFG_FDMOD);
	USB_FS->GUSBCFG |= USB_OTG_GUSBCFG_FDMOD;

	/* Waits for current mode to be device mode. */
	while ((USB_FS->GINTSTS & USB_OTG_GINTSTS_CMOD) != 0) {}

	/* Set all the IN endpoint FIFOs size to 0. */
	for (uint32_t i = 0U; i < 15U; i++)
  	{
    	USB_FS->DIEPTXF[i] = 0U;
  	}

	/* Enable soft disconnect. */
	USB_FS_DEVICE->DCTL |= USB_OTG_DCTL_SDIS;

	/* Deactivate VBUS Sensing B */
    USB_FS->GCCFG &= ~USB_OTG_GCCFG_VBDEN;

    /* B-peripheral session valid override enable */ 
	//Need this ??
    USB_FS->GOTGCTL |= USB_OTG_GOTGCTL_BVALOEN;
    USB_FS->GOTGCTL |= USB_OTG_GOTGCTL_BVALOVAL;

  	/* Restart the Phy Clock */
  	USB_OTG_PCGCCTL = 0U;

	/*Set device speed.*/
  	USB_FS_DEVICE->DCFG |= 0x3;

	/*Flush TX FIFOs.*/
	prv_wait_for_idle();
	USB_FS->GRSTCTL = (USB_OTG_GRSTCTL_TXFFLSH | (15 << USB_OTG_GRSTCTL_TXFNUM_Pos));
	while ((USB_FS->GRSTCTL & USB_OTG_GRSTCTL_TXFFLSH) == USB_OTG_GRSTCTL_TXFFLSH) {}

	/*Flush the RX FIFOs.*/
	prv_wait_for_idle();
	USB_FS->GRSTCTL = USB_OTG_GRSTCTL_RXFFLSH;
	while ((USB_FS->GRSTCTL & USB_OTG_GRSTCTL_RXFFLSH) == USB_OTG_GRSTCTL_RXFFLSH) {}

	/* In case phy is stopped, ensure to ungate and restore the phy CLK */
  	USB_OTG_PCGCCTL &= ~(USB_OTG_PCGCCTL_STOPCLK | USB_OTG_PCGCCTL_GATECLK);

	/* Set FIFO sizes. */
	USB_FS->GRXFSIZ = 128;								//This accounts for all OUT endpoints.
	USB_FS->DIEPTXF0_HNPTXFSIZ = (64 << 16) | 128;		//IN endpoint 0.
	USB_FS->DIEPTXF[0] = (128 << 16) | 192;				//IN endpoint 1 (does this need to be index 1)?

	/* Clear all pending Device Interrupts */
  	USB_FS_DEVICE->DIEPMSK = 0U;
  	USB_FS_DEVICE->DOEPMSK = 0U;
  	USB_FS_DEVICE->DAINTMSK = 0U;

  	/* Disable all interrupts. */
  	USB_FS->GINTMSK = 0U;

  	/* Clear any pending interrupts */
  	USB_FS->GINTSTS = 0xBFFFFFFFU;

	/* Enable interrupts matching to the Device mode ONLY */
  	USB_FS->GINTMSK |= USB_OTG_GINTMSK_RXFLVLM | USB_OTG_GINTMSK_USBRST |
                   USB_OTG_GINTMSK_ENUMDNEM | USB_OTG_GINTMSK_IEPINT |
                   USB_OTG_GINTMSK_OEPINT;
	/** Endpoint interrupts are enabled in the USB reset handler. **/

	/* Global interrupt enable bit. */
  	USB_FS->GAHBCFG |= USB_OTG_GAHBCFG_GINT;

	/* In case phy is stopped, ensure to ungate and restore the phy CLK */
  	USB_OTG_PCGCCTL &= ~(USB_OTG_PCGCCTL_STOPCLK | USB_OTG_PCGCCTL_GATECLK);
	USB_FS_DEVICE->DCTL &= ~USB_OTG_DCTL_SDIS;

	NVIC_EnableIRQ(OTG_FS_IRQn);
}

void OTG_FS_EP1_OUT_IRQHandler()
{
	assert(0);
}

void OTG_FS_EP1_IN_IRQHandler()
{
	assert(0);
}
void OTG_FS_WKUP_IRQHandler()
{
	assert(0);
}
void OTG_FS_IRQHandler()
{
	//usb_clear_gintmsk();
	uint32_t ir = USB_OTG_FS->GINTSTS;		//Read the interrupt status register.
	ir &= USB_OTG_FS->GINTMSK;				//Filter it against the enabled interrupts.

	//uint32_t all_endpoints_ir = USB_FS_DEVICE->DAINT;
	//uint32_t in_ep0_ir = USBx_INEP(0)->DIEPINT;
	//uint32_t out_ep0_ir = USBx_OUTEP(0)->DOEPINT;

	//"INT started: %x",ir
	/* If there's no interrupt bits set, return. */
	if (ir == 0)
	{
		return;
	}

	/****** Handle new RX. ******/
	if (ir & USB_OTG_GINTSTS_RXFLVL)
	{
		uint32_t status = USB_FS->GRXSTSP;
		usb_rx_fifo_handler(status);
	}

	/****** USB Reset handler. *******/
	if (ir & USB_OTG_GINTSTS_USBRST)
	{
		usb_reset_handler();
		usb_clear_gintsts_bit(USB_OTG_GINTSTS_USBRST);
	}

	/****** Enumeration done handler. ******/
	if (ir & USB_OTG_GINTSTS_ENUMDNE)
	{
		uint32_t enum_speed = USB_FS_DEVICE->DSTS & USB_OTG_DSTS_ENUMSPD;
		USB_FS->GUSBCFG &= ~(USB_OTG_GUSBCFG_TRDT);				//Clear the turn around time bits.
		USB_FS->GUSBCFG |= 0x6 << USB_OTG_GUSBCFG_TRDT_Pos;		//Set turnaround time to 6 (this is what HAL uses for a 120MHz AHB clk).
		usb_clear_gintsts_bit(USB_OTG_GINTSTS_ENUMDNE);
	}

	/****** OUT endpoint interrupt. ******/
	if (ir & USB_OTG_GINTSTS_OEPINT)
	{
		/* Only have an interrupt enabled for endpoint 0 right now so it has to be coming from that one. */

		/**
		 * Get the endpoint interrupt.
		 * Has to be either SETUP packet received, transfer complete,
		 * OUT token received with endpoint disabled, or status phase received. 
		 * */
		uint32_t endpoint_int = USBx_OUTEP(0)->DOEPINT;
		usb_ep_out_int_handler(endpoint_int);
	}

	/****** IN endpoint interrupt. ******/
	if (ir & USB_OTG_GINTSTS_IEPINT)
	{
		/**
		 * The only IN endpoint interrupt that is enabled right now is transfer
		 * complete on IN EP0.
		 * */
		uint32_t endpoint_int = USBx_INEP(0)->DIEPINT;
		usb_ep_in_int_handler(endpoint_int);
	}
	//usb_set_gintmsk();
}
