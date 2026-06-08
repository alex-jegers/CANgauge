
#ifndef _STM32_USB_H_
#define _STM32_USB_H_

#ifdef __cplusplus
extern "C" {
#endif

/**********     INCLUDES        **********/
#include "stm32h745xx.h"
#include "drivers/drivers.h"

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

typedef enum
{
	USB_BMREQUESTTYPE_TYPE_STANDARD,
	USB_BMREQUESTTYPE_TYPE_CLASS,
	USB_BMREQUESTTYPE_TYPE_VENDOR,
	USB_BMREQUESTTYPE_TYPE_RESERVED,
}usb_bmRequestTypeType_t;

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

typedef struct __attribute__((packed))
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
			uint8_t : 8;	//Reserved.
		}bit;
		uint16_t val;
	}wIndex;	//The lower 8 bits are used to specify an interface. Only use the specific bits when specifying an endpoint.

	uint16_t wLength;
} usb_setup_packet_t;

typedef struct __attribute__((packed))
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

typedef struct __attribute__((packed))
{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bInterfaceNumber;
	uint8_t bAlternateSetting;
	uint8_t bNumEndpoints;
	uint8_t bInterfaceClass;
	uint8_t bInterfaceSubClass;
	uint8_t bInterfaceProtocol;
	uint8_t iInterface;
}usb_interface_descriptor_t;

typedef struct __attribute__((packed))
{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bEndpointAddress;		//Bit 1-3: The endpoint number. Bit 4-6: Reserved set to 0. Bit 7: 1 for IN endpoint, 0 for OUT.
	uint8_t bmAttributes;
	uint16_t wMaxPacketSize;
	uint8_t bInterval;
}usb_endpoint_descriptor_t;

typedef struct __attribute__((packed))
{
	usb_config_descriptor_t config_desc;
	usb_interface_descriptor_t interface_desc;
	usb_endpoint_descriptor_t ep_desc_1;
	usb_endpoint_descriptor_t ep_desc_2;
}usb_config_packet_t;

/**********		DEFINES		**********/
#define USB_FS			USB2_OTG_FS								//Bc typing out all that is getting to be a pain in the ass.
#define USB_FS_DEVICE  	((__IO USB_OTG_DeviceTypeDef *) 0x40080800)	//Bc this didnt even exist.

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
#define USBx_INEP(i)    					((__IO USB_OTG_INEndpointTypeDef*)(USB2_OTG_FS_PERIPH_BASE\
                                                       + USB_OTG_IN_ENDPOINT_BASE + ((i) * USB_OTG_EP_REG_SIZE)))

#define USBx_OUTEP(i)   					((__IO USB_OTG_OUTEndpointTypeDef *)(USB2_OTG_FS_PERIPH_BASE\
                                                        + USB_OTG_OUT_ENDPOINT_BASE + ((i) * USB_OTG_EP_REG_SIZE)))
#define USB_DFIFO(i)   						(__IO uint32_t*)(USB2_OTG_FS_PERIPH_BASE + USB_OTG_FIFO_BASE + ((i) * USB_OTG_FIFO_SIZE))

#define USB_LANG_ID_ENGLISH_USA					0x0409

#define usb_clear_gintsts_bit(msk)					USB_FS->GINTSTS = msk; while (USB_FS->GINTSTS & msk){}
#define prv_clear_doepintx_bit(ep, msk)				USBx_OUTEP(ep)->DOEPINT = msk
#define prv_clear_diepintx_bit(ep, msk)				USBx_OUTEP(ep)->DIEPINT = msk
#define usb_set_gintmsk()							USB_FS->GAHBCFG |= USB_OTG_GAHBCFG_GINT_Msk
#define usb_clear_gintmsk()							USB_FS->GAHBCFG &= ~(USB_OTG_GAHBCFG_GINT_Msk)
#define usb_set_ep1_intmsk()						USB_FS_DEVICE->DEACHMSK |= USB_OTG_DEACHINTMSK_OEP1INTM | USB_OTG_DEACHINTMSK_IEP1INTM
#define usb_clear_ep1_intmsk()						USB_FS_DEVICE->DEACHMSK &= ~(USB_OTG_DEACHINTMSK_OEP1INTM | USB_OTG_DEACHINTMSK_IEP1INTM)
/**********		EXTERNAL VARIABLE DEFINITIONS		**********/

/**********		STATIC VARIABLES		**********/
static usb_setup_packet_t usb_setup_struct;
static uint16_t usb_device_status = 0;

static usb_config_descriptor_t usb_configuration_descriptor =
{
	.bLength = 0x9,
	.bDescriptorType = 0x2,
	.wTotalLength = 0,		//Set later.
	.bNumInterfaces = 1,
	.bConfigurationValue = 1,
	.iConfiguration = 0x0,
	.bmAttributes = (1 << 6) | (1 << 7),
	.bMaxPower = 150,
};

static usb_interface_descriptor_t usb_interface_descriptor =
{
	.bLength = 0x9,
	.bDescriptorType = USB_DESC_TYPE_INTERFACE,		//4
	.bInterfaceNumber = 0,
	.bAlternateSetting = 0,
	.bNumEndpoints = 2,
	.bInterfaceClass = 0x08,		//Mass Storage Class
	.bInterfaceSubClass = 0x06,		//SCSI transparent command set.
	.bInterfaceProtocol = 0x50,		//Bulk only communication.
	.iInterface = 0x00,
};

static usb_endpoint_descriptor_t usb_endpoint_descriptor_IN1 =
{
	.bLength = 0x07,
	.bDescriptorType = USB_DESC_TYPE_ENDPOINT,
	.bEndpointAddress = (1 << 7) | 1,			//IN EP, num 1.
	.bmAttributes = 0x02,
	.wMaxPacketSize = 0x40,
	.bInterval = 0
};

static usb_endpoint_descriptor_t usb_endpoint_descriptor_OUT1 =
{
	.bLength = 0x07,
	.bDescriptorType = USB_DESC_TYPE_ENDPOINT,
	.bEndpointAddress = (0 << 7) | 1,			//OUT EP, num 1.
	.bmAttributes = 0x02,
	.wMaxPacketSize = 0x40,
	.bInterval = 0
};

/**********     GLOBAL VARIABLE DECLRATIONS     **********/

/**********		GLOBAL FUNCTION DECLRATIONS		**********/
/**
 * usb_init:
 *      desc: turns on the clocks and IO for USB FS. Does not do anything
 *          with the USB core. usb_core_reset has to be called next.
 */
void usb_init();

/**
 * usb_core_reset:
 *      desc: resets the USB core. Must wait 3 USB PHY clock cycles to call any USB functions
 *          after this funciton returns.
 */
void usb_core_reset();

/**
 * usb_init_core:
 *      desc: initializes the USB core and device descriptors. Call after usb_core_reset.
 */
void usb_init_core();

/** usb_get_frame_number:
 * 		returns: the last SOF frame number from USB device DSTS register.
 */
uint16_t usb_get_frame_number();

void usb_write(volatile uint32_t* fifo, volatile void* data, uint8_t len);
void usb_write_fifo1(volatile uint32_t* fifo, volatile void* data, uint8_t len);
uint32_t usb_read(uint8_t ep);


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif  //_STM32_USB_H_
