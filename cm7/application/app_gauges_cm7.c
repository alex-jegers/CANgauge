/**********     INCLUDES        **********/
#include "app_gauges_cm7.h"
#include "applications_cm7.h"
#include "application/can_uds_def.h"

#include "ui/ui_gauges.h"
#include "ui/ui_helpers.h"
#include "ui/ui_settings.h"

#include "drivers/drivers.h"

#include "lvgl.h"

#include "lvgl_port/lvgl_port_def.h"
#include "bootloader/bootloader.h"

#include "system/system_cm7.h"

/**********		DEFINES		**********/
#define EVENT_BITS_TASK_STOPPED			0x1 << 0		//Set when the task is stopped.
#define EVENT_BITS_QUERY_TRANSMITTING	0x1 << 1		//Set when the ISO15765 query is transmitting.

/**********		EXTERNAL VARIABLE DEFINITIONS		**********/

/**********		STATIC VARIABLES		**********/
const char* prv_version = VERSION;
const char* prv_build = BUILD_TYPE_STR;
static bool prv_task_run = false;
static TaskHandle_t prv_gauges_task_handle;
static EventGroupHandle_t prv_event_group = NULL;
static saej1979_current_data_t* active_param[4] = { NULL, NULL, NULL, NULL };

/**********		STATIC FUNCTION DECLRATIONS		**********/
static void prv_task_gauges();							//The FreeRTOS task.
static uint8_t prv_load_gauges(const char *str[4], uint8_t num_gauges);		//Sends the gauge data to the UI, must call ui_load_gauge_screen afterwards to display them. Returns the number of gauges submitted that were found on OBD.
static void prv_create_gauge_select_btns();				//Creates the buttons on the GUI.
static void prv_update_units();							//Updates the units for the gauges based on what's in the config file.

static void prv_gauge_event_cb(lv_event_t* e);			//Handler for the gauge itself events.
static void prv_gauge_view_btn_cb(lv_event_t* e);		//Handler for a available being selected.
static void prv_gauge_back_btn_cb(lv_event_t* e);		//Handler for if the back button is pressed.
static void prv_refresh_btn_cb(lv_event_t* e);			//Handler for the refresh button being pressed.

static void prv_brightness_slider_handler(lv_event_t* e);	//Handler for the brightness slider being changed.
static void prv_settings_scr_load_handler(lv_event_t* e);

static void prv_settings_btn_clicked_cb(lv_event_t* e);
static void prv_settings_back_btn_clicked_cb(lv_event_t* e);
static void prv_data_trsnf_btn_handler(lv_event_t* e);			//Handler for trasnfer data btn in the settings menu.

