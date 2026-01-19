/**********     INCLUDES        **********/
#include <cangauge_common.h>
#include "app_gauges_cm7.h"
#include "system/system_cm7.h"

#include "applications_cm7.h"

#include "ui/ui_gauges.h"

#include "drivers/drivers.h"

#include "lvgl.h"

#include "common/saej1979.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/**********		DEFINES		**********/

/**********		EXTERNAL VARIABLE DEFINITIONS		**********/

/**********		STATIC VARIABLES		**********/
bool _run = false;
static TaskHandle_t prv_gauges_task_handle;
static saej1979_current_data_t* active_param = NULL;

/**********		STATIC FUNCTION DECLRATIONS		**********/
static void _task_gauges();							//The FreeRTOS task.

static void prv_gauge_event_cb(lv_event_t* e);			//Handler for the gauge itself events.
static void prv_gauge_scr_load_cb(lv_event_t* e);		//Handler for the gauge screen loading.
static void prv_gauge_select_btn_cb(lv_event_t* e);	//Handler for a gauge being selected.

/**********		STATIC FUNCTION DEFINITIONS		**********/
static void _task_gauges()
{
	_run = true;

	/* Set the LVGL event callbacks. */
	ui_gauges_set_gauge_cb(prv_gauge_event_cb);
	ui_gauges_set_scr_load_cb(prv_gauge_scr_load_cb);
	ui_gauges_set_gauge_select_btn_cb(prv_gauge_select_btn_cb);

	/*Change the priority back to 2.*/
	vTaskPrioritySet(NULL, 2);

	/* While _run is set to true. */
	while (_run)
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

static void prv_gauge_event_cb(lv_event_t* e)
{
	/* Stop transmitting the requestor on CAN. */
	common.p_can_transmit_period_list[0] = 0;
}

static void prv_gauge_scr_load_cb()
{
	CG_MEMORY_REGION_EXT static uint16_t can_transmit_period_list[10];
	common.p_can_transmit_period_list = can_transmit_period_list;
	app_can_transmit_run(can_transmit_period_list, 10);

	CG_MEMORY_REGION_EXT static uint8_t can_control_memory[176][10];
	common.p_can_controller_memory = &can_control_memory;
	app_can_controller_run(&can_control_memory);

	vTaskDelay(1000);

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
				y->gauge = true;
			}
			else
			{
				y->gauge = false;
			}
		}
	}


	for (uint8_t i = 0; i < 176; i++)
	{
		saej1979_current_data_t* y = saej1979_get_current_data(i);
		if ((y->gauge == true) && (y->units != NULL))
		{
			ui_gauges_create_gauge_btn(saej1979_get_current_data(i)->name);
		}
	}
}

static void prv_gauge_select_btn_cb(lv_event_t* e)
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
			active_param = x;
			saej1979_current_data_set_getter(i);
			return;
		}
	}
}

/**********		GLOBAL FUNCTION DEFINITIONS		**********/
void app_gauges_run()
{
	xTaskCreate(_task_gauges, "APP_GAUGES", 800, NULL, 4, prv_gauges_task_handle);
}

void app_gauges_stop()
{

}
