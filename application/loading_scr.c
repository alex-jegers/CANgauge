/**********     INCLUDES        **********/
#include "loading_scr.h"
#include "applications.h"
#include "ui/ui_gauges.h"
#include "ui/ui_settings.h"
#include "ui/ui_graph.h"
#include "ui/ui_loading_scr.h"
#include "gauges.h"
#include "cangauge.h"   //All the kernel files.

/**********     TYPEDEFS         **********/

/**********     DEFINES         **********/
 #define PRV_CG_OBD_INIT_TIMER_DURATION_MS          3000


/**********		EXTERNAL VARIABLE DEFINITIONS		**********/

/**********		STATIC VARIABLES		**********/
static TaskHandle_t prv_cg_obd_init_task_handle = NULL;
static TimerHandle_t prv_cg_obd_init_timer = NULL;

/**********		STATIC FUNCTION DECLRATIONS		**********/
static void prv_brightness_slider_handler(lv_event_t* e);	//Handler for the brightness slider being changed.
static void prv_update_settings_from_eeprom();				//Updates the brightness slider position and unit drop down boxes with the values saved in eeprom.
static void prv_save_settings_lvgl_cb(lv_event_t* e);       //Saves all the config data to the config file, TODO: make this a background task that can execute outside of the LVGL handler bc it's slow.
static void prv_data_trsnf_btn_handler(lv_event_t* e);
static void prv_auto_off_th_btn_handler(lv_event_t* e);		//Handler for the auto off threshold button.
static void prv_auto_on_th_btn_handler(lv_event_t* e);		//Handler for the auto on threshold button.
static void prv_save_on_off_thresholds_to_eeprom();			//What the function says.
static void prv_refresh_btn_cb(lv_event_t* e);			    //Handler for the refresh button being pressed. Just triggers a SW reset.
static void prv_restore_defaults_btn_cb(lv_event_t* e);	    //Handler for the restore defaults button being pressed. Write the default VIN file and system info.
static void prv_numberpad_closed_cb(lv_event_t* e);         //Gets the value that is in the data log rate txt box and saves it to the config file.
static void prv_low_power_mode_cb();                        //Call back for when the device is entering low power mode.
static void prv_update_units();							    //Updates the units for the gauges based on what's in the config file.
static void prv_cg_obd_init_timer_cb(TimerHandle_t timer);  //Just a 3ish second timer to make sure the loading screen has been displayed for long enough.
/**********		STATIC FUNCTION DEFINITIONS		**********/
static void prv_cg_obd_init_task(void* args)
{
	//Starts a timer so the loading screen stays on for a time of PRV_CG_OBD_INIT_TIMER_DURATION_MS even if the init function finishes first.
	prv_cg_obd_init_timer = xTimerCreate("CG_OBD_INIT", pdMS_TO_TICKS(PRV_CG_OBD_INIT_TIMER_DURATION_MS), pdFALSE, NULL, prv_cg_obd_init_timer_cb);
	if (prv_cg_obd_init_timer != NULL)
	{
		xTimerStart(prv_cg_obd_init_timer, 0);
	}

    /* Try to initialize the EEPROM file system,
        if it doesnt exist try to create it.
     */
    if (sys_mem_init_eeprom_fs() != FR_OK)
	{
		if (sys_mem_create_eeprom_fs() != FR_OK)
		{
			//EEPROM error.
		}
	}

    /* Check if the config file exists, if it
        doesn't, create one.
    */
	if (file_mngr_config_file_exists() != FR_OK)
	{
		if (file_mngr_create_default_config_file() != FR_OK)
		{
			//EEPROM error.
		}
	}

    /* Notify file manager to run once to update the file list. */
	file_mngr_notify();

    /* Set all the miscellaneous UI interaction callbacks. */
    assert(lv_port_take_lvgl_mutex(portMAX_DELAY));
    ui_set_brightness_slider_event_cb(prv_brightness_slider_handler);			//The brightness slider is changed (change the screen brightness).
    ui_set_settings_scr_load_event_cb(prv_update_settings_from_eeprom);			//The settings screen is loaded (recall the screen brightness value and demo mode status).
    ui_set_save_settings_cb(prv_save_settings_lvgl_cb);							//Start the gauges and CAN tasks again.
    ui_set_settings_data_trnsf_btn_event_cb(prv_data_trsnf_btn_handler);		//Connect the EEPROM file system to USB.
    ui_add_refresh_btn_event_cb(prv_refresh_btn_cb);							//The refresh button is pressed (restart the CAN connection.
    ui_settings_set_restore_defaults_btn_event_cb(prv_restore_defaults_btn_cb);	//Restore defaults button pressed.
    ui_add_settings_firmware_update_btn_event_cb(btldr_load);					//Update firmware button callback.
    ui_settings_set_numberpad_closed_cb(prv_numberpad_closed_cb);
    ui_settings_set_auto_off_th_btn_cb(prv_auto_off_th_btn_handler);
    ui_settings_set_auto_on_th_btn_cb(prv_auto_on_th_btn_handler);
    lv_port_give_lvgl_mutex();

    /* Set the low power mode callback. This will run before the device enters low power mode. */
    pwr_monitor_add_low_pwr_mode_cb(prv_low_power_mode_cb);

    /* Updates the brightness slider, units dropdown boxes, on/off thresholds, data logging period textbox, etc. */
    prv_update_settings_from_eeprom();

    prv_update_units();
    
    /* Wait for the timer to expire and notify the task then switch to the gauges screen. */
    ulTaskNotifyTakeIndexed(0, true, PRV_CG_OBD_INIT_TIMER_DURATION_MS);
    ui_gauges_load();
    app_gauges_run();
    vTaskDelete(NULL);
}

