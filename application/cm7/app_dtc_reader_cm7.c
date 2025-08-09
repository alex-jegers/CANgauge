/*
 * app_dtc_reader_cm7.c
 *
 *  Created on: Mar 7, 2025
 *      Author: awjpp
 */

#ifdef CORE_CM7

#include "app_dtc_reader_cm7.h"
#include "app_gauges_cm7.h"
#include "system/cm7/system_cm7.h"
#include "drivers/stm32_hsem.h"
#include "ui/ui_dtc_reader.h"
#include "stdbool.h"

/**********		STATIC VARIABLES		**********/
static bool app_run = false;

/**********		STATIC FUNCTION PROTOTYPES		**********/
static void app_dtc_reader_back_btn_clicked_handler();
static void app_dtc_reader_read_btn_clicked_handler();
static void app_dtc_reader_clear_btn_clicked_handler();

/**********		STATIC FUNCTION DEFINITIONS		**********/
static void app_dtc_reader_back_btn_clicked_handler()
{
	app_run = false;						//Tell CM7 to stop running this app.
	system_set_app_ptr(app_gauges_run);		//Point CM7 to what app to run next.
	hsem_signal(5, 0);						//Tell CM4 to stop running this app.
}

static void app_dtc_reader_read_btn_clicked_handler()
{

}

static void app_dtc_reader_clear_btn_clicked_handler()
{

}


void app_dtc_reader_run_cm7()
{
	app_run = true;
	ui_init_dtc_reader();
	ui_load_dtc_reader();
	ui_dtc_reader_assign_back_btn_clicked_handler(app_dtc_reader_back_btn_clicked_handler);

	/*Take this HSEM, used to tell CM4 to stop running later.*/
	hsem_lock(5,0);

	while (app_run)
	{
		lv_timer_handler();
	}
}


#endif //CORE_CM7
