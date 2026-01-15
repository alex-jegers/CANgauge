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

int main(void)
{
	rcc_main_clock_config();
	fmc_init_sdram();
   	xTaskCreate(system_task_init, "SYS_INIT", 450, NULL, 0, NULL);
   	vTaskStartScheduler();
   	while(1);
}


