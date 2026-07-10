/**********     INCLUDES        **********/
#include "usb_task.h"
#include "drivers/drivers.h"
#include "drivers/usb/stm32_usb_msc.h"

/**********     TYPEDEFS         **********/

/**********		DEFINES		**********/

/**********		EXTERNAL VARIABLE DEFINITIONS		**********/

/**********		STATIC VARIABLES		**********/
static TaskHandle_t prv_usb_watchdog_handle = NULL;
static TaskHandle_t prv_usb_msc_handle = NULL;

/* I use one task notification between the 3 interrupt callbacks (read,
 * write, and write complete), so these three bools are used to determine
 * which one is the gave the notification.
 */
static bool prv_write_complete_int = false;
static bool prv_write_int = false;
static bool prv_read_int = false;

static uint32_t prv_block_length = 512;
static uint32_t prv_num_blocks = 0;
static usb_fs_t prv_file_sys = 0xFF;

static uint8_t* file_sys_start_ptr = NULL;
static uint8_t* rd_starting_addr = NULL;
static uint32_t rd_size_blocks = 0;

/* Stuff for when the PC is writing to the device. */
static uint32_t* wr_start_addr = 0;
static uint32_t wr_full_length = 0;
static uint32_t wr_partial_length = 0;

/**********		STATIC FUNCTION DECLRATIONS		**********/
static void prv_msc_read_handler(uint32_t lba, uint32_t num_blocks);		//Called in an ISR.
static void prv_msc_write_handler(uint32_t lba, uint32_t num_blocks);		//Called in an ISR.
/**
 * prv_msc_write_complete_handler
 * 		returns: true if this was the last transfer of the entire write.
 */
static bool prv_msc_partial_wr_handler(uint32_t length_bytes);	//Called in an ISR.

