/*
 * app_can_get_baud_rate_cm4.h
 *
 *  Created on: May 31, 2024
 *      Author: awjpp
 */


#ifndef CORE_APPLICATION_CM4_APP_CAN_GET_BAUD_RATE_CM4_H_
#define CORE_APPLICATION_CM4_APP_CAN_GET_BAUD_RATE_CM4_H_

#include <cangauge_common.h>
#include "drivers/stm32_canbus.h"
#include "drivers/stm32_hsem.h"

#include "FreeRTOS.h"
#include "task.h"

void app_can_baud_rate_run();			//gets the speed of both CANbus and writes it to shared memory.
void app_can_baud_rate_stop();			//deinits can peripheral and what not.


#endif /* CORE_APPLICATION_CM4_APP_CAN_GET_BAUD_RATE_CM4_H_ */