static void prv_cg_obd_init_timer_cb(TimerHandle_t timer)
{
	xTaskNotifyGiveIndexed(prv_cg_obd_init_task_handle, 0);
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

static void prv_update_settings_from_eeprom()
{
	char str_buffer[FILE_MNGR_LONGEST_CONFIG_STR_LEN];

	/* Read and set the LCD backlight value. */
	file_mngr_get_config_data("BRIGHTNESS", str_buffer);
	char* split = file_mngr_csv_split(str_buffer, 1);
	uint32_t backlight_int = atoi(split);
	timer_set_pwm_duty_cycle(TIM12, backlight_int, 1);
	uint32_t timer_val = timer_get_pwm_duty_cycle(TIM12, 1);
	uint32_t slider_val = (timer_val - 5000) / 605;
	ui_settings_set_brightness_slider_value(slider_val);

	/* Set the units dropdowns. */
	char* units;
	file_mngr_get_config_data("PRESSURE UNITS", str_buffer);
	units = file_mngr_csv_split(str_buffer, 1);
	ui_settings_set_pressure_units_dropdown(units);

	file_mngr_get_config_data("TEMPERATURE UNITS", str_buffer);
	units = file_mngr_csv_split(str_buffer, 1);
	ui_settings_set_temperature_units_dropdown(units);

	file_mngr_get_config_data("SPEED UNITS", str_buffer);
	units = file_mngr_csv_split(str_buffer, 1);
	ui_settings_set_speed_units_dropdown(units);

	file_mngr_get_config_data("TORQUE UNITS", str_buffer);
	units = file_mngr_csv_split(str_buffer, 1);
	ui_settings_set_torque_units_dropdown(units);

	/*Get the data logging rate. */
	char* data_log_rate_val_str;
	uint32_t data_log_rate_val_uint = 0;
	file_mngr_get_config_data("DATA LOG RATE", str_buffer);
	data_log_rate_val_str = file_mngr_csv_split(str_buffer, 1);
	data_log_rate_val_uint = strtoul(data_log_rate_val_str, NULL, 10);
	ui_settings_set_data_logger_rate(data_log_rate_val_uint);

	/* Get the on/off thresholds. */
	float on_th, off_th;
	file_mngr_get_config_data("ON THRESHOLD VOLTS", str_buffer);
	char* val_str = file_mngr_csv_split(str_buffer, 1);
	on_th = atof(val_str);
	file_mngr_get_config_data("OFF THRESHOLD VOLTS", str_buffer);
	val_str = file_mngr_csv_split(str_buffer, 1);
	off_th = atof(val_str);
	if ((on_th == 0) || (off_th == 0))
	{
		pwr_monitor_set_auto_on_off_th(12.33, 13.34);
		ui_settings_set_auto_on_off_values(12.34, 13.34);
	}
	else
	{
		pwr_monitor_set_auto_on_off_th(off_th, on_th);
		ui_settings_set_auto_on_off_values(off_th, on_th);
	}

}

static void prv_save_settings_lvgl_cb(lv_event_t* e)
{
	/* Write the backlight brigthness data to the config file. */
	uint32_t timer_val = timer_get_pwm_duty_cycle(TIM12, 1);
    char config_str[28];
    sprintf(config_str, "BRIGHTNESS,%lu,\n", timer_val);
    file_mngr_set_config_data(config_str);

    /* Write the units to the config file. */
    char uints_str[7];
    ui_settings_get_pressure_units_dropdown(uints_str);
    sprintf(config_str, "PRESSURE UNITS,%s,\n", uints_str);
    file_mngr_set_config_data(config_str);

    ui_settings_get_temperature_units_dropdown(uints_str);
    sprintf(config_str, "TEMPERATURE UNITS,%s,\n", uints_str);
    file_mngr_set_config_data(config_str);

    ui_settings_get_speed_units_dropdown(uints_str);
    sprintf(config_str, "SPEED UNITS,%s,\n", uints_str);
    file_mngr_set_config_data(config_str);

    ui_settings_get_torque_units_dropdown(uints_str);
    sprintf(config_str, "TORQUE UNITS,%s,\n", uints_str);
    file_mngr_set_config_data(config_str);

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
		pwr_monitor_resume();
	}
	else	//It was the data transfer button.
	{
		lv_obj_t* btn = lv_event_get_target_obj(e);
		lv_obj_t* lbl = lv_obj_get_child(btn, 0);
		pwr_monitor_suspend();
		msg_box = ui_helpers_show_msgbox("Entering mass storage mode.", "Close", prv_data_trsnf_btn_handler);
		usb_connect(USB_FS_EEPROM);
	}

}

