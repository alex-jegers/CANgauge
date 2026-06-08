/**********     INCLUDES        **********/
#include "stm32_usb_msc.h"
#include "stm32_usb.h"
#include "stdlib.h"
#include "assert.h"
#include "stdbool.h"

#include "system/system_mem.h"

/**********     TYPEDEFS        ***********/
typedef enum
{
    USB_MSC_BREQUEST_GET_MAX_LUN    = 0xFE,
    USB_MSC_BREQUEST_RESET          = 0xFF,
}usb_msc_bRequest_t;

typedef enum
{
	USB_MSC_SCSI_REZERO_UNIT			= 0x01,
	USB_MSC_SCSI_CMD_INQUIRY			= 0x12,
	USB_MSC_SCSI_READ_FORMAT_CAPACITIES	= 0x23,
	USB_MSC_SCSI_READ_CAPACITIES		= 0x25,
	USB_MSC_SCSI_MODE_SENSE_6			= 0x1A,
	USB_MSC_SCSI_READ_10				= 0x28,
	USB_MSC_SCSI_TEST_UNIT_READY		= 0x00,
	USB_MSC_SCSI_MEDIUM_REMOVAL			= 0x1E,
	USB_MSC_SCSI_WRITE_10				= 0x2A,
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
#define USB_MSC_NUM_BLOCKS				0x4000

#define USB_MSC_CBW_SIGNATURE			0x43425355
#define USB_MSC_CSW_SIGNATURE			0x53425355


/**********		EXTERNAL VARIABLE DEFINITIONS		**********/

/**********		STATIC VARIABLES		**********/
static uint32_t usb_msc_max_lun = 0x01010101;
static usb_msc_cbw_t usb_msc_cbw_struct;
static void (*prv_msc_read_cb)(uint32_t lba, uint32_t num_blocks);
static void (*prv_msc_write_cb)(uint32_t lba, uint32_t num_blocks);
static bool (*prv_msc_write_complete_cb)(uint32_t length_bytes);

struct more_data_ll
{
	void* data_ptr;
	uint8_t data_size;
	struct more_data_ll* next_node;
};

struct more_data_ll more_data_header =
{
	.next_node = NULL,
};

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
		.capacity_list_header.bit.capacity_list_length = 8,
		.max_capacity_desc.bit.number_of_blocks = USB_MSC_NUM_BLOCKS,
		.max_capacity_desc.bit.descriptor_code = 0x1,
		.max_capacity_desc.bit.block_length = USB_MSC_BLOCK_LENGTH,
};

uint8_t* storage;

static uint32_t wr_lba;
static uint16_t wr_num_blocks;
static uint32_t* wr_start_addr;
static uint32_t wr_transfer_length = 0;


/**********		STATIC FUNCTION DECLRATIONS		**********/
static void add_more_data(void* data_ptr, uint8_t size);
static void usb_msc_handle_cbw();

/**********		STATIC FUNCTION DEFINITIONS		**********/
static void add_more_data(void* data_ptr, uint8_t size)
{
	struct more_data_ll* temp = &more_data_header;
	/* Get to the end of the ll. */
	while (temp->next_node != NULL)
	{
		temp = temp->next_node;
	}
	struct more_data_ll* next_data = calloc(sizeof(struct more_data_ll), 1);
	next_data->data_size = size;
	next_data->data_ptr = data_ptr;
	temp->next_node = next_data;
}

