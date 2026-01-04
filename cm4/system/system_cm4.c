/*
 * system_cm4.c
 *
 *  Created on: May 31, 2024
 *      Author: awjpp
 */
#ifdef CORE_CM4

#include "system/system_cm4.h"

#include "application/app_can_get_baud_rate_cm4.h"
#include <application/app_can_controller_cm4.h>

#include "drivers/stm32_hsem.h"
#include "drivers/stm32_hsem.h"
#include "drivers/stm32_rcc.h"
#include "drivers/stm32_io.h"

#include "touch_screen/cst830_touch_cm4.h"

#include "FreeRTOS.h"
#include <task.h>
#include <queue.h>
#include <list.h>
#include <semphr.h>


/**********     TYPEDEFS     **********/

/**********     DEFINES     **********/
#define SCB_CPACR_CP10_FULL_ACCESS			0x3 << 20
#define SCB_CPACR_CP11_FULL_ACCESS			0x3 << 22

#define HSEM_STARTUP_Msk						1


/**********		EXTERNAL VARIABLE DEFINITIONS		**********/
TaskHandle_t sys_task_handle_app_get_baud_rate = NULL;
TaskHandle_t sys_task_hdl_app_can_sniffer = NULL;

/**********     STATIC VARIABLES     **********/


/**********     STATIC FUNCTION DECLARATIONS     **********/


/**********     STATIC FUNCTION DEFINITIONS     **********/


/**********     GLOBAL FUNCTION DEFINITIONS     **********/
void system_task_init()
{
    /*Enable FPU (I do this in CM7, not sure if it's needed here as well.*/
	SCB->CPACR = SCB_CPACR_CP10_FULL_ACCESS | SCB_CPACR_CP11_FULL_ACCESS;		//enables the FPU.



	/*Wait for a signal from CM7.*/
	hsem_wait_void(HSEM_INIT, HSEM_ID_INIT_CM7);
	hsem_lock(HSEM_INIT, HSEM_ID_INIT_CM4);

	can_init(FDCAN1);
	can_init(FDCAN2);

	/*Enable HSEM interrupts.*/
	hsem2_assign_int_handler(system_hsem_handler);
	//NVIC_EnableIRQ(HSEM2_IRQn);

	/*Signal that initialization is done.*/
	hsem_signal(HSEM_INIT, HSEM_ID_INIT_CM4);

	while (!ENABLE_CM4){}
}

void system_task_monitor()
{
	while(1)
	{
		uint32_t ir = hsem_get_ir();
		if ((ir & (1 << HSEM_CAN_BAUD_RATE)) != 0)
		{
			hsem_clear_int(HSEM_CAN_BAUD_RATE);
			vTaskResume(sys_task_handle_app_get_baud_rate);
		}
		if(hsem_get_status(HSEM_APP_CAN_CONTROLLER_START))
		{
			hsem_clear_int(HSEM_APP_CAN_CONTROLLER_START);
			app_can_controller_run();
		}
		if (hsem_get_status(HSEM_APP_CAN_CONTROLLER_STOP))
		{
			hsem_clear_int(HSEM_APP_CAN_CONTROLLER_STOP);
			app_can_sniffer_stop();
		}

		vTaskDelay(100);
	}
}

void system_hsem_handler()
{


}


#endif	//CORE_CM4