/**********		STATIC FUNCTION DEFINITIONS		**********/
static void prv_msc_read_handler(uint32_t lba, uint32_t num_blocks)
{
	prv_read_int = true;
	BaseType_t higher_pri_task_woken = pdFALSE;

	/* This is to read from RAM. */
	rd_starting_addr = file_sys_start_ptr + (lba * prv_block_length);
	rd_size_blocks = num_blocks;
	usb_clear_gintmsk();						//Disable USB interrupts, have to re-enable after we read in this data.
	usb_clear_ep1_intmsk();
	/*****************************/

	vTaskNotifyGiveFromISR(prv_usb_msc_handle, &higher_pri_task_woken);
	portYIELD_FROM_ISR(higher_pri_task_woken);
}
static void prv_msc_write_handler(uint32_t lba, uint32_t num_blocks)
{
	prv_write_int = true;
	BaseType_t higher_pri_task_woken = pdFALSE;

	/* This is to write to RAM. */
	wr_start_addr = (uint32_t*)((uint8_t*)file_sys_start_ptr + (lba * prv_block_length));
	wr_full_length = num_blocks * prv_block_length;
	/***************************/

	vTaskNotifyGiveFromISR(prv_usb_msc_handle, &higher_pri_task_woken);
	portYIELD_FROM_ISR(higher_pri_task_woken);
}
static bool prv_msc_partial_wr_handler(uint32_t length_bytes)
{
	prv_write_complete_int = true;
	BaseType_t higher_pri_task_woken = pdFALSE;

	/* This is to write to RAM. */
	wr_partial_length = length_bytes;
	usb_clear_gintmsk();						//Disable USB interrupts, have to re-enable after we read in this data.
	/************************/

	vTaskNotifyGiveFromISR(prv_usb_msc_handle, &higher_pri_task_woken);
	portYIELD_FROM_ISR(higher_pri_task_woken);

	if (wr_full_length - wr_partial_length == 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

/**********		GLOBAL FUNCTION DEFINITIONS		**********/
void usb_disconnect()
{
	if (prv_usb_watchdog_handle != NULL)
	{
		vTaskDelete(prv_usb_watchdog_handle);
		prv_usb_watchdog_handle = NULL;
	}
	if (prv_usb_msc_handle != NULL)
	{
		vTaskDelete(prv_usb_msc_handle);
		prv_usb_msc_handle = NULL;
	}
	rcc_reset_usb2otg();
}

void usb_connect(usb_fs_t file_sys)
{
	usb_disconnect();		//Make sure were not currently connected with another file system.

	prv_file_sys = file_sys;

	if (file_sys == USB_FS_RAM)
	{
		prv_num_blocks = NUM_SECTORS_RAM;
		file_sys_start_ptr = sys_mem_get_ram_fs_ptr();	//Pointer to the start of the file system memory.
	}
	else if (file_sys == USB_FS_EEPROM)
	{
		prv_num_blocks = NUM_SECTORS_EEPROM;

		/* Pointer to the start of the file system memory.
		 * For EEPROM this is the physical address of the EEPROM
		 * which is just 0x00000.
		 */
		file_sys_start_ptr = 0;
	}
	/* Sets the interrupt handlers. */
	usb_msc_set_read_cb(prv_msc_read_handler);
	usb_msc_set_write_cb(prv_msc_write_handler);
	usb_msc_set_write_complete_cb(prv_msc_partial_wr_handler);

	/* Initialize the USB peripheral. */
	rcc_clr_reset_usb2otg();
	usb_init();
	usb_core_reset();
	usb_init_core();

	/* Start the tasks. */
	usb_watchdog_run();
	usb_msc_task_run();
}

void usb_watchdog_run()
{
	xTaskCreate(usb_watchdog_task, "USB_WD", 100, NULL, 4, &prv_usb_watchdog_handle);
}
void usb_watchdog_task()
{
	TickType_t last_run_time;
	last_run_time = xTaskGetTickCount();

	static bool usb_connected = false;
	static bool usb_last_state = false;
	static uint16_t last_frame_number = 0;


	while (1)
	{
		uint16_t frame_number = usb_get_frame_number();

		if (frame_number != last_frame_number)
		{
			usb_connected = true;
		}
		else
		{
			usb_connected = false;
		}

		/* Check for an cable unplugged condition. */
		if (usb_last_state == true && usb_connected == false)
		{
			rcc_reset_usb2otg();
			rcc_clr_reset_usb2otg();
			usb_init();
			usb_core_reset();
			usb_init_core();
			frame_number = 0;		//It'll be zero the next time it calls usb_get_frame number so make it zero now so last_frame_number is also zero.

		}

		usb_last_state = usb_connected;
		last_frame_number = frame_number;
		vTaskDelayUntil(&last_run_time, pdMS_TO_TICKS(200));
	}
}

void usb_msc_task_run()
{
	xTaskCreate(usb_msc_task, "USB_MSC", 450, NULL, 4, &prv_usb_msc_handle);
}

void usb_msc_task()
{
	while (1)
	{
		uint32_t count = ulTaskGenericNotifyTake(0, pdTRUE, portMAX_DELAY);
		if (count > 0)
		{
			if (prv_write_complete_int)
			{
				wr_full_length -= wr_partial_length;
				/* If RAM file system. */
				if (prv_file_sys == USB_FS_RAM)
				{
					while (wr_partial_length > 0)
					{
						*wr_start_addr = *USB_DFIFO(1);
						wr_start_addr++;
						wr_partial_length -= 4;
					}
				}
				else if (prv_file_sys == USB_FS_EEPROM)
				{
					/* Read the FIFO data into a temporary buffer. */
					uint32_t temp_buf[16];
					uint8_t counter = 0;
					uint32_t og_length = wr_partial_length;
					while (wr_partial_length > 0)
					{
						temp_buf[counter] = *USB_DFIFO(1);
						counter++;
						wr_partial_length -= 4;
					}

					/* Copy it to EEPROM and increment the wr_start_addr for next time through. */
					eeprom_write((uint32_t)wr_start_addr, temp_buf, og_length);
					wr_start_addr += 16;		//wr_start_address is a uint32_t* so +=16 is actually +=64.
				}
				prv_write_complete_int = false;
				count--;
			}

			if (prv_write_int)
			{
				count--;
				prv_write_int = false;
			}

			if (prv_read_int)
			{
				count--;

				if (prv_file_sys == USB_FS_RAM)
				{
					usb_msc_read_cmd(rd_starting_addr, rd_size_blocks);
				}
				else if (prv_file_sys == USB_FS_EEPROM)
				{
					uint32_t num_bytes = rd_size_blocks * prv_block_length;
					uint8_t* temp_buf = (uint8_t*)calloc(num_bytes, 1);
					eeprom_read(temp_buf, (uint32_t)rd_starting_addr, num_bytes);
					usb_msc_read_cmd(temp_buf, rd_size_blocks);
					free(temp_buf);
				}
				usb_set_ep1_intmsk();
				prv_read_int = false;
			}
			usb_set_gintmsk();
		}
		else //count <=0 (should never happen)
		{
			assert(0);
		}
	}
}

uint32_t usb_msc_get_block_size()
{
	return prv_block_length;
}

uint32_t usb_msc_get_num_blocks()
{
	return prv_num_blocks;
}
