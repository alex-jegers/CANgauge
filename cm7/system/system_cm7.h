/*
 * system.h
 *
 *  Created on: May 31, 2024
 *      Author: awjpp
 */



#ifndef CORE_SYSTEM_CM7_SYSTEM_H_
#define CORE_SYSTEM_CM7_SYSTEM_H_


/**********		INCLUDES		**********/
#include "stm32h745xx.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "list.h"
#include "semphr.h"

/**********		DEFINES		**********/
#define SYS_LVGL_TICK_INC_TASK_CODE				"LV_TICK_INC"
#define SYS_LVGL_TASK_HANDLER_TASK_CODE			"LV_TASK_HANDLER"

/**********     GLOBAL VARIABLE DECLRATIONS     **********/

/**********		GLOBAL FUNCTION DECLRATIONS		**********/
void system_task_init();
void system_task_blink(const uint32_t delay_time_ms);	//Task to blink the test LED.



#endif /* CORE_SYSTEM_CM7_SYSTEM_H_ */