static void usb_msc_handle_cbw()
{
	storage = sys_mem_get_ram_fs_ptr();
	usb_msc_ufi_cmd_t command = usb_msc_cbw_struct.CBWCB[0];

	uint32_t length = usb_msc_cbw_struct.dCBWDataTransferLength;
	header.dCSWTag = usb_msc_cbw_struct.dCBWTag;

	if (command == USB_MSC_SCSI_CMD_INQUIRY)
	{
		inquiry_data[1] = 0x80;		//Sets removable media bit.
		inquiry_data[3] = 0x02;		//2 for SCSI, 1 for UFI.
		inquiry_data[4] = 0x1F;		//
		
		/* Vendor info. */
		inquiry_data[8] = 0x53;
		inquiry_data[9] = 0x45;
		inquiry_data[10] = 0x41;
		inquiry_data[11] = 0x47;
		inquiry_data[12] = 0x41;
		inquiry_data[13] = 0x54;
		inquiry_data[14] = 0x45;
		inquiry_data[15] = 0x20;

		/*Product ID. */
		inquiry_data[16] = 0x75;		//From HAL
		inquiry_data[17] = 0x63;		//From HAL
		inquiry_data[18] = 0x20;
		inquiry_data[19] = 0x20;
		inquiry_data[20] = 0x20;
		inquiry_data[21] = 0x20;
		inquiry_data[22] = 0x20;
		inquiry_data[23] = 0x20;
		inquiry_data[24] = 0x20;
		inquiry_data[25] = 0x20;
		inquiry_data[26] = 0x20;
		inquiry_data[27] = 0x20;
		inquiry_data[28] = 0x20;
		inquiry_data[29] = 0x20;
		inquiry_data[30] = 0x20;
		inquiry_data[31] = 0x20;

		/*Product Rev. */
		inquiry_data[32] = 0x30;
		inquiry_data[33] = 0x2e;
		inquiry_data[34] = 0x30;
		inquiry_data[35] = 0x31;

		add_more_data(&header, sizeof(usb_msc_csw_t));
		usb_write_fifo1(USB_DFIFO(1), &inquiry_data, 0x24);
	}
	else if (command == USB_MSC_SCSI_READ_FORMAT_CAPACITIES)		//From UFI, not in SCSI.
	{
		uint8_t rd_fmt_capacities[12];
		uint32_t size = usb_msc_get_block_size();
		uint32_t num = usb_msc_get_num_blocks() - 1;


		rd_fmt_capacities[3] = 0x80;			//Capacity list length.

		rd_fmt_capacities[4] = (num >> 24) & 0xFF;//0x00;			//Number of blocks MSB.
		rd_fmt_capacities[5] = (num >> 16) & 0xFF; //0x00;
		rd_fmt_capacities[6] = (num >> 8) & 0xFF; //0x01;
		rd_fmt_capacities[7] = (num & 0xFF); //0xff;			//Number of blocks LSB.

		rd_fmt_capacities[8] = 0x02;			//Descriptor Code.

		rd_fmt_capacities[9] = (size >> 16) & 0xFF;//0x00;			//Block length MSB.
		rd_fmt_capacities[10] = (size >> 8) & 0xFF;//0x02;
		rd_fmt_capacities[11] = (size & 0xFF);//0x00;			//Block length LSB.
		uint32_t residual = length - sizeof(rd_fmt_capacities);
		header.dCSWDataResidue = residual;

		add_more_data(&header, sizeof(usb_msc_csw_t));
		usb_write_fifo1(USB_DFIFO(1), &rd_fmt_capacities, sizeof(rd_fmt_capacities));
	}
	else if (command == USB_MSC_SCSI_READ_CAPACITIES)		//From SCSI
	{
		uint8_t read_capacities_response[8];
		uint32_t size = usb_msc_get_block_size();
		uint32_t num = usb_msc_get_num_blocks() - 1;

		read_capacities_response[0] = (num >> 24) & 0xFF;//0x00;		//Largest LBA MSB.
		read_capacities_response[1] = (num >> 16) & 0xFF; //0x00;
		read_capacities_response[2] = (num >> 8) & 0xFF; //0x01;
		read_capacities_response[3] = (num & 0xFF); //0xff;		//Largest LBA LSB.

		read_capacities_response[4] = (size >> 24) & 0xFF;//0x00;		//Block length MSB.
		read_capacities_response[5] = (size >> 16) & 0xFF;//0x00;
		read_capacities_response[6] = (size >> 8) & 0xFF;//0x02;
		read_capacities_response[7] = (size & 0xFF);//0x00;		//Block length LSB.

		uint32_t residual = length - 8;
		header.dCSWDataResidue = residual;

		add_more_data(&header, sizeof(usb_msc_csw_t));
		usb_write_fifo1(USB_DFIFO(1), &read_capacities_response, 8);
	}
	else if (command == USB_MSC_SCSI_MODE_SENSE_6)
	{
		/* Mode parameter header. */
		uint8_t response[4] = { 0x03, 0x00, 0x00, 0x00,	};
		uint32_t residual = length - 4;
		header.dCSWDataResidue = residual;

		add_more_data(&header, sizeof(usb_msc_csw_t));
		usb_write_fifo1(USB_DFIFO(1), &response, 4);
	}
	else if (command == USB_MSC_SCSI_READ_10)
	{

		//Logical Block Address
		uint32_t lba = usb_msc_cbw_struct.CBWCB[2] << 24
					| usb_msc_cbw_struct.CBWCB[3] << 16
					| usb_msc_cbw_struct.CBWCB[4] << 8
					| usb_msc_cbw_struct.CBWCB[5];

		uint16_t num_blocks = usb_msc_cbw_struct.CBWCB[7] << 8
							| usb_msc_cbw_struct.CBWCB[8];

		uint32_t residual = length - (num_blocks * USB_MSC_BLOCK_LENGTH);
		header.dCSWDataResidue = residual;


		uint8_t* starting_addr = (uint8_t*)storage + (lba * USB_MSC_BLOCK_LENGTH);
		//usb_msc_read_cmd(starting_addr, num_blocks);
		if (prv_msc_read_cb != NULL)
		{
			prv_msc_read_cb(lba, num_blocks);
		}
	}
	else if (command == USB_MSC_SCSI_TEST_UNIT_READY)
	{
		header.dCSWDataResidue = 0;
		usb_write_fifo1(USB_DFIFO(1), &header, sizeof(usb_msc_csw_t));
	}
	else if (command == USB_MSC_SCSI_MEDIUM_REMOVAL)
	{
		header.dCSWDataResidue = 0;
		usb_write_fifo1(USB_DFIFO(1), &header, sizeof(usb_msc_csw_t));
	}
	else if (command == USB_MSC_SCSI_WRITE_10)
	{
		header.dCSWDataResidue = 0;
		wr_lba = usb_msc_cbw_struct.CBWCB[2] << 24
					| usb_msc_cbw_struct.CBWCB[3] << 16
					| usb_msc_cbw_struct.CBWCB[4] << 8
					| usb_msc_cbw_struct.CBWCB[5];
		wr_num_blocks = usb_msc_cbw_struct.CBWCB[7] << 8
							| usb_msc_cbw_struct.CBWCB[8];

		if (prv_msc_write_cb != NULL)
		{
			prv_msc_write_cb(wr_lba, wr_num_blocks);
		}

		assert( wr_transfer_length <= 0x10000 );

	}
	else if (command == USB_MSC_SCSI_REZERO_UNIT)
	{
		header.dCSWDataResidue = 0;
		usb_write_fifo1(USB_DFIFO(1), &header, sizeof(usb_msc_csw_t));
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
		volatile uint8_t* dest_addr = (uint8_t*)&usb_msc_cbw_struct;
		uint32_t offset = 8;
		while (offset--)
		{
			*(volatile uint32_t*)dest_addr = *(volatile uint32_t*)USB_DFIFO(0);
			dest_addr += 4;
		}
		usb_msc_handle_cbw();

	}
	/* Were expecting a WR to the drive. */
	else if(length != 0)
	{
		bool last_trnsf = false;
		/* Call the interrupt handler if it's been assigned. */
		if (prv_msc_write_complete_cb != NULL)
		{
			last_trnsf = prv_msc_write_complete_cb(length);
		}

		if (last_trnsf == true)
		{

			usb_write_fifo1(USB_DFIFO(1), &header, sizeof(usb_msc_csw_t));
		}
	}
	else
	{
		assert(0);
	}
}

