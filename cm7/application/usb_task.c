/**********     INCLUDES        **********/
#include "usb_task.h"
#include "drivers/drivers.h"
#include "drivers/usb/stm32_usb_msc.h"

#include "file_system/fatfs/ff.h"
#include "file_system/fatfs/diskio.h"
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

static uint8_t* ram_fs_ptr = NULL;
static uint8_t* rd_starting_addr = NULL;
static uint32_t rd_size_bytes = 0;

/**********		STATIC FUNCTION DECLRATIONS		**********/
static void prv_msc_read_handler(uint32_t lba, uint32_t num_blocks);		//Called in an ISR.
static void prv_msc_write_handler(uint32_t lba, uint32_t num_blocks);		//Called in an ISR.
static void prv_msc_write_complete_handler(uint8_t* buf, uint32_t bytes);	//Called in an ISR.

/**********		STATIC FUNCTION DEFINITIONS		**********/
static void prv_msc_read_handler(uint32_t lba, uint32_t num_blocks)
{
	prv_read_int = true;

	BaseType_t higher_pri_task_woken = pdFALSE;

	rd_starting_addr = ram_fs_ptr + (lba * 512);
	rd_size_bytes = num_blocks * 512;

	vTaskNotifyGiveFromISR(prv_usb_msc_handle, &higher_pri_task_woken);

	portYIELD_FROM_ISR(higher_pri_task_woken);
}
static void prv_msc_write_handler(uint32_t lba, uint32_t num_blocks)
{
	prv_write_int = true;

	BaseType_t higher_pri_task_woken = pdFALSE;



	vTaskNotifyGiveFromISR(prv_usb_msc_handle, &higher_pri_task_woken);

	portYIELD_FROM_ISR(higher_pri_task_woken);
}
static void prv_msc_write_complete_handler(uint8_t* buf, uint32_t bytes)
{
	prv_write_complete_int = true;

	BaseType_t higher_pri_task_woken = pdFALSE;



	vTaskNotifyGiveFromISR(prv_usb_msc_handle, &higher_pri_task_woken);

	portYIELD_FROM_ISR(higher_pri_task_woken);
}

/**********		GLOBAL FUNCTION DEFINITIONS		**********/
void usb_watchdog_run()
{
	xTaskCreate(usb_watchdog_task, "USB_WD", 100, NULL, 4, &prv_usb_watchdog_handle);
}
void usb_watchdog_task()
{
	/**** TESTING USB CONFIGURATION *****/
	usb_init();
	usb_core_reset();
	usb_init_core();
	/***********************************/

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
	xTaskCreate(usb_msc_task, "USB_MSC", 250, NULL, 4, &prv_usb_msc_handle);
}

void usb_msc_task()
{
	/* Sets the interrupt handlers. */
	usb_msc_set_read_cb(prv_msc_read_handler);
	usb_msc_set_write_cb(prv_msc_write_handler);
	usb_msc_set_write_complete_cb(prv_msc_write_complete_handler);

	/* Create the file system. */
	static FATFS fs;           // Filesystem object
	FIL fil;            // File object
	FRESULT res;        // API result code
	UINT bw;            // Bytes written
	ram_fs_ptr = sys_mem_get_ram_fs_ptr();	//Pointer to the start of the file system memory.

	uint8_t* work = calloc(4096, 1);
	res = f_mkfs("1:", NULL, work, 4096);
	if (res != FR_OK)
	{
		assert(0);
	}

	// Give a work area to the default drive
	res = f_mount(&fs, "1:", 0);
	if (res != FR_OK)
	{
		assert(0);
	}

	// Create a file as new
	res = f_open(&fil, "1:hello.txt", FA_CREATE_NEW | FA_WRITE);
	if (res != FR_OK)
	{
		assert(0);
	}

	// Write a message
	f_write(&fil, "Hello, World!\r\n", 15, &bw);
	if (bw != 15)
	{
		assert(0);
	}
	
	// Close the file
	res = f_close(&fil);
	// Unregister work area
	//res = f_unmount("0");


	while (1)
	{
		uint32_t count = ulTaskGenericNotifyTake(0, pdTRUE, portMAX_DELAY);
		if (count > 0)
		{
			if (prv_write_complete_int)
			{

			}
			if (prv_write_int)
			{

			}
			if (prv_read_int)
			{

			}
		}
		else //count <=0 (should never happen)
		{
			assert(0);
		}
	}
}

uint32_t usb_msc_get_block_size()
{
	return 512;
}

uint32_t usb_msc_get_num_blocks()
{
	return 0x4000;
}
