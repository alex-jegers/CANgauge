/*
 * app_can_get_baud_rate_cm4.c
 *
 *  Created on: May 31, 2024
 *      Author: awjpp
 */

#include "app_can_get_baud_rate_cm4.h"

void app_can_baud_rate_run()
{
	while(1)
	{
		FDCAN_GlobalTypeDef* canbus = shared_get_target_can();

		/*Take the HSEM.*/
		hsem_lock(HSEM_CAN_BAUD_RATE, 0);

		/*Initialize the msg ram so the device is capable of receiving data.*/
		can_take(canbus);
		can_init(canbus);

		/*Get the baud rates and write to shared memory.*/
		shared_set_can_baud(canbus, can_get_baud_rate(canbus));

		app_can_baud_rate_stop();

		/*Release the HSEM to tell CM7 the baud rates are written to shared mem.*/
		hsem_signal(HSEM_CAN_BAUD_RATE, 0);

		vTaskSuspend(NULL);
	}
}

void app_can_baud_rate_stop()
{
	can_stop(HS_CAN);
	can_stop(LS_CAN);
	can_deinit(HS_CAN);
	can_deinit(LS_CAN);
}