static void prv_auto_off_th_btn_handler(lv_event_t* e)
{
	/* Get the current input voltage. */
	float current_input_voltage = pwr_monitor_get_last_conversion_volts();

	/* Get the current HIGH threshold and rewrite the limits to pwr monitor. */
	float current_high_th = pwr_monitor_get_on_th_volts();
	current_input_voltage += 0.25;		//Make the low threshold slightly higher than what the battery voltage actually is rn.
	pwr_monitor_set_auto_on_off_th(current_input_voltage, current_high_th);		//Loop back in the high threshold and set the new low threshold.
	ui_settings_set_auto_on_off_values(pwr_monitor_get_off_th_volts(), pwr_monitor_get_on_th_volts());		//Set them both in the UI.

	prv_save_on_off_thresholds_to_eeprom();		//Save it to the EEPROM.
}

static void prv_auto_on_th_btn_handler(lv_event_t* e)
{
	/* Lets do it like we did in the off th btn handler. */
	/* Get the current input voltage. */
	float current_input_voltage = pwr_monitor_get_last_conversion_volts();

	/* Get the current LOW threshold and rewrite the limits to pwr monitor. */
	float current_low_th = pwr_monitor_get_off_th_volts();
	current_input_voltage -= 0.25;		//Make this a little lower.
	pwr_monitor_set_auto_on_off_th(current_low_th, current_input_voltage);		//Set that shit in the pwr monitor.
	ui_settings_set_auto_on_off_values(pwr_monitor_get_off_th_volts(), pwr_monitor_get_on_th_volts());	//And in the UI.

	prv_save_on_off_thresholds_to_eeprom();		//Save it to the EERPOM.
}

static void prv_save_on_off_thresholds_to_eeprom()
{
	char* str_buf = (char*)malloc(FILE_MNGR_LONGEST_CONFIG_STR_LEN);	//Allocate some memory for the strings.
	if (str_buf == NULL) { return; }									//Quick null check.

	float low_th = pwr_monitor_get_off_th_volts();		//Get the off/low threshold.
	snprintf(str_buf, FILE_MNGR_LONGEST_CONFIG_STR_LEN, "OFF THRESHOLD VOLTS,%.2f,\n", low_th);	//Write that shit to a string to save to the config file.
	file_mngr_set_config_data(str_buf);		//Save that shit to the config file.

	/* Rinse and repeat with the high/on threshold. */
	float high_th = pwr_monitor_get_on_th_volts();		//Get the current threshold from pwr_monitor.
	snprintf(str_buf, FILE_MNGR_LONGEST_CONFIG_STR_LEN, "ON THRESHOLD VOLTS,%.2f,\n", high_th);		//Format that shit into a string.
	file_mngr_set_config_data(str_buf);		//Write that string to the config file.

	free(str_buf);	//Last but not least lets free that string memory.
}

