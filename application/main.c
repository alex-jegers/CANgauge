/**
 ******************************************************************************
 * @file           : main.c
 * @author         : A. Jegers
 * @brief          : Main program body
 ******************************************************************************
 *
 ******************************************************************************
 */
#include "cangauge.h"
#include "application/applications_cm7.h"

int main(void)
{
	/* Configures the clocks to run at full speed in full power mode. */
 	rcc_main_clock_config();

	/* Initializes the external SDRAM. */
	fmc_init_sdram();

	/* Enable the hardware floating point unit (ensure this is enabled in project settings too). */
	system_init_fpu();

	system_init();

	/* Creates a task to finish the rest of the system initialization. */
   	xTaskCreate(system_task_init, "SYS_INIT", 650, NULL, 5, NULL);

	/* Load the menu screen. */
	system_set_ui_init_cb(app_gauges_run);

	/* Starts the FreeRTOS scheduler. */
	vTaskStartScheduler();

	/* Should never reach here. */
   	while(1);
}


