/**********     INCLUDES        **********/
#include "gauges.h"
#include "application/can_uds_def.h"
#include "application/can_uds.h"
#include "application/data_logger.h"
#include "system/error_handler.h"
#include "ui/ui_gauges.h"
#include "file_mngr/file_mngr.h"
#include "pwr_monitor.h"


/**********		DEFINES		**********/
#define EVENT_BITS_TASK_STOPPED			0x1 << 0		//Set when the task is stopped.
#define EVENT_BITS_QUERY_TRANSMITTING	0x1 << 1		//Set when the ISO15765 query is transmitting.

/**********		EXTERNAL VARIABLE DEFINITIONS		**********/

/**********		STATIC VARIABLES		**********/
static TaskHandle_t prv_gauges_task_handle;
static EventGroupHandle_t prv_event_group = NULL;
static saej1979_current_data_t* active_param[4] = { NULL, NULL, NULL, NULL };
static data_logger_handle_t prv_data_logger_handle;

/**********		STATIC FUNCTION DECLRATIONS		**********/
static void prv_task_gauges();							//The FreeRTOS task.
static uint8_t prv_load_gauges(char *str[4], uint8_t num_gauges);		//Sends the gauge data to the UI, must call ui_load_gauge_screen afterwards to display them. Returns the number of gauges submitted that were found on OBD.
static void prv_create_gauge_select_btns();				//Creates the buttons on the GUI.
static void prv_save_vin_to_file();

