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
#define EVENT_BITS_TASK_STOPPED			0x1 << 0		//Set when the task is stopped.
#define EVENT_BITS_QUERY_TRANSMITTING	0x1 << 1		//Set when the ISO15765 query is transmitting.

/**********		EXTERNAL VARIABLE DEFINITIONS		**********/

/**********		STATIC VARIABLES		**********/
static bool prv_task_run = false;
static TaskHandle_t prv_gauges_task_handle;
static EventGroupHandle_t prv_event_group = NULL;
static saej1979_current_data_t* active_param[4] = { NULL, NULL, NULL, NULL };
static can_transmit_handle_t* prv_current_data_query[4] = { NULL, NULL, NULL, NULL };
static uint32_t prv_can_id = 0x00;
static can_id_t prv_id_type = 0x00;

/**********		STATIC FUNCTION DECLRATIONS		**********/
static void prv_task_gauges();							//The FreeRTOS task.

static bool prv_update_available_uds_data();			//Checks to see if the CAN controller task found UDS data, returns false if there's nothing there.
static void prv_create_gauge_select_btns();				//Creates the buttons on the GUI.

static void prv_gauge_event_cb(lv_event_t* e);			//Handler for the gauge itself events.
static void prv_gauge_scr_load_cb(lv_event_t* e);		//Handler for the gauge screen loading.
static void prv_gauge_view_btn_cb(lv_event_t* e);		//Handler for a available being selected.
static void prv_gauge_back_btn_cb(lv_event_t* e);		//Handler for if the back button is pressed.

static void prv_brightness_slider_handler(lv_event_t* e);	//Handler for the brightness slider being changed.
static void prv_menu_scr_load_handler(lv_event_t* e);

static void prv_settings_btn_clicked_cb(lv_event_t* e);
static void prv_settings_back_btn_clicked_cb(lv_event_t* e);

/**********		STATIC FUNCTION DEFINITIONS		**********/
static void prv_task_gauges()
{
	/* Load the UI. */
	ui_gauges_load();

	//TODO: This check for demo mode is obsolete bc were starting from gauges now not the main menu.
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
	can_transmit_run(FDCAN1, 5);

	/* Start the CAN receiver task. */
	app_can_controller_run();

	/* Set the LVGL event callbacks. */
	ui_gauges_set_gauge_cb(prv_gauge_event_cb);			//A gauge is clicked (go back to selection screen).
	ui_gauges_set_scr_load_cb(prv_gauge_scr_load_cb);		//The gauge screen loads (nothing programmed).
	ui_gauges_set_view_btn_cb(prv_gauge_view_btn_cb);		//A gauge is selected (load the gauge and set the CAN getter).

    ui_set_brightness_slider_event_cb(prv_brightness_slider_handler);		//The brightness slider is changed (change the screen brightness).
    ui_set_settings_scr_load_event_cb(prv_menu_scr_load_handler);		//The settings screen is loaded (recall the screen brightness value and demo mode status).
    ui_set_settings_btn_event_cb(prv_settings_btn_clicked_cb);		//Stop the gauges and CAN tasks.
    ui_set_settings_back_btn_event_cb(prv_settings_back_btn_clicked_cb);		//Start the gauges and CAN tasks again.

	/*Change the priority back to 2.*/
	vTaskPrioritySet(NULL, 2);

	/* Wait for the CAN controller to initialize. */
	if (!app_can_controller_is_init( pdMS_TO_TICKS(10000) ) )
	{
		/* Do something if it fails. */
	}

	bool found_car = app_can_controller_get_can_id();

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

	/* Print out bus info, for debugging. */
	uint32_t avail_pids_1 = can_controller_get_data(0x00, 0, 4);
	uint32_t avail_pids_2 = can_controller_get_data(0x20, 0, 4);
	uint32_t avail_pids_3 = can_controller_get_data(0x40, 0, 4);
	uint32_t avail_pids_4 = can_controller_get_data(0x60, 0, 4);
	uint32_t can_id = app_can_controller_get_can_id();
	uint32_t rx_ecr = can_get_rx_error_counter(FDCAN1);
	uint32_t tx_ecr = can_get_tx_error_counter(FDCAN1);
	can_error_code_t ec = can_get_last_error_code(FDCAN1);
	char* label = calloc(300, sizeof(uint8_t));
	uint32_t str_size = sprintf(label, "PIDs 0x00: 0x%X\n \
										PIDs 0x20: 0x%X\n \
										PIDs 0x40: 0x%X\n \
										PIDs 0x60: 0x%X\n \
										CAN ID: 0x%X\n \
										RX ECR: %d\n \
										TX ECR: %d\n \
										LEC: %d",
										avail_pids_1, avail_pids_2, avail_pids_3, avail_pids_4, can_id,
										rx_ecr, tx_ecr, ec);
	realloc(label, str_size);

	/* Write the label to the screen. */
	lv_port_take_lvgl_mutex(portMAX_DELAY);
	ui_helpers_add_text_to_act_scr(label, LV_ALIGN_CENTER, 0, 425);
	lv_port_give_lvgl_mutex();
	free(label);

	TickType_t last_wake_time = xTaskGetTickCount();

	/********** 	TASK LOOP	**********/
	/* While _run is set to true. */
	while (prv_task_run)
	{
		/* Check to see if we are transmitting. */
	    uint32_t rtn = xEventGroupWaitBits(prv_event_group, EVENT_BITS_QUERY_TRANSMITTING,    //Bits to wait for.
	                                        pdFALSE,        //Dont clear the bits on exit.
	                                        pdTRUE,         //wait for all the bits (it's only 1)
	                                        pdMS_TO_TICKS(500)); //Block time.
		if ((rtn & EVENT_BITS_QUERY_TRANSMITTING) != 0)
		{
			for (uint8_t d = 0; d < 4; d++)
			{
				if (active_param[d] == NULL)
				{
					break;
				}
				uint8_t current_pid = active_param[d]->pid_code;
				uint8_t num_params = active_param[d]->data_bytes;
				uint8_t first_byte = active_param[d]->first_byte;
				uint32_t raw_value = can_controller_get_data(current_pid, first_byte, num_params);

				float scale = active_param[d]->scale;
				float offset = active_param[d]->offset;
				float processed_val = ((float)raw_value * scale) + offset;

				if (lv_port_take_lvgl_mutex(portMAX_DELAY))
				{
					ui_gauges_set_gauge_value(processed_val, d);
					lv_port_give_lvgl_mutex();
				}
			}

		}
		vTaskDelayUntil(&last_wake_time, 25);
	}
	/****************************************/

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
			ui_gauges_create_gauge_checkbox(saej1979_get_current_data(i)->name);
			lv_port_give_lvgl_mutex();
		}
	}
}

