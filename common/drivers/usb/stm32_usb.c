/**********     INCLUDES        **********/
#include "stm32_usb.h"
#include "stm32_usb_msc.h"
#include "string.h"
#include "assert.h"
#include "stdlib.h"

static volatile usb_dev_descriptor_t usb_device_descriptor =
{
	.bLength = 0x12,
	.bDescriptorType = 0x01,
	.bcdUSB = 0x0200,
	.bDeviceClass = 0x00,
	.bDeviceSubClass = 0x00,
	.bDeviceProtocol = 0x00,
	.bMaxPacketSize0 = 64,
	.idVendor = 0x0000,
	.idProduct = 0xa5a5,
	.bcdDevice = 0x0200,
	.iManufacturer = 0x00,
	.iProduct = 0x00,
	.iSerialNumber = 0x00,
	.bNumConfigurations = 1,
};

/**********		STATIC FUNCTION DECLRATIONS		**********/
static void prv_usb_write_stall(volatile uint32_t* fifo, void* data, uint8_t len);
static void prv_wait_for_tx_fifo_flush();
static void prv_wait_for_idle();
static void usb_ep_out_int_handler(uint8_t ep, uint32_t ir);
static void usb_ep_in_int_handler(uint8_t ep, uint32_t ir);
static void usb_handle_get_descriptor();
/**
 * this handles an interrupt for a new RX.
 */
static void usb_rx_fifo_handler(uint32_t grxstsp);

/**
 * usb_reset_handler:
 * 		desc: handles an interrupt for a USB reset.
 */
static void usb_reset_handler();

/**********		STATIC FUNCTION DEFINITIONS		**********/
static void prv_usb_write_stall(volatile uint32_t* fifo, void* data, uint8_t len)
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

static void prv_wait_for_tx_fifo_flush()
{
	while ((USB_FS->GRSTCTL & USB_OTG_GRSTCTL_TXFFLSH) != 0) {}
}
static void prv_wait_for_idle()
{
	while ((USB_FS->GRSTCTL & USB_OTG_GRSTCTL_AHBIDL) == 0) {}
}

static void usb_ep_out_int_handler(uint8_t ep, uint32_t ir)
{
	//"OUT EP: %x\nRequest Type: %d\n",ir,usb_setup_struct.bRequest

	/****** Status phase receieved. ******/
	if ((ir & USB_OTG_DOEPMSK_OTEPSPRM) == USB_OTG_DOEPMSK_OTEPSPRM)
	{
		USBx_OUTEP(ep)->DOEPTSIZ |= (1 << USB_OTG_DOEPTSIZ_PKTCNT_Pos);		//Maybe need to increase transfer size here too?
        USBx_OUTEP(ep)->DOEPCTL |= USB_OTG_DOEPCTL_CNAK | USB_OTG_DOEPCTL_EPENA;
	}

	/* OUT token recieved on a disabled endpoint. */
	if ((ir & USB_OTG_DOEPMSK_OTEPDM) == USB_OTG_DOEPMSK_OTEPDM)
	{
		//Not sure what to do here.
		USBx_OUTEP(ep)->DOEPINT;
	}

	/****** Transfer complete. *****/
	if ((ir & USB_OTG_DOEPMSK_XFRCM) == USB_OTG_DOEPMSK_XFRCM)
	{
		//The example just does the same thing as for status phase done.
		USBx_OUTEP(ep)->DOEPTSIZ |= (1 << USB_OTG_DOEPTSIZ_PKTCNT_Pos);		//Maybe need to increase transfer size here too?
        USBx_OUTEP(ep)->DOEPCTL |= USB_OTG_DOEPCTL_CNAK | USB_OTG_DOEPCTL_EPENA;
	}

	USBx_OUTEP(ep)->DOEPINT = ir;		//Clear the interrupts.
}

