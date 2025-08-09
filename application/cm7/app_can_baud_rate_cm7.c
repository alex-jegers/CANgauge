/*
 * app_can_baud_rate.c
 *
 *  Created on: May 31, 2024
 *      Author: awjpp
 */
#ifdef CORE_CM7

#include "app_can_baud_rate_cm7.h"
#include "ui/ui_loading.h"
#include "drivers/stm32_sys_timer.h"
#include "application/app_shared_mem.h"
#include "lvgl/lvgl.h"
#include "ui/ui_loading.h"
#include "ui/ui_helpers.h"
#include "drivers/stm32_hsem.h"
#include "system/cm7/system_cm7.h"
#include "application/cm7/app_gauges_cm7.h"

/**********     STATIC VARIABLES     **********/
static bool retry_button_clicked = false;
static bool demo_button_clicked = false;

/**********     STATIC FUNCTION DECLRATIONS     **********/
static void error_screen_retry_button_cb(lv_event_t* e);
static void error_screen_demo_button_cb(lv_event_t* e);

/**********     STATIC FUNCTION DEFINITIONS     **********/
static void error_screen_retry_button_cb(lv_event_t* e)
{
	retry_button_clicked = true;
}

static void error_screen_demo_button_cb(lv_event_t* e)
{
	demo_button_clicked = true;
}


/**********     GLOBAL FUNCTION DEFINITIONS     **********/
void app_can_baud_rate_run()
{
	/*Load the screen, it's already initialized in ui_helpers.c.*/
	ui_load_loading_screen();

	/*Assign callbacks for the buttons in the loading error screen.*/
	ui_assign_demo_btn_cb(error_screen_demo_button_cb);
	ui_assign_retry_btn_cb(error_screen_retry_button_cb);

	/*Take the semaphore and send the signal.
	 * This signal causes app_can_baud_rate to run on CM4. */
	hsem_lock(0, 0);
	hsem_signal(0, 0);

	/*Wait for CM4 to signal that it's done.
	 * Render LVGL while waiting. */
	while (hsem_wait_bool(0,0) == false)
	{
		lv_timer_handler();
	}

	/*Check HS CAN speed.*/
	if (app_shared_mem->hs_can_baud == CAN_BAUD_ERROR)
	{
		ui_load_loading_error_screen();				//if there's an error load the error screen.
	}
	else
	{
		ui_demo_mode = false;
		system_set_app_ptr(app_gauges_run);			//if there was no error request to run gauges.
		return;
	}

	/*Reset the button clicked variables to false.*/
	retry_button_clicked = false;
	demo_button_clicked = false;

	/*Wait for one of the buttons to be clicked.*/
	while (retry_button_clicked == false && demo_button_clicked == false)
	{
		/*Continue to render LVGL while waiting.*/
		lv_timer_handler();

		if (retry_button_clicked)
		{
			system_set_app_ptr(app_can_baud_rate_run);	//set the app pointer to re-run this application.
		}

		if (demo_button_clicked)
		{
			ui_demo_mode = true;
			system_set_app_ptr(app_gauges_run);			//if there was no error request to run gauges.
			return;
		}
	}

}

void app_can_baud_rate_stop()
{
	ui_delete_loading_screen();
	ui_delete_loading_error_screen();
}

#endif	//CORE_CM7
