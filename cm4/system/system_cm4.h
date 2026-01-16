/*
 * system_cm4.h
 *
 *  Created on: May 31, 2024
 *      Author: awjpp
 */


#ifndef CORE_SYSTEM_CM4_SYSTEM_CM4_H_
#define CORE_SYSTEM_CM4_SYSTEM_CM4_H_

#define ENABLE_CM4		true

#include "stm32h745xx.h"
#include "common/cangauge_common.h"

#include "application/applications_cm4.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "list.h"
#include "semphr.h"

/**********     GLOBAL VARIABLE DECLRATIONS     **********/
extern TaskHandle_t sys_task_handle_app_get_baud_rate;

void system_task_init();		//Basic system initialization.
void system_task_monitor();		//Check and see if CM7 is requesting anything to be run or stopped.
void system_hsem_handler();


#endif /* CORE_SYSTEM_CM4_SYSTEM_CM4_H_ */