static void usb_ep_in_int_handler(uint8_t ep, uint32_t ir)
{
	//"IN EP: %x\n",ir
	if ((ir & USB_OTG_DIEPINT_XFRC) == USB_OTG_DIEPINT_XFRC)
	{
		USBx_OUTEP(ep)->DOEPTSIZ |= (1 << USB_OTG_DOEPTSIZ_PKTCNT_Pos);
        USBx_OUTEP(ep)->DOEPCTL |= USB_OTG_DOEPCTL_CNAK | USB_OTG_DOEPCTL_EPENA;
        usb_msc_ep_in_handler(ep, ir);
	}

	USBx_INEP(ep)->DIEPINT = ir;
}

void usb_rx_fifo_handler(uint32_t grxstsp)
{
	bool status_phase_start = grxstsp & (1 << 27); //WTF ST no macro for USB_OTG_GRXSTSP_STSPHST???
	uint32_t frame_number = (grxstsp >> 21) & 0xF;	//Again no macro for this, what am i missing?
	usb_packet_sts_t packet_status = (grxstsp & USB_OTG_GRXSTSP_PKTSTS) >> USB_OTG_GRXSTSP_PKTSTS_Pos;
	usb_data_pid_t data_pid = (grxstsp & USB_OTG_GRXSTSP_DPID) >> USB_OTG_GRXSTSP_DPID_Pos; 
	uint32_t byte_count = (grxstsp & USB_OTG_GRXSTSP_BCNT) >> USB_OTG_GRXSTSP_BCNT_Pos;
	uint8_t end_pt_number = grxstsp & USB_OTG_GRXSTSP_EPNUM;

	//Dynamic printf here: "Byte Cnt: %d\nPacket Sts: %d\n", byte_count, packet_status



	/* If it's a setup packet, move it to the setup struct. */
	if(packet_status == USB_PACKET_STS_SETUP_RECIEVED)
	{
		/***** Read the data into the setup struct. *****/
		uint32_t* struct_addr = (uint32_t*)&usb_setup_struct;
		*struct_addr = *USB_DFIFO(0);
		struct_addr++;
		//Dynamic printf here: "Data: %x",*dest_addr
		*struct_addr = *USB_DFIFO(0);

		/****** DEVICE TO HOST ******/
		if (usb_setup_struct.bmRequestType.bit.direction == 1)		
		{
			if (usb_setup_struct.bmRequestType.bit.reciepient == USB_BMREQUESTTYPE_TYPE_STANDARD)
			{
				if (usb_setup_struct.bRequest == USB_BREQUEST_GET_DESCRIPTOR)
				{
					usb_handle_get_descriptor();
				}
			
				else if (usb_setup_struct.bRequest == USB_BREQUEST_GET_STATUS)
				{
					usb_write(USB_DFIFO(0), (void*)&usb_device_status, 2);
				}	
				else
				{
					assert(0);
				}
			}
			else if (usb_setup_struct.bmRequestType.bit.reciepient == USB_BMREQUESTTYPE_TYPE_CLASS)
			{
				usb_msc_process_setup_packet(&usb_setup_struct);
			}

		}
		/***** HOST TO DEVICE *****/
		else
		{
			if (usb_setup_struct.bRequest == USB_BREQUEST_SET_ADDRESS)
			{
				USB_FS_DEVICE->DCFG &= ~(USB_OTG_DCFG_DAD);		//Clear the bits.
				USB_FS_DEVICE->DCFG |= usb_setup_struct.wValue << USB_OTG_DCFG_DAD_Pos;	//Set the bits.
				
				usb_write(USB_DFIFO(0), 0, 0);
			}

			else if (usb_setup_struct.bRequest == USB_BREQUEST_SET_CONFIGURATION)
			{
				USBx_INEP(1)->DIEPINT = 0xFB7F;						//Clears all the IN endpoint interrupts.
				USBx_OUTEP(1)->DOEPINT = 0xFB7F;					//Clears all the OUT endpoint interrupts.
				USB_FS_DEVICE->DAINTMSK |= (1 << 1) | (1 << 17);

				USBx_OUTEP(1)->DOEPTSIZ = 0U;
			  	USBx_OUTEP(1)->DOEPTSIZ |= ((1U << USB_OTG_DOEPTSIZ_PKTCNT_Pos) | 31);		//Packet count.
			    USBx_OUTEP(1)->DOEPCTL |= 0x40
			    						| (0x2 << USB_OTG_DOEPCTL_EPTYP_Pos)
										| USB_OTG_DOEPCTL_EPENA
										| USB_OTG_DOEPCTL_USBAEP
										| USB_OTG_DOEPCTL_CNAK;


			    USBx_INEP(1)->DIEPCTL |= 0x40
			    						| USB_OTG_DIEPCTL_USBAEP
										| (0x2 << USB_OTG_DIEPCTL_EPTYP_Pos)
										| (1 << USB_OTG_DIEPCTL_TXFNUM_Pos)
										| USB_OTG_DIEPCTL_SD0PID_SEVNFRM;

				USB_FS_DEVICE->DOUTEP1MSK = USB_OTG_DOEPEACHMSK1_XFRCM;
				USB_FS_DEVICE->DINEP1MSK = USB_OTG_DIEPEACHMSK1_XFRCM;
				USB_FS_DEVICE->DEACHMSK = USB_OTG_DEACHINTMSK_OEP1INTM
										| USB_OTG_DEACHINTMSK_IEP1INTM;						//OUT EP1 interrupt mask.

			    usb_write(USB_DFIFO(0), 0, 0);
			}
			else
			{
				assert(0);
			}
		}
		memset(&usb_setup_struct, 0, sizeof(usb_setup_packet_t));
	}

	/***** Not for EP0. *****/
	else if (packet_status == USB_PACKET_STS_OUT_PCKT_RECIEVED)
	{
		if (end_pt_number == 1)
		{
			usb_msc_handle_data(byte_count);
		}

		USB_FS_DEVICE->DAINTMSK |= (1 << 17);	//Enable EP1 interrupts.
		//USBx_OUTEP(1)->DOEPCTL |=
	}

}

