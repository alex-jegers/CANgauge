/**
 ******************************************************************************
 * @file           : main.c
 * @author         : A. Jegers
 * @brief          : Main program body
 ******************************************************************************
 *
 ******************************************************************************
 */
#ifdef CORE_CM7

#if defined(TARGET_HARDWARE_CANGAUGE) && defined(TARGET_HARDWARE_STM32H745DISCO)
	#error "Error: Both hardware targets defined, please only define one."
#endif

#if !defined(TARGET_HARDWARE_CANGAUGE) && !defined(TARGET_HARDWARE_STM32H745DISCO)
	#error "Error: No hardware target defined. Please define either TARGET_HARDWARE_CANGAUGE or TARGET_HARDWARE_STM32H745DISCO."
#endif

#include <stdint.h>
#include "stm32h745xx.h"
#include "system/cm7/system_cm7.h"


int main(void)
{
   	system_init();
 	system_run();
}

#endif	//CORE_CM7