/* Function callbacks. */
static void prv_gauge_event_cb(lv_event_t* e);				//Handler for the gauge itself events.
static void prv_gauge_view_btn_cb(lv_event_t* e);			//Handler for a available being selected.
static void prv_toggle_data_logging_cb(lv_event_t* e);		//Handler for when the gauge screen is long pressed meaning it's time to start or stop data logging.
static void prv_data_logger_error_cb(data_logger_error_code_t code);	//Handler for if the data logger errors out.
static void prv_low_power_mode_cb();
/**********		STATIC FUNCTION DEFINITIONS		**********/
static void prv_task_gauges()
{
	/* Create an event group for this file/task. */
	if (prv_event_group == NULL)
	{
		prv_event_group = xEventGroupCreate();
	}

	/* Start the CAN receiver task. */
	assert( can_uds_run() == pdPASS );

	assert(lv_port_take_lvgl_mutex(portMAX_DELAY));
	/* Set the LVGL event callbacks. */
	ui_gauges_set_gauge_single_clicked_cb(prv_gauge_event_cb);					//A gauge is clicked (go back to selection screen).
	ui_gauges_set_view_btn_cb(prv_gauge_view_btn_cb);							//A gauge is selected (load the gauge and set the CAN getter).
	ui_gauges_set_gauge_long_pressed_cb(prv_toggle_data_logging_cb);    
	lv_port_give_lvgl_mutex();

    /* Set the low power mode callbacks. */
    pwr_monitor_add_low_pwr_mode_cb(prv_low_power_mode_cb);

	/*Change the priority back to 2.*/
	vTaskPrioritySet(NULL, 2);

	/* Wait for the CAN controller to initialize. */
	if (!app_can_controller_is_init( pdMS_TO_TICKS(10000) ) )
	{
		error_show_msgbox("Error: UDS driver timeout.");
		prv_gauges_task_handle = NULL;
		vTaskDelete(NULL);
	}

	bool found_car = can_uds_get_query_can_id();

	if (found_car == true)
	{
		prv_create_gauge_select_btns();			//Creates the buttons in the UI.
	}
	else
	{
		error_show_msgbox("No UDS on CAN interface found. Make sure OBD II connection is good and ignition is on.\n\nContact support at support@can-gauge.com");
		prv_gauges_task_handle = NULL;
		vTaskDelete(NULL);
	}

	/* Print out bus info, for debugging. */
	unsigned int avail_pids_1 = can_uds_get_raw_current_data(0x00, 0, 4);
	unsigned int avail_pids_2 = can_uds_get_raw_current_data(0x20, 0, 4);
	unsigned int avail_pids_3 = can_uds_get_raw_current_data(0x40, 0, 4);
	unsigned int avail_pids_4 = can_uds_get_raw_current_data(0x60, 0, 4);
	uint32_t can_id = can_uds_get_response_can_id();
	uint32_t rx_ecr = can_get_rx_error_counter(FDCAN1);
	uint32_t tx_ecr = can_get_tx_error_counter(FDCAN1);
	can_error_code_t ec = can_get_last_error_code(FDCAN1);
	char* label = calloc(300, sizeof(uint8_t));
	uint32_t str_size = sprintf(label, "PIDs 0x00: 0x%X\n\
										PIDs 0x20: 0x%X\n\
										PIDs 0x40: 0x%X\n\
										PIDs 0x60: 0x%X\n\
										CAN ID: 0x%X\n\
										RX ECR: %lu\n\
										TX ECR: %lu\n\
										LEC: %lu\n\
										%s - %s",
										avail_pids_1, avail_pids_2, avail_pids_3, avail_pids_4, (unsigned int)can_id,
										rx_ecr, tx_ecr, ec, VERSION, BUILD_TYPE_STR);

	/* Write the diagnostic label to the screen. */
	lv_port_take_lvgl_mutex(portMAX_DELAY);
	ui_helpers_add_text_to_act_scr(label, LV_ALIGN_CENTER, 0, 425);
	lv_port_give_lvgl_mutex();
	free(label);

	/* Save the VIN data. */
	prv_save_vin_to_file();

	/* Check to see if there's a config file with the last state. */
	char* line = calloc(250, 1);
	uint32_t bytes_wr = file_mngr_get_config_data("LAST GAUGES STATE", line);
	if (bytes_wr != 0)
	{
		char* split[5];		//Hold the strings from the config file.
		char* sv_ptr;		//For strtok_r.
		split[0] = strtok_r(line, ",", &sv_ptr);

		/* Get each gauge PID. */
		for (uint8_t i = 1; i < 5; i++)
		{
			split[i] = strtok_r(NULL, ",", &sv_ptr);
		}
		uint8_t num_gauges = 0;
		while (strcmp(split[num_gauges + 1], "0") != 0)
		{
			num_gauges++;
			if (num_gauges == 4) { break; }
		}
		if (num_gauges != 0)
		{
			/* Load the gauge screen only if the gauges that were saved are currently being
			 * supported.
			 */
			if (prv_load_gauges(&split[1], num_gauges) == num_gauges)
			{
				lv_port_take_lvgl_mutex(portMAX_DELAY);
				ui_load_gauge_screen();
				lv_port_give_lvgl_mutex();
			}
		}
	}
	free(line);

	TickType_t last_wake_time = xTaskGetTickCount();		//This is for calculating delay time.

	/********** 	TASK LOOP	**********/
	/* While _run is set to true. */
	while (1)
	{
		/* Check to see if we are transmitting. */
	    uint32_t rtn = xEventGroupWaitBits(prv_event_group, EVENT_BITS_QUERY_TRANSMITTING,    //Bits to wait for.
	                                        pdFALSE,        //Dont clear the bits on exit.
	                                        pdTRUE,         //wait for all the bits (it's only 1)
	                                        pdMS_TO_TICKS(500)); //Block time.
		if ((rtn & EVENT_BITS_QUERY_TRANSMITTING) != 0)
		{
			//static float last_value[4];		//Use this to see if the value changed, if it didn't we dont change the gauge value,
			//then it wont get rendered and we get better lcd performance.
			for (uint8_t d = 0; d < 4; d++)
			{
				if (active_param[d] == NULL)
				{
					break;
				}
				uint8_t current_pid = active_param[d]->pid_code;
				uint8_t num_params = active_param[d]->data_bytes;
				uint8_t first_byte = active_param[d]->first_byte;
				uint32_t raw_value = can_uds_get_raw_current_data(current_pid, first_byte, num_params);

				float scale = active_param[d]->scale;
				float offset = active_param[d]->offset;
				float processed_val = ((float)raw_value * scale) + offset;

				//if (processed_val == last_value[d])
				//{
			//		continue;
				//}

				if (lv_port_take_lvgl_mutex(portMAX_DELAY))
				{
					ui_gauges_set_gauge_value(processed_val, d);
					lv_port_give_lvgl_mutex();
				}
				//last_value[d] = processed_val;
			}

		}
		vTaskDelayUntil(&last_wake_time, 25);
	}
	/****************************************/

	/* Delete the task. */
	vTaskDelete(NULL);

}

