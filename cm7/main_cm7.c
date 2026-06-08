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

	/* Starts the FreeRTOS scheduler. */
	vTaskStartScheduler();

	/* Should never reach here. */
   	while(1);
}


