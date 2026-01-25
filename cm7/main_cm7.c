/**
 ******************************************************************************
 * @file           : main.c
 * @author         : A. Jegers
 * @brief          : Main program body
 ******************************************************************************
 *
 ******************************************************************************
 */

#if defined(TARGET_HARDWARE_CANGAUGE) && defined(TARGET_HARDWARE_STM32H745DISCO)
	#error "Error: Both hardware targets defined, please only define one."
#endif

#if !defined(TARGET_HARDWARE_CANGAUGE) && !defined(TARGET_HARDWARE_STM32H745DISCO)
	#error "Error: No hardware target defined. Please define either TARGET_HARDWARE_CANGAUGE or TARGET_HARDWARE_STM32H745DISCO."
#endif

#include "system/system_cm7.h"
#include "drivers/drivers.h"
#include "lvgl_port/lvgl_port_def.h"
#include "application/applications_cm7.h"

int main(void)
{
	/* Configures the clocks to run at full speed in full power mode. */
	rcc_main_clock_config();

	/* Initializes the external SDRAM. */
	fmc_init_sdram();

	/* Creates a task to finish the rest of the system initialization. */
   	xTaskCreate(system_task_init, "SYS_INIT", 450, NULL, 4, NULL);

		   	/* Set up the display and input device callbacks for LVGL. */
   	static touch_info_t touch_data;						//Where the touch data will be stored.
   	static touch_info_t* p_touch_data = &touch_data;
	lv_port_run();										//
	disp_init();										//LVGL display bindings
	indev_init(&p_touch_data);							//LVGL input device callback (touch screen).
	touch_scr_run(p_touch_data);						//Runs the touch screen task.
		
	/* Load the menu screen. */
	app_menu_run();

	/* Starts the FreeRTOS scheduler. */
	vTaskStartScheduler();

	/* Should never reach here. */
   	while(1);
}


