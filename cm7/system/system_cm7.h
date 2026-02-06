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
#include <stdbool.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "list.h"
#include "semphr.h"

#include "system_mem.h"

/**********		DEFINES		**********/

/**********     GLOBAL VARIABLE DECLRATIONS     **********/

/**********		GLOBAL FUNCTION DECLRATIONS		**********/
void system_task_init();
void system_init_fpu();
void system_blink_run(const uint32_t delay_time_ms);
void system_blink_set_delay(uint32_t on_ms, uint32_t off_ms);
void system_blink_stop();


#endif /* CORE_SYSTEM_CM7_SYSTEM_H_ */