static void prv_create_gauge_select_btns()
{
	for (uint8_t i = 0; i < 176; i++)
	{
		saej1979_current_data_t* y = saej1979_get_current_data_lut_by_pid(i);
		if ((y->available == true) && ((y->min != y->max) || (y->nested != NULL)))
		{
			const char* txt;
			/* Check if this one is nested. */
			if (y->nested != NULL)
			{
				/* If it is, cycle through its parameters. */
				for (uint8_t idx = 0; idx < 8; idx++)
				{
					saej1979_current_data_t* x = y->nested[idx];
					if (x == NULL) { continue; }
					if (x->available)
					{
						txt = x->name;
						lv_port_take_lvgl_mutex(portMAX_DELAY);
						ui_gauges_create_gauge_checkbox(txt);
						lv_port_give_lvgl_mutex();
					}
				}

			}
			else
			{
				txt = y->name;
				lv_port_take_lvgl_mutex(portMAX_DELAY);
				ui_gauges_create_gauge_checkbox(txt);
				lv_port_give_lvgl_mutex();
			}
		}
	}
}

static void prv_save_vin_to_file()
{
	/* Get the VIN from the raw data storage. */
	char vin[18];
	memset(vin, 0, 18);
	for (uint8_t i = 0; i < 17; i++)
	{
		vin[i] = (char)can_uds_get_raw_infotype_data(0x2, i + 1, 1);
	}
	file_mngr_save_vin_to_file(vin);
}

static void prv_gauge_event_cb(lv_event_t* e)
{
	/* Stop transmitting the requestor on CAN. */
	can_uds_stop_query();
	xEventGroupClearBits(prv_event_group, EVENT_BITS_QUERY_TRANSMITTING);

	/* Write to the save state file. */
	char* str = "LAST GAUGES STATE,0,0,0,0,\n";
	file_mngr_set_config_data(str);

	/*Stop the data logger. */
	data_logger_stop_recording(&prv_data_logger_handle);
	lv_obj_clean(lv_layer_top());
}

static uint8_t prv_load_gauges(char* str[4], uint8_t num_gauges)
{
	uint8_t rtn_val = 0;		//Returns how many of the gauges are available on the platform.
	/* Tell the UI how many gauges were gonna load. */
	ui_gauges_set_number_of_gauges(num_gauges);

	/* Load the gauges into the UI and set the ISO15675 query on CAN. */
	for (uint8_t g = 0; g < num_gauges; g++)
	{
		const char *txt = str[g];
		for (uint8_t i = 0; i < 176; i++)
		{
			saej1979_current_data_t *x = saej1979_get_current_data_lut_by_pid(i);

			/* Check to see if this PID is supported on the current platform.
			 * If it's not, we dont need to check if it's a match so go to the
			 * next PID. */
			if (x->available == false)
			{
				continue;
			}

			/* Check if it's nested. */
			if (x->nested != NULL)
			{
				/* Cycle through the nested parameters. */
				for (uint8_t idx = 0; idx < 8; idx++)
				{
					saej1979_current_data_t* nested_x = x->nested[idx];
					if (nested_x == NULL) { continue; }
					/* Check to see if the current data element matches the label
					 * and set x equal to the nested element if it does. */
					if (strcmp(nested_x->name, txt) == 0)
					{
						x = nested_x;
						break;
					}
				}
			}

			/* Check to see if the checkbox text matches the PID text.
			 * This was already checked if it's nested but whatevuh. */
			if (strcmp(x->name, txt) == 0) {
				/* For gauges on the right side of the screen we want to swap the min and max values. */
				if ((g % 2) != 0) {
					ui_gauges_create_gauge(txt, x->units, x->max, x->min, g);
				} else {
					ui_gauges_create_gauge(txt, x->units, x->min, x->max, g);
				}
				active_param[g] = x;	//active_param tells the gauges task what's currently being displayed so it can process data.
				rtn_val++;
			}
		}
	}
	/* If the active param is NULL, set the PID to 0, otherwise set it to the PID code. */
	uint8_t pid0 = (active_param[0] == NULL) ? 0 : active_param[0]->pid_code;
	uint8_t pid1 = (active_param[1] == NULL) ? 0 : active_param[1]->pid_code;
	uint8_t pid2 = (active_param[2] == NULL) ? 0 : active_param[2]->pid_code;
	uint8_t pid3 = (active_param[3] == NULL) ? 0 : active_param[3]->pid_code;

	/* Check if anything is being transmitted and set the transmitting event bits if there is. */
	if (can_uds_set_current_data_query(pid0, pid1, pid2, pid3))
	{
		xEventGroupSetBits(prv_event_group, EVENT_BITS_QUERY_TRANSMITTING);
	}
	return rtn_val;
}

