/**
 ******************************************************************************
 * @file           : main_cm4.c
 * @author         : Alex Jegers
 * @brief          : CANgauge CM4 entry point
 ******************************************************************************
 */



#include <stdint.h>
#include <stdbool.h>

#include "stm32h745xx.h"

#include "system/system_cm4.h"


CG_MEMORY_REGION_EXT touch_info_t touch_info_storage_area;


int main(void)
{
	system_task_init();

	/*
	 * Updates the touch screen data. Needs to be high priority, runs quick and the latency of
	 * the touch screen relies on it.
	 * */
	common.p_touch_data = &touch_info_storage_area;
	xTaskCreate((TaskFunction_t)cst830_task_update, "CST830_UPDATE", 1024, &touch_info_storage_area, 4, NULL);

	/*Gets CAN baud rates, triggered by HSEM 0.*/
	xTaskCreate(app_can_baud_rate_run, "APP_GET_BAUD_RATE", 1000, NULL, 2, &sys_task_handle_app_get_baud_rate);
	vTaskSuspend(sys_task_handle_app_get_baud_rate);

	/*Monitors the HSEMs for a signal from CM7 to run something.*/
	xTaskCreate(system_task_monitor, "SYS_MONITOR", 500, NULL, 3, NULL);

	vTaskStartScheduler();

}