static void usb_handle_get_descriptor()
{
	usb_desc_types_t desc_type = usb_setup_struct.wValue >> 8;		//Get the upper 8 bits (USB2.0, 9.4.3).
	if (desc_type == USB_DESC_TYPE_DEVICE)
	{
		usb_write(USB_DFIFO(0), (volatile void*)&usb_device_descriptor, 0x12);
	}
	else if (desc_type == USB_DESC_TYPE_CONFIGURATION)
	{
		uint32_t total_size = sizeof(usb_config_packet_t);
		usb_configuration_descriptor.wTotalLength = total_size;
		usb_config_packet_t usb_config_packet;
		memcpy(&usb_config_packet.config_desc, &usb_configuration_descriptor, sizeof(usb_config_descriptor_t));
		memcpy(&usb_config_packet.interface_desc, &usb_interface_descriptor, sizeof(usb_interface_descriptor_t));
		memcpy(&usb_config_packet.ep_desc_1, &usb_endpoint_descriptor_IN1, sizeof(usb_endpoint_descriptor_t));
		memcpy(&usb_config_packet.ep_desc_2, &usb_endpoint_descriptor_OUT1, sizeof(usb_endpoint_descriptor_t));
		
		uint32_t requested_length = usb_setup_struct.wLength;
		if (total_size <= requested_length)
		{
			usb_write(USB_DFIFO(0), (void*)&usb_config_packet, total_size);
		}
		else
		{
			usb_write(USB_DFIFO(0), (void*)&usb_configuration_descriptor, requested_length);
		}
	}
	else if (desc_type == USB_DESC_TYPE_DEVICE_QUALIFIER)
	{
		USBx_INEP(0)->DIEPTSIZ = (1 << USB_OTG_DIEPTSIZ_PKTCNT_Pos);
		USBx_INEP(0)->DIEPCTL |= USB_OTG_DIEPCTL_EPENA | USB_OTG_DIEPCTL_STALL | USB_OTG_DIEPCTL_CNAK;
		USBx_OUTEP(0)->DOEPTSIZ = (1 << USB_OTG_DOEPTSIZ_PKTCNT_Pos);
		USBx_OUTEP(0)->DOEPCTL |= USB_OTG_DOEPCTL_EPENA | USB_OTG_DOEPCTL_STALL;
	}
	else if (desc_type == USB_DESC_TYPE_STRING)
	{
		/* Index is the low byte of wValue */
		uint8_t index = usb_setup_struct.wValue & 0xFF;
		uint16_t lang_id = usb_setup_struct.wIndex.val;
		if (index == 0)
		{
			uint8_t string_desc[4];
			string_desc[0] = 0x4;
			string_desc[1] = USB_DESC_TYPE_STRING;
			string_desc[2] = 0x09;
			string_desc[3] = 0x04;

			usb_write(USB_DFIFO(0), (void*)&string_desc, 8);
		}
		else if (index == 1 && lang_id == 0x0409)
		{
			uint8_t strng[8];
			strng[0] = 8;
			strng[1] = 0x04;
			strng[2] = 'c';
			strng[3] = 0x00;
			strng[4] = 'a';
			strng[5] = 0x00;
			strng[6] = 'n';
			strng[7] = 0x00;

			usb_write(USB_DFIFO(0), (void*)&strng, 8);
		}
		else if (index == 2  && lang_id == 0x0409)
		{
			uint8_t strng[8];
			strng[0] = 8;
			strng[1] = 0x03;
			strng[2] = 'u';
			strng[3] = 0x00;
			strng[4] = 'g';
			strng[5] = 0x00;
			strng[6] = 'h';
			strng[7] = 0x00;
			usb_write(USB_DFIFO(0), (void*)&strng, 6);
		}

	}
	else
	{
		assert(0);
	}
}