static void prv_gauge_view_btn_cb(lv_event_t* e)
{
	/* I tried to just access the pointer but it was NOT working so I'm using memcpy for now. */
	/* Copy the lv_checkbox pointers. */
	lv_obj_t* gauge_select_checkboxes[4];
	void* src_addr = lv_event_get_user_data(e);			//User data contains an array of up to 4 checkboxes that are selected from the UI.
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

	/* Start a string that we can write to the config file that saves what gauges are displayed. */
	char* str[4];
	const char* header = "LAST GAUGES STATE,";
	uint32_t str_len = strlen(header) + 8; 		//Plus 8 for 3 commas and an endline (4) and another 4 for zeros in case any of the strlen are zero.
	for (uint8_t i = 0; i < 4; i++)
	{
		if (gauge_select_checkboxes[i] == NULL)
		{
			continue;
		}
		str[i] = lv_checkbox_get_text(gauge_select_checkboxes[i]);
		str_len += strlen(str[i]);
	}

	char* save_str = (char*)malloc(str_len);
	if (str[0] == NULL)
	{
		return;
	}
	snprintf(save_str, strlen(str[0]) + strlen(header) + 1, "%s%s,", header, str[0]);
	for (uint32_t s = 1; s < 4; s++)
	{
		if (s < num_gauges)
		{
			strcat(save_str, ",");
			strcat(save_str, str[s]);
		}
		else
		{
			strcat(save_str, ",0");
		}
	}
	strcat(save_str, ",");
	str_len = strlen(save_str);		//Double check this.

	file_mngr_set_config_data(save_str);
	free(save_str);

	/* Load the gauges into the UI and set the ISO15675 query on CAN. */
	if (prv_load_gauges(str, num_gauges) == num_gauges)
	{
		ui_load_gauge_screen();
	}
}

/**********		GLOBAL FUNCTION DEFINITIONS		**********/
void app_gauges_run()
{
	if (prv_gauges_task_handle != NULL)
	{
		vTaskResume(prv_gauges_task_handle);
		return;		//Task is already created.
	}
	else
	{
		/* Create the task. */
		xTaskCreate(prv_task_gauges, "APP_GAUGES", 3000 / 4, NULL, 4, &prv_gauges_task_handle);
	}
}

void app_gauges_stop()
{
	vTaskSuspend(prv_gauges_task_handle);
}

