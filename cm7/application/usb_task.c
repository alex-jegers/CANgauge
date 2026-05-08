/**********     INCLUDES        **********/
#include "usb_task.h"
#include "drivers/drivers.h"
/**********     TYPEDEFS         **********/

/**********		DEFINES		**********/

/**********		EXTERNAL VARIABLE DEFINITIONS		**********/

/**********		STATIC VARIABLES		**********/

/**********		STATIC FUNCTION DECLRATIONS		**********/
TaskHandle_t usb_watchdog_handle = NULL;

/**********		STATIC FUNCTION DEFINITIONS		**********/

/**********		GLOBAL FUNCTION DEFINITIONS		**********/
void usb_watchdog_run()
{
	xTaskCreate(usb_watchdog_task, "USB_WD", 100, NULL, 4, &usb_watchdog_handle);
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