static void usb_reset_handler()
{
    USB_FS_DEVICE->DCTL &= ~USB_OTG_DCTL_RWUSIG;	//Clearing the remote wakeup signaling bit.
	
	/*Flush TX FIFOs.*/
	prv_wait_for_idle();
	USB_FS->GRSTCTL = (USB_OTG_GRSTCTL_TXFFLSH | (15 << USB_OTG_GRSTCTL_TXFNUM_Pos));
	while ((USB_FS->GRSTCTL & USB_OTG_GRSTCTL_TXFFLSH) == USB_OTG_GRSTCTL_TXFFLSH) {}

	/*Flush the RX FIFOs.*/
	prv_wait_for_idle();
	USB_FS->GRSTCTL = USB_OTG_GRSTCTL_RXFFLSH;
	while ((USB_FS->GRSTCTL & USB_OTG_GRSTCTL_RXFFLSH) == USB_OTG_GRSTCTL_RXFFLSH) {}

	/* Endpoint interrupts. */
	USBx_INEP(0)->DIEPINT = 0xFB7F;						//Clears all the IN endpoint interrupts.
	USBx_OUTEP(0)->DOEPINT = 0xFB7F;					//Clears all the OUT endpoint interrupts.
	USB_FS_DEVICE->DAINTMSK = 1 | (1 << 16);			//Enable interrupts for IN EP0 and OUT EP0.
	USB_FS_DEVICE->DOEPMSK = USB_OTG_DOEPMSK_XFRCM		//Transfer complete.
							| USB_OTG_DOEPMSK_OTEPDM	//OUT token received with endpoint disabled.
							| USB_OTG_DOEPMSK_OTEPSPRM;	//Status phase received.

	USB_FS_DEVICE->DIEPMSK = USB_OTG_DIEPMSK_XFRCM;		//Transfer complete.

	/* Configure the OUT EP0 for setup packets. */
	USBx_OUTEP(0U)->DOEPTSIZ = 0U;
  	USBx_OUTEP(0U)->DOEPTSIZ |= (USB_OTG_DOEPTSIZ_PKTCNT & (1U << 19));		//Packet count.
  	USBx_OUTEP(0U)->DOEPTSIZ |=  1 << USB_OTG_DOEPTSIZ_STUPCNT_Pos;					//Setup packet size count = 3.
    USBx_OUTEP(0)->DOEPCTL |= USB_OTG_DOEPCTL_SNAK;		//I think this means any write to this OUT endpoint will be NAK'd.

    /* Configure EP 1 for MSC. */
   // USBx_OUTEP(1)->DOEPCTL
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

	NVIC_SetPriority(OTG_FS_IRQn, 0x9);
	NVIC_EnableIRQ(OTG_FS_IRQn);
	NVIC_EnableIRQ(OTG_FS_EP1_OUT_IRQn);
	NVIC_EnableIRQ(OTG_FS_EP1_IN_IRQn);
}

