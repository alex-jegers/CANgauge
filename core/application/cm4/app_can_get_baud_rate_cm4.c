/*
 * app_can_get_baud_rate_cm4.c
 *
 *  Created on: May 31, 2024
 *      Author: awjpp
 */
#ifdef CORE_CM4

#include "app_can_get_baud_rate_cm4.h"
#include "drivers/stm32_hsem.h"
#include "drivers/stm32_canbus.h"

void app_can_baud_rate_run()
{
	/*Take the HSEM.*/
	hsem_lock(0, 0);

	/*Initialize the msg ram so the device is capable of receiving data.*/
	can_init_clk();
	can_init(FDCAN1);

	/*Get the baud rates and write to shared memory.*/
	app_shared_mem->hs_can_baud = can_get_baud_rate(HS_CAN);
	app_shared_mem->ls_can_baud = can_get_baud_rate(LS_CAN);


	/*Release the HSEM to tell CM7 the baud rates are written to shared mem.*/
	hsem_signal(0, 0);

	app_can_baud_rate_stop();
}

void app_can_baud_rate_stop()
{
	can_deinit();
}


#endif	//CORE_CM4