static void prv_refresh_btn_cb(lv_event_t* e)
{
	/* Stop the CAN and gauge tasks and restart them. */
	/* Wait until all the tasks have been stopped. */
	rcc_sw_reset();
}

static void prv_restore_defaults_btn_cb(lv_event_t* e)
{
	FRESULT res1;
	FRESULT res2;
	FRESULT res3;
	res1 = sys_mem_create_eeprom_fs();
	if (res1 != FR_OK)
	{
		error_show_msgbox_from_lvgl_task("Failed to create file system.");
	}
	res1 = file_mngr_create_default_config_file();
	if (res1 != FR_OK)
	{
		lv_obj_t* msg_box = ui_helpers_show_msgbox("Failed to restore config file.", NULL, NULL);
		ui_helpers_add_msgbox_close_btn(msg_box, NULL);
	}
	res2 = file_mngr_create_default_vin_file();
	if (res2 != FR_OK)
	{
		lv_obj_t* msg_box = ui_helpers_show_msgbox("Failed to restore VIN file.", NULL, NULL);
		ui_helpers_add_msgbox_close_btn(msg_box, NULL);
	}
	res3 = f_mkdir("0:/Data Logs/");
	if (res3 != FR_OK)
	{
		error_show_msgbox("Failed to create logs directory.");
	}
	if (res1 == FR_OK && res2 == FR_OK && res3 == FR_OK)
	{
		lv_obj_t* msg_box = ui_helpers_show_msgbox("Default files restored.", NULL, NULL);
		ui_helpers_add_msgbox_close_btn(msg_box, NULL);
	}
}

static void prv_numberpad_closed_cb(lv_event_t* e)
{
	uint32_t data_logging_period = ui_settings_get_data_logger_rate();
    char config_str[25];
    sprintf(config_str, "DATA LOG RATE,%lu,\n", data_logging_period);
    file_mngr_set_config_data(config_str);
}

static void prv_low_power_mode_cb()
{
	can_transmit_stop(pdMS_TO_TICKS(1000));
	file_mngr_stop();
	system_blink_stop();
	lv_port_stop(1000);
}

static void prv_update_units()
{
	char buf[25];
	char* split;
	char pressure_units[4];
	char temperature_units[2];
	char speed_units[4];
	char torque_units[8];

	file_mngr_get_config_data("PRESSURE UNITS", buf);
	split = file_mngr_csv_split(buf, 1);
	strcpy(pressure_units, split);

	file_mngr_get_config_data("TEMPERATURE UNITS", buf);
	split = file_mngr_csv_split(buf, 1);
	strcpy(temperature_units, split);

	file_mngr_get_config_data("SPEED UNITS", buf);
	split = file_mngr_csv_split(buf, 1);
	strcpy(speed_units, split);

	file_mngr_get_config_data("TORQUE UNITS", buf);
	split = file_mngr_csv_split(buf, 1);
	strcpy(torque_units, split);

	/* If the units are something other than the default, update them in the CAN UDS array. */
	if (strcmp(pressure_units, "kPa"))
	{
		can_uds_change_pressure_units(pressure_units);
	}
	if (strcmp(temperature_units, "C"))
	{
		can_uds_change_temperature_units(temperature_units);
	}
	if (strcmp(speed_units, "kph"))
	{
		can_uds_change_speed_units(speed_units);
	}
	if (strcmp(torque_units, "Nm"))
	{
		can_uds_change_torque_units(torque_units);
	}

}

/**********		GLOBAL FUNCTION DEFINITIONS		**********/
void cg_obd_initialize()
{
	/* Load the UI. */
	ui_gauges_init();
	ui_settings_init();
	ui_graph_init();
    ui_loading_scr_init();
    ui_loading_scr_set_text("Check for the latest software updates on our website at\nwww.can-gauge.com");
    ui_loading_scr_load();

    xTaskCreate(prv_cg_obd_init_task, "CG_OBD_INIT", 2000 / 4, NULL, 1, &prv_cg_obd_init_task_handle);
}
