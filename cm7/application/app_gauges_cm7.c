/**********     INCLUDES        **********/
#include "cangauge_common.h"
#include "app_gauges_cm7.h"
#include "system/system_cm7.h"

#include "applications_cm7.h"

#include "ui/ui_gauges.h"
#include "ui/ui_helpers.h"

#include "drivers/drivers.h"

#include "lvgl.h"

#include "common/saej1979.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/**********		DEFINES		**********/

/**********		EXTERNAL VARIABLE DEFINITIONS		**********/

/**********		STATIC VARIABLES		**********/
static bool prv_task_run = false;
static TaskHandle_t prv_gauges_task_handle;
static saej1979_current_data_t* active_param = NULL;

/* Creates an area in external RAM for the CAN controller task. */
CG_MEMORY_REGION_EXT static uint8_t can_control_memory[176][10];

/* Creates an area in external RAM for the CAN transmit task. */
CG_MEMORY_REGION_EXT static uint16_t can_transmit_period_list[10];

/**********		STATIC FUNCTION DECLRATIONS		**********/
static void prv_task_gauges();							//The FreeRTOS task.

static bool prv_update_available_uds_data();			//Checks to see if the CAN controller task found UDS data, returns false if there's nothing there.
static void prv_create_gauge_select_btns();				//Creates the buttons on the GUI.

static void prv_gauge_event_cb(lv_event_t* e);			//Handler for the gauge itself events.
static void prv_gauge_scr_load_cb(lv_event_t* e);		//Handler for the gauge screen loading.
static void prv_gauge_select_btn_cb(lv_event_t* e);		//Handler for a available being selected.
static void prv_gauge_back_btn_cb(lv_event_t* e);		//Handler for if the back button is pressed.

/**********		STATIC FUNCTION DEFINITIONS		**********/
static void prv_task_gauges()
{
	if (ui_helpers_is_demo_mode())
	{
		app_gauges_stop();
		vTaskDelete(NULL);
	}

	/* Start the CAN transmitter task. */
	common.p_can_transmit_period_list = can_transmit_period_list;
	app_can_transmit_run(can_transmit_period_list, 10);

	/* Start the CAN receiver task. */
	common.p_can_controller_memory = &can_control_memory;
	app_can_controller_run(&can_control_memory);

	/* Set the LVGL event callbacks. */
	ui_gauges_set_gauge_cb(prv_gauge_event_cb);
	ui_gauges_set_scr_load_cb(prv_gauge_scr_load_cb);
	ui_gauges_set_gauge_select_btn_cb(prv_gauge_select_btn_cb);
	ui_gauges_set_back_btn_cb(prv_gauge_back_btn_cb);

	/*Change the priority back to 2.*/
	vTaskPrioritySet(NULL, 2);

	while (app_can_controller_is_init() == false)
	{
		vTaskDelay(100);
	}

	uint8_t retries = 0;
	while (retries++ < 5)
	{
		if (prv_update_available_uds_data())
		{
			prv_create_gauge_select_btns();
			prv_task_run = true;
			break;
		}
		prv_task_run = false;
		vTaskDelay(pdMS_TO_TICKS(200));
	}

	/* While _run is set to true. */
	while (prv_task_run)
	{
		uint8_t current_pid = active_param->pid_code;
		uint8_t num_params = active_param->data_bytes;
		uint8_t first_byte = active_param->first_byte;
		uint32_t raw_value = 0;
		for (uint8_t i = 0; i < num_params; i++)
		{
			raw_value |= (*common.p_can_controller_memory)[current_pid][i + first_byte] << ((num_params - (i+1)) * 8);
		}

		float scale = active_param->scale;
		float offset = active_param->offset;
		float processed_val = ((float)raw_value * scale) + offset;

		if (xSemaphoreTake(sys_mutex_lvgl, portMAX_DELAY) == pdPASS)
		{
			ui_gauges_set_gauge_value(processed_val);
			xSemaphoreGive(sys_mutex_lvgl);
		}
		vTaskDelay(25);
	}
	/* Stop running. */



	/* Delete the task. */
	vTaskDelete(NULL);

}

static bool prv_update_available_uds_data()
{
	uint8_t num_params = 0;

	/* Check the generic parameters. */
	for (uint8_t x = 0; x < 0x80; x += 0x20)
	{
		uint32_t available_pids_1 = can_control_memory[x][0] << 24;
		available_pids_1 |= can_control_memory[x][1] << 16;
		available_pids_1 |= can_control_memory[x][2] << 8;
		available_pids_1 |= can_control_memory[x][3];
		for (int8_t i = 31; i >= 0; i--)
		{
			saej1979_current_data_t* y = saej1979_get_current_data(32 - i + x);
			if ((available_pids_1 & (1 << i)) != 0)
			{
				y->available = true;
				num_params++;
			}
			else
			{
				y->available = false;
			}
		}
	}
	

	return num_params;
}

static void prv_create_gauge_select_btns()
{
	for (uint8_t i = 0; i < 176; i++)
	{
		saej1979_current_data_t* y = saej1979_get_current_data(i);
		if ((y->available == true) && (y->min != y->max))
		{
			ui_gauges_create_gauge_btn(saej1979_get_current_data(i)->name);
		}
	}
}

static void prv_gauge_event_cb(lv_event_t* e)
{
	/* Stop transmitting the requestor on CAN. */
	common.p_can_transmit_period_list[0] = 0;
}

static void prv_gauge_scr_load_cb()
{

}

static void prv_gauge_select_btn_cb(lv_event_t* e)
{
	lv_obj_t* btn = lv_event_get_target_obj(e);
	lv_obj_t* lbl = lv_obj_get_child(btn, 0);
	char* txt = lv_label_get_text(lbl);

	for (uint8_t i = 0; i < 176; i++)
	{
		saej1979_current_data_t* x = saej1979_get_current_data(i);
		if (x->available == false)
		{
			continue;
		}
		if (strcmp(x->name, txt) == 0)
		{
			ui_gauges_create_gauge(txt, x->units, x->min, x->max);
			active_param = x;
			saej1979_current_data_set_getter(i);
			return;
		}
	}
}

static void prv_gauge_back_btn_cb(lv_event_t* e)
{
	app_can_transmit_stop();
	app_can_controller_stop();
	app_gauges_stop();
}

/**********		GLOBAL FUNCTION DEFINITIONS		**********/
void app_gauges_run()
{
	prv_task_run = true;
	xTaskCreate(prv_task_gauges, "APP_GAUGES", 800, NULL, 4, prv_gauges_task_handle);
}

void app_gauges_stop()
{
	prv_task_run = false;
}
