/*
 * app_gauges_cm7.c
 *
 *  Created on: Jun 6, 2024
 *      Author: awjpp
 */

#ifdef CORE_CM7

/**********     INCLUDES     **********/
#include "app_gauges_cm7.h"
#include "app_gauges_cm7_def.h"
#include "application/app_shared_mem.h"
#include "ui/ui_gauges.h"
#include "ui/ui_helpers.h"
#include "lvgl/lvgl.h"
#include "drivers/stm32_hsem.h"
#include "application/cm7/app_can_baud_rate_cm7.h"
#include "application/cm7/app_dtc_reader_cm7.h"
#include "system/cm7/system_cm7.h"

/**********     STATIC VARIABLES     **********/
static bool app_run = true;			//set to false to stop application execution.
static app_gauges_lut_t* active_gauge = NULL;

/**********     STATIC FUNCTION PROTOTYPES     **********/
static void gauge_clicked_cb(lv_event_t* e);
static void dtc_reader_btn_clicked_cb(lv_event_t* e);


/**********     STATIC FUNCTION DEFINITIONS     **********/
static void gauge_clicked_cb(lv_event_t* e)
{
	/*Figure out which gauge was clicked/pressed.*/
	lv_obj_t* clicked_gauge = lv_event_get_current_target(e);

	/*Using the gauge that was clicked, get the respecitve LUT containing the CAN information from app_gauges_cm7_def.*/
	app_gauges_lut_t* gauge_lut = app_gauges_get_lut_from_lv_obj(clicked_gauge);
	active_gauge = gauge_lut;

	/*Set the shared memory with the data CM4 needs to transmit/recieve from CAN.*/
	shared_mem_set_can_tx(gauge_lut->can_msg);												//Set the TX data.
	shared_mem_set_can_tx_interval(gauge_lut->tx_interval_ms);								//Set the TX interval.
	shared_mem_set_can_tx_data(gauge_lut->can_msg->data, gauge_lut->can_msg->T1.bit.DLC);	//
	shared_mem_set_can_ext_id_filter(gauge_lut->can_return_id);
	shared_mem_set_gauge_val_ptr(gauge_lut->gauge->set_value);

	/*Signal to CM4 that there's new data to transmit.*/
	hsem_lock(3,0);
	hsem_signal(3,0);
}

static void dtc_reader_btn_clicked_cb(lv_event_t* e)
{
	app_run = false;							//Tell CM7 to stop running this application.
	system_set_app_ptr(app_dtc_reader_run_cm7);	//Tell CM7 what application to start running.
	hsem_signal(5,0);							//Tell CM4 to stop running gauges.

}

/**********     GLOBAL FUNCTION DEFINITIONS     **********/
void app_gauges_run()
{
	ui_assign_gauge_clicked_cb(gauge_clicked_cb);
	ui_assign_dtc_reader_clicked_cb(dtc_reader_btn_clicked_cb);
	ui_load_gauge_screen();
	app_run = true;

	/*This HSEM tells CM4 to run gauges.*/
	hsem_lock(2,0);
	hsem_signal(2, 0);

	/*This is used to tell CM4 to stop running gauges when necessary.*/
	hsem_lock(5,0);

	while (app_run)
	{
		while (!hsem_wait_bool(4, 0))
		{
			if (app_run == false)
			{
				break;
			}
			lv_timer_handler();
		}
		int32_t active_gauge_value = shared_mem_get_converted_value();
		if (active_gauge != NULL)
		{
			active_gauge->gauge->set_value(active_gauge_value);
		}
		hsem_clear_int(4);
	}

	/*This tells CM4 to stop running gauges.*/
	hsem_signal(5,0);
}

#endif //CORE_CM7