uint16_t usb_get_frame_number()
{
	return ((USB_FS_DEVICE->DSTS >> 8) & 0x3FFF);
}

uint32_t usb_read(uint8_t ep)
{
	return *USB_DFIFO(ep);
}

void usb_write_fifo1(volatile uint32_t* fifo, volatile void* data, uint8_t len)
{
	USBx_INEP(1)->DIEPTSIZ = (1 << USB_OTG_DIEPTSIZ_PKTCNT_Pos) | len;
	USBx_INEP(1)->DIEPCTL |= USB_OTG_DIEPCTL_EPENA | USB_OTG_DIEPCTL_CNAK;


	uint32_t fifoWord;
	volatile uint32_t* buffer = (uint32_t*)data;
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
    /* We're only using EP0 right now so set up that endpoint to transmit. */
	//USBx_INEP(1)->DIEPCTL |= USB_OTG_DIEPCTL_CNAK;

}

void usb_write(volatile uint32_t* fifo, volatile void* data, uint8_t len)
{
	USBx_INEP(0)->DIEPTSIZ = (1 << USB_OTG_DIEPTSIZ_PKTCNT_Pos) | len;
	USBx_INEP(0)->DIEPCTL |= USB_OTG_DIEPCTL_EPENA | USB_OTG_DIEPCTL_CNAK;

	uint32_t fifoWord;
    volatile uint32_t* buffer = (volatile uint32_t*)data;
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
    /* We're only using EP0 right now so set up that endpoint to transmit. */
	//USBx_INEP(0)->DIEPCTL |= USB_OTG_DIEPCTL_EPENA | USB_OTG_DIEPCTL_CNAK;

}

void OTG_FS_EP1_OUT_IRQHandler()
{
	uint32_t endpoint_int = USBx_OUTEP(1)->DOEPINT;
	usb_ep_out_int_handler(1, endpoint_int);
}

void OTG_FS_EP1_IN_IRQHandler()
{
	uint32_t endpoint_int = USBx_INEP(1)->DIEPINT;
	usb_ep_in_int_handler(1, endpoint_int);
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

	/****** Handle new RX. ******/
	if (ir & USB_OTG_GINTSTS_RXFLVL)
	{
		uint32_t status = USB_FS->GRXSTSP;
		usb_rx_fifo_handler(status);
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
		usb_ep_out_int_handler(0, endpoint_int);
	}

	/****** IN endpoint interrupt. ******/
	if (ir & USB_OTG_GINTSTS_IEPINT)
	{
		/**
		 * The only IN endpoint interrupt that is enabled right now is transfer
		 * complete on IN EP0.
		 * */
		uint32_t endpoint_int = USBx_INEP(0)->DIEPINT;
		usb_ep_in_int_handler(0, endpoint_int);
	}

	if (ir & USB_OTG_GINTSTS_ESUSP)
	{
		usb_init();
		usb_init_core();
		usb_clear_gintsts_bit(USB_OTG_GINTSTS_ESUSP);
	}
	//usb_set_gintmsk();
}


