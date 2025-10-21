/*
 * app_can_get_baud_rate_cm4.c
 *
 *  Created on: May 31, 2024
 *      Author: awjpp
 */
#ifdef CORE_CM4

#include "app_can_get_baud_rate_cm4.h"

void app_can_baud_rate_run()
{
	while(1)
	{
		/*Take the HSEM.*/
		hsem_lock(0, 0);

		/*Initialize the msg ram so the device is capable of receiving data.*/
		can_take(HS_CAN);
		can_take(LS_CAN);
		can_init(HS_CAN);
		can_init(LS_CAN);

		/*Get the baud rates and write to shared memory.*/
		shared_set_can_baud(HS_CAN, can_get_baud_rate(HS_CAN));
		shared_set_can_baud(LS_CAN, can_get_baud_rate(LS_CAN));

		app_can_baud_rate_stop();

		/*Release the HSEM to tell CM7 the baud rates are written to shared mem.*/
		hsem_signal(0, 0);

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


#endif	//CORE_CM4
