/**********     INCLUDES        **********/

#include "bootloader.h"
#include "ui/ui_gauges.h"
#include "file_system/fatfs/ff.h"
#include "lvgl_port/lvgl_port_def.h"
#include "lvgl/lvgl.h"
#include "application/applications_cm7.h"

/**********     TYPEDEFS         **********/

/**********		DEFINES		**********/
#define foreach(_x)		uint32_t _i = _x; for(_x = 0; _x < _i; _x++)
/**********		EXTERNAL VARIABLE DEFINITIONS		**********/

/**********		STATIC VARIABLES		**********/
static TaskHandle_t prv_btldr_task_handle = NULL;			//The handle to the bootloader task.
static lv_obj_t* msg_box;									//The message box that pops up when the user presses "Update Firmware"
static SemaphoreHandle_t prv_mutex_file_ready = NULL;		//Used to signal when the user presses the "done" button.
static void* firmware_buf = NULL;							//Pointer to the firmware data read from the .bin file.
static uint32_t file_size = 0;								//The size of the file that was uploaded to the gauge over USB. This is increased so it aligns with a flash sector.
/**********		STATIC FUNCTION DECLRATIONS		**********/
static void btldr_firmware_btn_cb(lv_event_t* e);
static void prv_file_ready(lv_event_t* e);
static void prv_msgbox_close();

/**
 * btldr_reprogram:
 * 		desc: This function writes the new program into flash.
 * 			It and any functions it calls need to run out of RAM
 * 			or somewhere that's not the flash being reprogrammed.
 */
SYS_MEM_REGION_RAM_EXE static void btldr_reprogram();
/**********		STATIC FUNCTION DEFINITIONS		**********/
static void btldr_firmware_btn_cb(lv_event_t* e)
{
	/* Stop the power monitor. */
	pwr_monitor_suspend();

	/* Connect to USB. */
	usb_connect(USB_FS_RAM);

	/* Load the message box. */
	msg_box = ui_helpers_show_msgbox("Waiting for file upload.", "Done", &prv_file_ready);
	ui_helpers_add_msgbox_close_btn(msg_box, prv_msgbox_close);

	/* Create the mutex. */
	prv_mutex_file_ready = xSemaphoreCreateMutex();
	assert( prv_mutex_file_ready != NULL );
	BaseType_t res = xSemaphoreTake(prv_mutex_file_ready, 0);
	assert( res );

	/* Start the task. */
	xTaskCreate(btldr_task, "BOOTLDR", 500, NULL, 4, &prv_btldr_task_handle);
}

static void prv_file_ready(lv_event_t* e)
{
	xSemaphoreGive(prv_mutex_file_ready);
}

SYS_MEM_REGION_RAM_EXE static void btldr_reprogram()
 {
	portENTER_CRITICAL();
	uint8_t n_sectors = file_size / 0x20000;
	for (uint8_t i = 0; i < n_sectors; i++)
	{
		GPIOB->ODR ^= GPIO_PIN15_Msk;	//Toggle the test LED. This is register level to avoid function calls to flash.
		sys_mem_flash_write_sector(i, firmware_buf + (i * 0x20000));
	}
	rcc_sw_reset();
}

static void prv_msgbox_close()
{
	vTaskDelete(prv_btldr_task_handle);
	vSemaphoreDelete(prv_mutex_file_ready);
	usb_disconnect();
	pwr_monitor_resume();
}

/**********		GLOBAL FUNCTION DEFINITIONS		**********/
void btldr_init()
{
	ui_add_settings_firmware_update_btn_event_cb(btldr_firmware_btn_cb);
}

TaskFunction_t btldr_task()
{
	while (1)
	{
		FIL file;
		FRESULT res = FR_DISK_ERR;

		/* Wait for the user to press "Done." */
		while (xSemaphoreTake(prv_mutex_file_ready, portMAX_DELAY) == pdFAIL) {}

		/* Try to open the file, do something to handle the error if it's not there. */
		res = f_open(&file, "1:/Firmware/cangauge.bin", FA_READ);
		if (res != FR_OK)
		{
			lv_port_take_lvgl_mutex(portMAX_DELAY);
			lv_obj_delete(msg_box);
			lv_obj_t* fail_msgbox = ui_helpers_show_msgbox("File not found. Canceling.", NULL, NULL);
			ui_helpers_add_msgbox_close_btn(fail_msgbox, prv_msgbox_close);
			lv_port_give_lvgl_mutex();
		}
		else
		{
			/* Determine the file size and how much we need to adjust it so it's an even number of flash sectors. */
			file_size = f_size(&file);
			uint32_t remainder = file_size % 0x20000;			//0x20000 is the flash mem sector size.
			uint32_t bytes_to_add = 0x20000 - remainder;	//Figure out how many bytes we need to add to make it a full sector.
			file_size += bytes_to_add;

			/* Allocate memory to hold the binary data. */
			firmware_buf = calloc(file_size, 1);

			/* Read the binary file into the buffer just created. */
			uint32_t bytes_read = 0;
			res = f_read(&file, firmware_buf, file_size, &bytes_read);
			assert ( res == FR_OK );

			btldr_reprogram();

			vTaskDelete(NULL);
		}
	}
}
