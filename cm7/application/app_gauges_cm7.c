/**********     INCLUDES        **********/
#include "cangauge_common.h"
#include "app_gauges_cm7.h"
#include "app_gauges_prv.h"

#include "applications_cm7.h"

#include "ui/ui_gauges.h"
#include "ui/ui_helpers.h"

#include "drivers/drivers.h"

#include "lvgl.h"

#include "lvgl_port/lvgl_port_def.h"



/**********		DEFINES		**********/
#define EVENT_BITS_TASK_STOPPED			0x1 << 0

/**********		EXTERNAL VARIABLE DEFINITIONS		**********/

/**********		STATIC VARIABLES		**********/
static bool prv_task_run = false;
static TaskHandle_t prv_gauges_task_handle;
static EventGroupHandle_t prv_event_group = NULL;
static saej1979_current_data_t* active_param = NULL;
static can_transmit_handle_t* prv_current_data_query = NULL;
static uint32_t prv_can_id = 0x00;
static can_id_t prv_id_type = 0x00;

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
		/* Stop the task but dont block at all because we're calling from in the task just to set prv_run to false. */
		app_gauges_stop(pdMS_TO_TICKS(0));
		vTaskDelete(NULL);
	}

	/* Turn on the CAN peripheral and set the baud rate. */
	can_init(FDCAN1);
	can_set_baud_rate(FDCAN1, CAN_BAUD_500K);
	can_run(FDCAN1);

	/* Start the CAN transmitter task. */
	can_transmit_run(FDCAN1, 15);

	/* Start the CAN receiver task. */
	app_can_controller_run();

	/* Set the LVGL event callbacks. */
	ui_gauges_set_gauge_cb(prv_gauge_event_cb);
	ui_gauges_set_scr_load_cb(prv_gauge_scr_load_cb);
	ui_gauges_set_gauge_select_btn_cb(prv_gauge_select_btn_cb);
	ui_gauges_set_back_btn_cb(prv_gauge_back_btn_cb);

	/*Change the priority back to 2.*/
	vTaskPrioritySet(NULL, 2);

	/* Wait for the CAN controller to initialize. */
	bool found_car = app_can_controller_is_init(portMAX_DELAY);

	if (found_car == true)
	{
		prv_can_id = app_can_controller_get_can_id();
		prv_id_type = (prv_can_id > 0x7ff) ? CAN_ID_XTD : CAN_ID_STD;
		prv_update_available_uds_data();
		prv_create_gauge_select_btns();
	}
	else
	{
		prv_task_run = false;
	}

	/* While _run is set to true. */
	while (prv_task_run)
	{
		uint8_t current_pid = active_param->pid_code;
		uint8_t num_params = active_param->data_bytes;
		uint8_t first_byte = active_param->first_byte;
		uint32_t raw_value = can_controller_get_data(current_pid, first_byte, num_params);

		float scale = active_param->scale;
		float offset = active_param->offset;
		float processed_val = ((float)raw_value * scale) + offset;

		if (lv_port_take_lvgl_mutex(portMAX_DELAY))
		{
			ui_gauges_set_gauge_value(processed_val);
			lv_port_give_lvgl_mutex();
		}
		vTaskDelay(25);
	}
	/* Stop running. */
	app_can_controller_stop(portMAX_DELAY);
	can_transmit_stop(portMAX_DELAY);
	can_stop(FDCAN1);
	can_deinit();
	xEventGroupSetBits(prv_event_group, EVENT_BITS_TASK_STOPPED);

	/* Delete the task. */
	vTaskDelete(NULL);

}

static bool prv_update_available_uds_data()
{
	uint8_t num_params = 0;

	/* Check the generic parameters. */
	for (uint8_t x = 0; x < 0x80; x += 0x20)
	{
		uint32_t available_pids_1 = can_controller_get_data(x, 0, 4);
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
			lv_port_take_lvgl_mutex(portMAX_DELAY);
			ui_gauges_create_gauge_btn(saej1979_get_current_data(i)->name);
			lv_port_give_lvgl_mutex();
		}
	}
}

static void prv_gauge_event_cb(lv_event_t* e)
{
	/* Stop transmitting the requestor on CAN. */
	can_transmit_set_inactive(prv_current_data_query);
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
	/* Wait until all the tasks have been stopped. */
	can_transmit_stop(0);
	app_can_controller_stop(0);
	app_gauges_stop(0);
}

/**********		GLOBAL FUNCTION DEFINITIONS		**********/
void app_gauges_run()
{
	prv_task_run = true;
	
	if (prv_event_group == NULL)
	{
		prv_event_group = xEventGroupCreate();	
	}
	xEventGroupClearBits(prv_event_group, EVENT_BITS_TASK_STOPPED);

	xTaskCreate(prv_task_gauges, "APP_GAUGES", 800, NULL, 4, prv_gauges_task_handle);
}

bool app_gauges_stop(uint32_t block_time_ms)
{
    prv_task_run = false;

	/* If the event group is NULL, the task was never even created in the first place. */
    if (prv_event_group == NULL)
    {
    	return pdTRUE;
    }

    uint32_t rtn = xEventGroupWaitBits(prv_event_group, EVENT_BITS_TASK_STOPPED,    //Bits to wait for.
                                        pdFALSE,        //Dont clear the bits on exit.
                                        pdTRUE,         //wait for all the bits (it's only 1)
                                        block_time_ms); //Block time.

    if (rtn & EVENT_BITS_TASK_STOPPED != 0)
    {
        return pdTRUE;
    }
    else
    {
        return pdFALSE;
    }
}


void saej1979_current_data_set_getter(uint8_t pid)
{
	/* Data byte 2 needs to be changed depending on the data being requested. */
	can_tx_buffer_entry_t saej1979_getter_template =
	{
		.T0.bit.ID = prv_can_id, .T0.bit.XTD = prv_id_type, .T0.bit.RTR = CAN_RTR_DATA_FRAME,
		.T1.bit.DLC = 8, .T1.bit.EFC = 0, .T1.bit.BRS = 0, .T1.bit.FDF = 0,
		.data[0] = 0x02, .data[1] = 0x01, .data[2] = 0x00, .data[3] = 0xCC,
		.data[4] = 0xCC, .data[5] = 0xCC, .data[6] = 0xCC, .data[7] = 0xCC,
	};

	if (prv_current_data_query == NULL)
	{
		prv_current_data_query = can_transmit_create_msg();
	}
	saej1979_getter_template.data[2] = pid;
	can_transmit_set_msg_data(prv_current_data_query, &saej1979_getter_template);
	can_transmit_set_period(prv_current_data_query, 25);
	can_transmit_set_active(prv_current_data_query);
	return;
}

int32_t saej1979_current_data_process_data(can_rx_buffer_entry_t* input)
{
	uint32_t id = input->R0.bit.ID;
	uint8_t pid = input->data[2];
	uint32_t data = 0;
	uint32_t num_params = sizeof(saej1979_current_data_arr) / sizeof(saej1979_current_data_t*);
    for (uint8_t i = 0; i < num_params; i++)
	{
		if (pid == saej1979_current_data_arr[i]->pid_code)
		{
			if (saej1979_current_data_arr[i]->data_bytes == 1)
			{
				data = input->data[3];
			}
			else if (saej1979_current_data_arr[i]->data_bytes == 2)
			{
				data = (input->data[3] << 8) | input->data[4];
			}
			//data = saej1979_current_data_arr[i]->conversion_func(data);
			return data;
		}
	}	
	return 0;
}

saej1979_current_data_t* saej1979_get_current_data(uint8_t pid)
{
	return saej1979_current_data_arr[pid];
}
