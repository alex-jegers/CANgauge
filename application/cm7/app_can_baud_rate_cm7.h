/*
 * app_can_baud_rate.h
 *
 *  Created on: May 31, 2024
 *      Author: awjpp
 */

#ifdef CORE_CM7

#ifndef CORE_APPLICATION_CM7_APP_CAN_BAUD_RATE_H_
#define CORE_APPLICATION_CM7_APP_CAN_BAUD_RATE_H_

#include "stm32h745xx.h"
#include "drivers/stm32_canbus.h"

typedef enum
{
	APP_CAN_BAUD_RATE_SUCCESS,
	APP_CAN_BAUD_RATE_DEMO,
	APP_CAN_BAUD_RATE_RETRY,
	APP_CAN_BAUD_RATE_ERROR,
}app_can_baud_rate_return_t;

extern FDCAN_GlobalTypeDef* hs_can;
extern FDCAN_GlobalTypeDef* ls_can;

extern can_baud_rate_t hs_can_baud;
extern can_baud_rate_t ls_can_baud;

void app_can_baud_rate_run();
void app_can_baud_rate_stop();


#endif /* CORE_APPLICATION_CM7_APP_CAN_BAUD_RATE_H_ */


#endif //CORE_CM7