void usb_msc_ep_in_handler(uint32_t ep, uint32_t ir)
{
	static struct more_data_ll* temp = &more_data_header;

	if (ep != 1)
	{
		return;
	}



	/* The last time we sent a message was the last one or there was never more than 1. */
	if (temp->next_node == NULL)
	{
		/* There was never more than 1. */
		if (temp == &more_data_header)
		{
			return;
		}

		/* Start freeing the whole linked list. */
		temp = more_data_header.next_node;
		while (temp->next_node != NULL)
		{
			struct more_data_ll* free_this_node = temp;
			temp = temp->next_node;
			free(free_this_node);
		}
		/* Temp is pointing to the last node. */
		more_data_header.next_node = temp;
		temp = &more_data_header;
		free(more_data_header.next_node);
		more_data_header.next_node = NULL;
		return;
	}

	temp = temp->next_node;

	usb_write_fifo1(USB_DFIFO(1), temp->data_ptr, temp->data_size);
}

void usb_msc_read_cmd(uint8_t* buf, uint32_t num_blocks)
{
	//Transfer Length in number of blocks minus 1 bc were sending one right now.
	uint16_t size = (USB_MSC_BLOCK_LENGTH * num_blocks) - 0x40;
	uint8_t* starting_addr = buf;
	uint8_t* next_addr = starting_addr + 0x40;
	/* Queue a block into the more data link list. */
	while (size > 0)
	{
		add_more_data(next_addr, 0x40);
		next_addr += 0x40;
		size -= 0x40;
	}
	add_more_data(&header, sizeof(usb_msc_csw_t));
	usb_write_fifo1(USB_DFIFO(1), starting_addr, 0x40);
}

void usb_msc_set_read_cb(void (*func)(uint32_t lba, uint32_t num_blocks))
{
	prv_msc_read_cb = func;
}

void usb_msc_set_write_cb(void (*func)(uint32_t lba, uint32_t num_blocks))
{
	prv_msc_write_cb = func;
}

void usb_msc_set_write_complete_cb(bool (*func)(uint32_t length_bytes))
{
	prv_msc_write_complete_cb = func;
}
