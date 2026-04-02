/**********     INCLUDES        **********/
#include "stm32_usb_msc.h"
#include "stm32_usb.h"
#include "stdlib.h"
#include "assert.h"
#include "stdbool.h"

/**********     TYPEDEFS        ***********/
typedef enum
{
    USB_MSC_BREQUEST_GET_MAX_LUN    = 0xFE,
    USB_MSC_BREQUEST_RESET          = 0xFF,
}usb_msc_bRequest_t;

typedef enum
{
	USB_MSC_SCSI_CMD_INQUIRY			= 0x12,
	USB_MSC_SCSI_READ_FORMAT_CAPACITIES	= 0x23,
	USB_MSC_SCSI_READ_CAPACITIES		= 0x25,
	USB_MSC_SCSI_MODE_SENSE_6			= 0x1A,
}usb_msc_ufi_cmd_t;

typedef struct __attribute__((packed))
{
	uint32_t dCBWSignature;
	uint32_t dCBWTag;
	uint32_t dCBWDataTransferLength;	//Length of the next transfer.
	uint8_t bmCBWFlags;			//0x80 for data-In (device to host), 0x00 for data-OUT (host to device).
	uint8_t bCBWLUM;			//Which logical unit.
	uint8_t bCBWCBLength;		//Length of the CBWCB
	uint8_t CBWCB[16];
	
}usb_msc_cbw_t;		//Command Block Wrapper

typedef struct __attribute__((packed))
{
	uint32_t dCSWSignature;
	uint32_t dCSWTag;
	uint32_t dCSWDataResidue;
	uint8_t bCSWStatus;
}usb_msc_csw_t;		//Command Status Wrapper


typedef struct __attribute__((packed))
{
	union
	{
		struct
		{
			uint8_t reserved[3];
			uint8_t capacity_list_length;
		}bit;
	}capacity_list_header;

	union
	{
		struct
		{
			uint32_t number_of_blocks;
			uint8_t descriptor_code : 2;
			uint8_t reserved : 6;
			uint32_t block_length : 24;
		}bit;
	}max_capacity_desc;
}usb_msc_read_format_response_t;


/**********		DEFINES		**********/
#define USB_MSC_BLOCK_LENGTH			0x200
#define USB_MSC_NUM_BLOCKS				0x1000

#define USB_MSC_CBW_SIGNATURE			0x43425355
#define USB_MSC_CSW_SIGNATURE			0x53425355


/**********		EXTERNAL VARIABLE DEFINITIONS		**********/

/**********		STATIC VARIABLES		**********/
static uint32_t usb_msc_max_lun = 0x01010101;
static usb_msc_cbw_t usb_msc_cbw_struct;
static bool more_data = false;
static void* data_ptr;
static uint8_t data_size;
usb_msc_csw_t header =
{
	.dCSWSignature = USB_MSC_CSW_SIGNATURE,
	.dCSWTag = 0,
	.dCSWDataResidue = 0,
	.bCSWStatus = 0,
};

uint8_t inquiry_data[36];
usb_msc_read_format_response_t read_format_response =
{
		.capacity_list_header.bit.capacity_list_length = 0,
		.max_capacity_desc.bit.number_of_blocks = USB_MSC_NUM_BLOCKS,
		.max_capacity_desc.bit.descriptor_code = 0x1,
		.max_capacity_desc.bit.block_length = USB_MSC_BLOCK_LENGTH,
};
/**********		STATIC FUNCTION DECLRATIONS		**********/
static void usb_msc_handle_cbw();

/**********		STATIC FUNCTION DEFINITIONS		**********/
static void usb_msc_handle_cbw()
{
	usb_msc_ufi_cmd_t command = usb_msc_cbw_struct.CBWCB[0];

	uint32_t length = usb_msc_cbw_struct.dCBWDataTransferLength;
	header.dCSWTag = usb_msc_cbw_struct.dCBWTag;

	if (command == USB_MSC_SCSI_CMD_INQUIRY)
	{
		inquiry_data[0] = 0x20;
		inquiry_data[1] = 0x80;		//Sets removable media bit.
		inquiry_data[3] = 0x02;		//2 for SCSI, 1 for UFI.
		inquiry_data[4] = 0x1F;		//

		more_data = true;
		data_ptr = &header;
		data_size = sizeof(usb_msc_csw_t);
		usb_write_fifo1(USB_DFIFO(1), &inquiry_data, 0x24);
	}
	else if (command == USB_MSC_SCSI_READ_FORMAT_CAPACITIES)
	{
		uint32_t residual = length - sizeof(usb_msc_read_format_response_t);
		header.dCSWDataResidue = residual;

		more_data = true;
		data_ptr = &header;
		data_size = sizeof(usb_msc_csw_t);
		usb_write_fifo1(USB_DFIFO(1), &read_format_response, sizeof(usb_msc_read_format_response_t));
	}
	else if (command == USB_MSC_SCSI_READ_CAPACITIES)
	{
		uint32_t read_capacities_response[2];
		read_capacities_response[0] = 0;
		read_capacities_response[1] = 0x200 * 0x1000;
		uint32_t residual = length - 8;
		header.dCSWDataResidue = residual;

		more_data = true;
		data_ptr = &header;
		data_size = sizeof(usb_msc_csw_t);
		usb_write_fifo1(USB_DFIFO(1), &read_capacities_response, 8);
	}
	else if (command == USB_MSC_SCSI_MODE_SENSE_6)
	{
		/* Mode parameter header. */
		uint8_t response[4] = { 0x03, 0x00, 0x00, 0x00,	};
		uint32_t residual = length - 4;
		header.dCSWDataResidue = residual;

		more_data = true;
		data_ptr = &header;
		data_size = sizeof(usb_msc_csw_t);
		usb_write_fifo1(USB_DFIFO(1), &response, 4);
	}
	else
	{
		assert(0);
	}

}

/**********		GLOBAL FUNCTION DEFINITIONS		**********/
void usb_msc_process_setup_packet(usb_setup_packet_t* setup_packet)
{
    if (setup_packet->bRequest == USB_MSC_BREQUEST_GET_MAX_LUN)
    {
		//USBx_INEP(0)->DIEPTSIZ = (1 << USB_OTG_DIEPTSIZ_PKTCNT_Pos);
		//USBx_INEP(0)->DIEPCTL |= USB_OTG_DIEPCTL_EPENA | USB_OTG_DIEPCTL_STALL | USB_OTG_DIEPCTL_CNAK;
		//USBx_OUTEP(0)->DOEPCTL |= USB_OTG_DOEPCTL_EPENA | USB_OTG_DOEPCTL_STALL;
        usb_write(USB_DFIFO(0), &usb_msc_max_lun, 1);
    }
}

void usb_msc_handle_data(uint32_t length)
{
	/* Check if it's a CBW. */
	if (length == 31)
	{
		uint8_t* dest_addr = (uint8_t*)&usb_msc_cbw_struct;
		uint32_t offset = 8;
		while (offset--)
		{
			*(uint32_t*)dest_addr = *(uint32_t*)USB_DFIFO(0);
			dest_addr += 4;
		}
		usb_msc_handle_cbw();

	}
	else
	{
		assert(0);
	}
}

void usb_msc_ep_in_handler(uint32_t ep, uint32_t ir)
{
	if (ep != 1)
	{
		return;
	}

	if (more_data == false)
	{
		return;
	}
	usb_write_fifo1(USB_DFIFO(1), data_ptr, data_size);
	more_data = false;

}