static void prv_gauge_event_cb(lv_event_t* e)
{
	/* Stop transmitting the requestor on CAN. */
	can_transmit_set_inactive(prv_current_data_query[0]);
	can_transmit_set_inactive(prv_current_data_query[1]);
	xEventGroupClearBits(prv_event_group, EVENT_BITS_QUERY_TRANSMITTING);

}

static void prv_gauge_scr_load_cb()
{

}

static void prv_gauge_view_btn_cb(lv_event_t* e)
{
	/* I tried to just access the pointer but it was NOT working so I'm using memcpy for now. */
	/* Copy the lv_checkbox pointers. */
	lv_obj_t* gauge_select_checkboxes[4];
	void* src_addr = lv_event_get_user_data(e);
	memcpy(&gauge_select_checkboxes, src_addr, sizeof(lv_obj_t*) * 4);

	/* Determine how many are checked, this tell us how many gauges to display. */
	uint8_t num_gauges = 0;
	while (gauge_select_checkboxes[num_gauges] != NULL)
	{
		num_gauges++;
		if (num_gauges == 4)
		{
			break;
		}
	}

	/* If no gauges are selected, return. */
	if (num_gauges == 0)
	{
		return;
	}

	/* Zero out the active_param array. */
	memset(&active_param, 0, sizeof(saej1979_current_data_t*) * 4);

	/* Tell the UI how many gauges were gonna load. */
	ui_gauges_set_number_of_gauges(num_gauges);

	/* Load the gauges into the UI and set the ISO15675 query on CAN. */
	for (uint8_t g = 0; g < num_gauges; g++)
	{
		const char* txt = lv_checkbox_get_text(gauge_select_checkboxes[g]);
		for (uint8_t i = 0; i < 176; i++)
		{
			saej1979_current_data_t* x = saej1979_get_current_data(i);
			/* Check to see if this PID is supported by CANgauge, continue if not. */
			if (x->available == false)
			{
				continue;
			}

			/* Check to see if the checkbox text matches the PID text. */
			if (strcmp(x->name, txt) == 0)
			{
				/* For gauges on the right side of the screen we want to swap the min and max values. */
				if ((g % 2) != 0)
				{
					ui_gauges_create_gauge(txt, x->units, x->max, x->min, g);
				}
				else
				{
					ui_gauges_create_gauge(txt, x->units, x->min, x->max, g);
				}
				active_param[g] = x;
				//return;
			}
		}
	}
	/* If the active param is NULL, set the PID to 0, otherwise set it to the PID code. */
	uint8_t pid0 = (active_param[0] == NULL ) ? 0 : active_param[0]->pid_code;
	uint8_t pid1 = (active_param[1] == NULL ) ? 0 : active_param[1]->pid_code;
	uint8_t pid2 = (active_param[2] == NULL ) ? 0 : active_param[2]->pid_code;
	uint8_t pid3 = (active_param[3] == NULL ) ? 0 : active_param[3]->pid_code;
	saej1979_set_current_data_query(pid0, pid1, pid2, pid3);
	xEventGroupSetBits(prv_event_group, EVENT_BITS_QUERY_TRANSMITTING);
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
	
	/* If the event group for this task hasn't been created, create it.*/
	if (prv_event_group == NULL)
	{
		prv_event_group = xEventGroupCreate();	
	}

	/* Clear the TASK_STOPPED bit. */
	xEventGroupClearBits(prv_event_group, EVENT_BITS_TASK_STOPPED);

	/* Create the task. */
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


void saej1979_set_current_data_query(uint8_t pid1, uint8_t pid2, uint8_t pid3, uint8_t pid4)
{
	/* Data byte 2 needs to be changed depending on the data being requested. */
	can_tx_buffer_entry_t iso15765_query =
	{
		.T0.bit.ID = prv_can_id, .T0.bit.XTD = prv_id_type, .T0.bit.RTR = CAN_RTR_DATA_FRAME,
		.T1.bit.DLC = 8, .T1.bit.EFC = 0, .T1.bit.BRS = 0, .T1.bit.FDF = 0,
		.data[0] = 0x02, .data[1] = 0x01, .data[2] = 0x00, .data[3] = 0xCC,
		.data[4] = 0xCC, .data[5] = 0xCC, .data[6] = 0xCC, .data[7] = 0xCC,
	};

	/* Create a CAN TX message if it hasnt been created already. */
	if (prv_current_data_query[0] == NULL)
	{
		prv_current_data_query[0] = can_transmit_create_msg();
		prv_current_data_query[1] = can_transmit_create_msg();
		prv_current_data_query[2] = can_transmit_create_msg();
		prv_current_data_query[3] = can_transmit_create_msg();
	}

	/* Determine how many PIDs are being passed in. */
	int8_t num_pids = 4;
	if (pid1 == 0) { return; }
	else if (pid2 == 0) { num_pids = 1; }
	else if (pid3 == 0) { num_pids = 2; }
	else if (pid4 == 0) { num_pids = 3; }
	//iso15765_query.data[0] = num_pids + 1;		//This sets the data length code (DLC).
	/* These set the data field. */
	iso15765_query.data[2] = pid1;

	for (;num_pids > 0; num_pids--)
	{
		switch (num_pids - 1)
		{
		case 0:
			iso15765_query.data[2] = pid1;
			break;
		case 1:
			iso15765_query.data[2] = pid2;
			break;
		case 2:
			iso15765_query.data[2] = pid3;
			break;
		case 3:
			iso15765_query.data[2] = pid4;
			break;
		}
		/* Add the CAN message, set its transmit period, and activate it. */
		can_transmit_set_msg_data(prv_current_data_query[num_pids - 1], &iso15765_query);
		can_transmit_set_period(prv_current_data_query[num_pids - 1], 25);
		can_transmit_set_active(prv_current_data_query[num_pids - 1]);
	}

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

static void prv_brightness_slider_handler(lv_event_t* e)
{
    lv_obj_t* obj = lv_event_get_target_obj(e);
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_VALUE_CHANGED)
    {
        uint8_t slider_val = lv_slider_get_value(obj);      //Returns a value between 0 and 100.
        uint32_t timer_val = (605 * slider_val) + 5000;		//Map the slider value of 0 to 100 to 5000 to 65535.
        timer_set_pwm_duty_cycle(TIM12, timer_val, 1);
    }

}

static void prv_menu_scr_load_handler(lv_event_t* e)
{
	lv_event_code_t code = lv_event_get_code(e);

	if (code == LV_EVENT_SCREEN_LOADED)
	{
			lv_obj_t** slider = lv_event_get_user_data(e);
			uint32_t timer_val = timer_get_pwm_duty_cycle(TIM12, 1);
			uint32_t slider_val = (timer_val - 5000) / 605;
			lv_slider_set_value(*slider, slider_val, LV_ANIM_OFF);
	}

}

static void prv_settings_btn_clicked_cb(lv_event_t* e)
{
	/* Wait until all the tasks have been stopped. */
	can_transmit_stop(0);
	app_can_controller_stop(0);
	app_gauges_stop(portMAX_DELAY);
}

static void prv_settings_back_btn_clicked_cb(lv_event_t* e)
{
	app_gauges_run();
}