/**********		STATIC FUNCTION DEFINITIONS		**********/
static void prv_task_gauges()
{
	/* Load the UI. */
	ui_gauges_init();
	ui_settings_init();
	ui_gauges_load();

	/* Turn on the CAN peripheral and set the baud rate. */
	can_init(FDCAN1);
	can_set_baud_rate(FDCAN1, CAN_BAUD_500K);
	can_run(FDCAN1);

	/* Start the CAN transmitter task. */
	assert( can_transmit_run(FDCAN1, 15) == pdPASS );

	/* Start the CAN receiver task. */
	assert( app_can_controller_run() == pdPASS );

	/* Set the LVGL event callbacks. */
	ui_gauges_set_gauge_cb(prv_gauge_event_cb);			//A gauge is clicked (go back to selection screen).
	ui_gauges_set_view_btn_cb(prv_gauge_view_btn_cb);		//A gauge is selected (load the gauge and set the CAN getter).
    ui_set_brightness_slider_event_cb(prv_brightness_slider_handler);		//The brightness slider is changed (change the screen brightness).
    ui_set_settings_scr_load_event_cb(prv_settings_scr_load_handler);		//The settings screen is loaded (recall the screen brightness value and demo mode status).
    ui_set_settings_btn_event_cb(prv_settings_btn_clicked_cb);		//Stop the gauges and CAN tasks.
    ui_set_settings_back_btn_event_cb(prv_settings_back_btn_clicked_cb);		//Start the gauges and CAN tasks again.
    ui_set_settings_data_trnsf_btn_event_cb(prv_data_trsnf_btn_handler);	//Connect the EEPROM file system to USB.
    ui_add_refresh_btn_event_cb(prv_refresh_btn_cb);						//The refresh button is pressed (restart the CAN connection.

	/*Change the priority back to 2.*/
	vTaskPrioritySet(NULL, 2);

	/* Wait for the CAN controller to initialize. */
	if (!app_can_controller_is_init( pdMS_TO_TICKS(10000) ) )
	{
		/* Do something if it fails. */
	}

	prv_update_units();

	bool found_car = app_can_controller_get_can_id();

	if (found_car == true)
	{
		prv_create_gauge_select_btns();			//Creates the buttons in the UI.
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
	uint32_t str_size = sprintf(label, "PIDs 0x00: 0x%X\n\
										PIDs 0x20: 0x%X\n\
										PIDs 0x40: 0x%X\n\
										PIDs 0x60: 0x%X\n\
										CAN ID: 0x%X\n\
										RX ECR: %d\n\
										TX ECR: %d\n\
										LEC: %d\n\
										%s - %s",
										avail_pids_1, avail_pids_2, avail_pids_3, avail_pids_4, can_id,
										rx_ecr, tx_ecr, ec, prv_version, prv_build);
	realloc(label, str_size);

	/* Write the diagnostic label to the screen. */
	lv_port_take_lvgl_mutex(portMAX_DELAY);
	ui_helpers_add_text_to_act_scr(label, LV_ALIGN_CENTER, 0, 425);
	lv_port_give_lvgl_mutex();
	free(label);

	/* Check to see if there's a config file with the last state. */
	char* line = calloc(250, 1);
	uint32_t bytes_wr = sys_mem_get_config_data("LAST GAUGES STATE", line);
	if (bytes_wr != 0)
	{
		realloc(line, bytes_wr);
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
	while (prv_task_run)
	{
		/* Check to see if we are transmitting. */
	    uint32_t rtn = xEventGroupWaitBits(prv_event_group, EVENT_BITS_QUERY_TRANSMITTING,    //Bits to wait for.
	                                        pdFALSE,        //Dont clear the bits on exit.
	                                        pdTRUE,         //wait for all the bits (it's only 1)
	                                        pdMS_TO_TICKS(500)); //Block time.
		if ((rtn & EVENT_BITS_QUERY_TRANSMITTING) != 0)
		{
			static float last_value[4];		//Use this to see if the value changed, if it didn't we dont change the gauge value,
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
				uint32_t raw_value = can_controller_get_data(current_pid, first_byte, num_params);

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
				last_value[d] = processed_val;
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

static void prv_create_gauge_select_btns()
{
	for (uint8_t i = 0; i < 176; i++)
	{
		saej1979_current_data_t* y = saej1979_get_current_data(i);
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

static void prv_update_units()
{
	char buf[25];
	char* split;
	char pressure_units[4];
	char temperature_units[2];

	sys_mem_get_config_data("PRESSURE UNITS", buf);
	split = sys_mem_csv_split(buf, 1);
	strcpy(pressure_units, split);
	sys_mem_get_config_data("TEMPERATURE UNITS", buf);
	split = sys_mem_csv_split(buf, 1);
	strcpy(temperature_units, split);

	if (strcmp(pressure_units, "kPa"))
	{
		can_uds_change_pressure_units(pressure_units);
	}
	if (strcmp(temperature_units, "C"))
	{
		can_uds_change_temperature_units(temperature_units);
	}

}

static void prv_gauge_event_cb(lv_event_t* e)
{
	/* Stop transmitting the requestor on CAN. */
	can_uds_stop_query();
	xEventGroupClearBits(prv_event_group, EVENT_BITS_QUERY_TRANSMITTING);

	/* Write to the save state file. */
	const char* str = "LAST GAUGES STATE,0,0,0,0,\n";
	sys_mem_set_config_data(str);
}

static uint8_t prv_load_gauges(const char* str[4], uint8_t num_gauges)
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
			saej1979_current_data_t *x = saej1979_get_current_data(i);

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
	const char* str[4];
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

	sys_mem_set_config_data(save_str);
	free(save_str);

	/* Load the gauges into the UI and set the ISO15675 query on CAN. */
	if (prv_load_gauges(str, num_gauges) == num_gauges)
	{
		ui_load_gauge_screen();
	}
}

static void prv_gauge_back_btn_cb(lv_event_t* e)
{
	/* Wait until all the tasks have been stopped. */
	can_transmit_stop(0);
	app_can_controller_stop(0);
	app_gauges_stop(0);
}

static void prv_refresh_btn_cb(lv_event_t* e)
{
	/* Stop the CAN and gauge tasks and restart them. */
	/* Wait until all the tasks have been stopped. */
	rcc_sw_reset();
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
	xTaskCreate(prv_task_gauges, "APP_GAUGES", 3000 / 4, NULL, 4, prv_gauges_task_handle);
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

static void prv_settings_scr_load_handler(lv_event_t* e)
{
	lv_event_code_t code = lv_event_get_code(e);

	if (code == LV_EVENT_SCREEN_LOADED)
	{
		/* Set the slider value. */
		lv_obj_t** slider = lv_event_get_user_data(e);
		uint32_t timer_val = timer_get_pwm_duty_cycle(TIM12, 1);
		uint32_t slider_val = (timer_val - 5000) / 605;
		lv_slider_set_value(*slider, slider_val, LV_ANIM_OFF);

		/* Set the units dropdowns. */
		char units_config_str[25];
		char* units;
		sys_mem_get_config_data("PRESSURE UNITS", units_config_str);
		units = sys_mem_csv_split(units_config_str, 1);
		ui_settings_set_pressure_units_dropdown(units);

		sys_mem_get_config_data("TEMPERATURE UNITS", units_config_str);
		units = sys_mem_csv_split(units_config_str, 1);
		ui_settings_set_temperature_units_dropdown(units);
	}

}

static void prv_settings_btn_clicked_cb(lv_event_t* e)
{
	/* Wait until all the tasks have been stopped. */
	can_transmit_stop(0);
	app_can_controller_stop(0);
	app_gauges_stop(portMAX_DELAY);
	ui_gauges_delete();


	/* Initialize the boot loader, this sets the function CB
	 * for the update firmware button.
	 */
	btldr_init();
}

static void prv_settings_back_btn_clicked_cb(lv_event_t* e)
{
	/* Write the backlight brigthness data to the config file. */
	uint32_t timer_val = timer_get_pwm_duty_cycle(TIM12, 1);
    char config_str[25];
    sprintf(config_str, "BRIGHTNESS,%lu,\n", timer_val);
    sys_mem_set_config_data(config_str);

    /* Write the units to the config file. */
    char uints_str[5];
    ui_settings_get_pressure_units_dropdown(uints_str);
    sprintf(config_str, "PRESSURE UNITS,%s,\n,", uints_str);
    sys_mem_set_config_data(config_str);

    ui_settings_get_temperature_units_dropdown(uints_str);
    sprintf(config_str, "TEMPERATURE UNITS,%s,\n,", uints_str);
    sys_mem_set_config_data(config_str);


	ui_settings_delete();
	app_gauges_run();
}

static void prv_data_trsnf_btn_handler(lv_event_t* e)
{

	static lv_obj_t* msg_box = NULL;
	if (msg_box != NULL)	//It's closing the message box.
	{
		lv_obj_t* btn = lv_event_get_target(e);
		lv_obj_t* footer = lv_obj_get_parent(btn);
		lv_obj_t* msgbox = lv_obj_get_parent(footer);
		lv_obj_delete(msgbox);
		usb_disconnect();
		msg_box = NULL;
	}
	else	//It was the data transfer button.
	{
		lv_obj_t* btn = lv_event_get_target_obj(e);
		lv_obj_t* lbl = lv_obj_get_child(btn, 0);
		msg_box = ui_helpers_show_msgbox("Entering mass storage mode.", "Close", prv_data_trsnf_btn_handler);
		usb_connect(USB_FS_EEPROM);
	}

}

