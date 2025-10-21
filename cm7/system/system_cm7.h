/*
 * system.h
 *
 *  Created on: May 31, 2024
 *      Author: awjpp
 */

#ifdef CORE_CM7

#ifndef CORE_SYSTEM_CM7_SYSTEM_H_
#define CORE_SYSTEM_CM7_SYSTEM_H_


/**********		INCLUDES		**********/
#include "stm32h745xx.h"
#include "FreeRTOS.h"
#include <task.h>
#include "semphr.h"

/**********		DEFINES		**********/
#define SYS_LVGL_TICK_INC_TASK_CODE				"LV_TICK_INC"
#define SYS_LVGL_TASK_HANDLER_TASK_CODE			"LV_TASK_HANDLER"

/**********     GLOBAL VARIABLE DECLRATIONS     **********/
extern SemaphoreHandle_t sys_mutex_lvgl;

/**********		GLOBAL FUNCTION DECLRATIONS		**********/
void system_task_init();
void system_task_lvgl_tick_inc();		//Task to increment the LVGL timer.
void system_task_lvgl_timer_update();	//Task to run LVGL renderer.
void system_task_blink();				//Task to blink the test LED at 2 HZ.



#endif /* CORE_SYSTEM_CM7_SYSTEM_H_ */

#endif	//CORE_CM7
