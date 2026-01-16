/**********     INCLUDES        **********/
#include <cangauge_common.h>
#include "app_gauges_cm7.h"
#include "system/system_cm7.h"

#include "ui/ui_gauges.h"

#include "drivers/stm32_canbus.h"
#include "drivers/stm32_hsem.h"

#include "lvgl.h"

#include "common/saej1979.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/**********		DEFINES		**********/

/**********		EXTERNAL VARIABLE DEFINITIONS		**********/

/**********		STATIC VARIABLES		**********/
bool _run = false;
static TaskHandle_t _gauges_task_handle;

/* The PIDs that are to be plugged into data byte 2 of the template above. */
uint8_t saej1979_pid_coolant_temp = 0x05;
uint8_t saej1979_pid_fuel_pressure = 0x0A;
uint8_t saej1979_pid_intake_air_pressure = 0x0B;
uint8_t saej1979_pid_timing_advance = 0x0E;
uint8_t saej1979_pid_intake_air_temp = 0x0F;
uint8_t saej1979_pid_maf_flow_rate = 0x10;
uint8_t saej1979_pid_fuel_rail_pressure = 0x22;
uint8_t saej1979_pid_air_fuel_ratio = 0x34;
static uint8_t _currently_displayed_pid = 0;


/**********		STATIC FUNCTION DECLRATIONS		**********/
static void _task_gauges();							//The FreeRTOS task.
static void _gauge_btn_event_cb(lv_event_t* e);		//Handler for the gauge selection buttons events.
static void _gauge_event_cb(lv_event_t* e);			//Handler for the gauge itself events.
static void _gauge_scr_load_cb(lv_event_t* e);		//Handler for the gauge screen loading.
static void _gauge_select_btn_cb(lv_event_t* e);	//Handler for a gauge being selected.

/**********		STATIC FUNCTION DEFINITIONS		**********/
static void _task_gauges()
{
	_run = true;

	/* Set the LVGL event callbacks. */
	ui_gauges_set_gauge_select_btn_cb(_gauge_btn_event_cb);
	ui_gauges_set_gauge_cb(_gauge_event_cb);
	ui_gauges_set_scr_load_cb(_gauge_scr_load_cb);
	ui_gauges_set_gauge_select_btn_cb(_gauge_select_btn_cb);

	/* Manually set the baud rate. */
	//shared_set_can_baud_override(FDCAN1, CAN_BAUD_500K);

	/* Start the CAN controller on CM4. */
	//hsem_lock(HSEM_APP_CAN_CONTROLLER_START, HSEM_ID_APP_CAN_CONTROLLER_START);
	//hsem_signal(HSEM_APP_CAN_CONTROLLER_START, HSEM_ID_APP_CAN_CONTROLLER_START);

	/* Wait for CAN controller to be up and running. */
	//TODO: Make this actually check if CAN has been init-ed not just a delay.
	vTaskDelay(500);

	/*Change the priority back to 2.*/
	vTaskPrioritySet(NULL, 2);

	/* While _run is set to true. */
	while (_run)
	{
		/* Check if there is data in FIFO1. */

			/*If there's not, wait for a bit and check again. */
			vTaskDelay(30);

		/* Check the PID code against what it should be. */
		can_rx_buffer_entry_t* new_data = NULL;
		//shared_get_can_rx1_buffer_entry(FDCAN1, 0, new_data);

		/* Convert the raw CAN data into something that can be sent to LVGL. */
		int32_t processed_data = saej1979_current_data_process_data(new_data);

		//if (xSemaphoreTake(sys_mutex_lvgl, portMAX_DELAY) == pdPASS)
		//{
		//	ui_gauges_set_gauge_value(processed_data);
		//	xSemaphoreGive(sys_mutex_lvgl);
		//}
		vTaskDelay(25);
	}
	/* Stop running. */



	/* Delete the task. */
	vTaskDelete(NULL);

}

static void _gauge_btn_event_cb(lv_event_t* e)
{
	/* Determine which button was clicked. */
	lv_obj_t* btn = lv_event_get_target_obj(e);
	lv_obj_t* lbl = lv_obj_get_child(btn, 0);
	const char* lbl_text = lv_label_get_text(lbl);

	saej1979_current_data_set_getter(lbl_text);
}

static void _gauge_event_cb(lv_event_t* e)
{
	/* Tell CM4 to stop transmitting data. */
	//shared_set_can_tx_unique_ids(FDCAN1, 0);
}

static void _gauge_scr_load_cb()
{
	for (uint8_t i = 0; i < 209; i++)
	{
		saej1979_current_data_t* x = saej1979_get_current_data(i);
		if (x->gauge == true)
		{
			ui_gauges_create_gauge_btn(saej1979_get_current_data(i)->name);
		}
	}
}

static void _gauge_select_btn_cb(lv_event_t* e)
{
	lv_obj_t* btn = lv_event_get_target_obj(e);
	lv_obj_t* lbl = lv_obj_get_child(btn, 0);
	char* txt = lv_label_get_text(lbl);

	for (uint8_t i = 0; i < 209; i++)
	{
		saej1979_current_data_t* x = saej1979_get_current_data(i);
		if (x->gauge == false)
		{
			continue;
		}
		if (strcmp(x->name, txt) == 0)
		{
			ui_gauges_create_gauge(txt, x->min, x->max);
			if (ui_helpers_is_demo_mode() == false)
			{
				saej1979_current_data_set_getter(i);
			}
			return;
		}
	}
}

/**********		GLOBAL FUNCTION DEFINITIONS		**********/
void app_gauges_run()
{
	xTaskCreate(_task_gauges, "APP_GAUGES", 800, NULL, 4, _gauges_task_handle);
}

void app_gauges_stop()
{

}