static void prv_toggle_data_logging_cb(lv_event_t* e)
{
	/* If were not recording... */
	if (!data_logger_recording(&prv_data_logger_handle))
	{
		/* Start a data logging task based on the params in the active_param array. */
		data_logger_set_data(&prv_data_logger_handle, active_param);

		/*Get the data logging rate. */
		char data_logging_rate_str[25];
		char* data_log_rate_val_str;
		uint32_t data_log_rate_val_uint = 0;
		file_mngr_get_config_data("DATA LOG RATE", data_logging_rate_str);
		data_log_rate_val_str = file_mngr_csv_split(data_logging_rate_str, 1);
		data_log_rate_val_uint = strtoul(data_log_rate_val_str, NULL, 10);
		data_logger_set_period(&prv_data_logger_handle, data_log_rate_val_uint);

		data_logger_set_error_cb(&prv_data_logger_handle, prv_data_logger_error_cb);
		bool rtn_val = data_logger_start_recording(&prv_data_logger_handle);

		if (rtn_val == false)
		{
			return;
		}
		/* Create the recording label and LED. */
		lv_obj_t* rec_container = lv_obj_create(lv_layer_top());
		lv_obj_set_size(rec_container, 130, 60);
		lv_obj_set_style_bg_color(rec_container, UI_COLOR_BLACK, LV_STATE_DEFAULT);
		lv_obj_set_style_bg_opa(rec_container, 127, LV_STATE_DEFAULT);
		lv_obj_align(rec_container, LV_ALIGN_TOP_MID, 0, 0);
		lv_obj_set_style_border_width(rec_container, 0, LV_STATE_DEFAULT);
		lv_obj_set_style_radius(rec_container, 0, LV_STATE_DEFAULT);
		lv_obj_clear_flag(rec_container, LV_OBJ_FLAG_SCROLLABLE);
		lv_obj_set_scrollbar_mode(rec_container, LV_SCROLLBAR_MODE_OFF);

		lv_obj_t* lbl = lv_label_create(rec_container);
		lv_label_set_text(lbl, "Rec.");
		lv_obj_set_style_text_font(lbl, &lv_font_montserrat_20, LV_STATE_DEFAULT);
		lv_obj_align(lbl, LV_ALIGN_LEFT_MID, 0, 0);
		lv_obj_set_style_text_color(lbl, UI_COLOR_WHITE, LV_STATE_DEFAULT);

		lv_obj_t* led = lv_led_create(rec_container);
		lv_obj_set_size(led, 6, 6);
		lv_led_set_color(led, UI_COLOR_RED);
		lv_led_set_brightness(led, LV_LED_BRIGHT_MAX);
		lv_obj_align(led, LV_ALIGN_RIGHT_MID, 0, 0);
	}
	else
	{
		data_logger_stop_recording(&prv_data_logger_handle);
		lv_obj_clean(lv_layer_top());
	}

}

static void prv_data_logger_error_cb(data_logger_error_code_t code)
{
	lv_port_take_lvgl_mutex(1000);

	lv_obj_clean(lv_layer_top());		//Get rid of the recording tag on the screen.

	if (code == DATA_LOGGER_ERROR_NO_MEM)
	{
		lv_obj_t* msgbox = ui_helpers_show_msgbox("Logger out of memory.", NULL, NULL);
		ui_helpers_add_msgbox_close_btn(msgbox, NULL);
	}
	else if (code == DATA_LOGGER_ERROR_FILE_SYS_ERR)
	{
		lv_obj_t* msgbox = ui_helpers_show_msgbox("Logger file system error.", NULL, NULL);
		ui_helpers_add_msgbox_close_btn(msgbox, NULL);
	}
	else if (code == DATA_LOGGER_ERROR_HEAP_ERR)
	{
		lv_obj_t* msgbox = ui_helpers_show_msgbox("Logger internal memory error.", NULL, NULL);
		ui_helpers_add_msgbox_close_btn(msgbox, NULL);
	}
	lv_port_give_lvgl_mutex();
}

static void prv_low_power_mode_cb()
{
	app_gauges_stop();
	can_uds_stop(pdMS_TO_TICKS(1000));
	data_logger_stop_recording(&prv_data_logger_handle);
	while (data_logger_recording(&prv_data_logger_handle)) {}	//Do nothing while we wait for the handle to finish its recording.

}

