/**********     INCLUDES        **********/
#include "file_mngr.h"
#include "application/can_uds.h"		//TODO: Change how file_mngr_save_vin_to_file works so i dont have to include this.
#include "ui/ui_graph.h"

/**********     TYPEDEFS         **********/

/**********		DEFINES		**********/
#define PRV_DATA_LOGS_DIR_PATH		"0:/Data Logs/"
/**********		EXTERNAL VARIABLE DEFINITIONS		**********/

/**********		STATIC VARIABLES		**********/
static TaskHandle_t prv_file_mngr_task_handle;
static const char* prv_file_name_to_delete;
static const char* prv_file_name_to_display;
static const char* prv_displayed_file;
static lv_obj_t* prv_loading_wheel;
static const char* const prv_vin_info_file_path = "0:/VIN Info.csv/";
static const char* const prv_vin_file_header = "VIN,Srvc0x1 PID0x00,Srvc0x1 PID0x20,Srvc0x1 PID0x40,Srvc0x1 PID0x60,Srvc0x1 PID0x80,Srvc0x1 PID0xA0,Srvc0x9 PID0x00,Srvc0x9 PID0x20,Srvc0x9 PID0x40,Srvc0x9 PID0x60,\n\0";


/**********		STATIC FUNCTION DECLRATIONS		**********/
/* Task functions. */
static void prv_file_mngr_task(void* args);
static void prv_update_files_list();					//Makes sure the directory and UI list are matching.
static void prv_delete_file();							//Checks if prv_file_name_to_delete is NULL and deletes the file if it's not.
static void prv_display_data();							//Reads the data from the selected file and puts in on the graph.

static uint32_t prv_get_number_of_log_files();

/* LVGL callbacks. */
static void prv_file_selected_cb(lv_event_t* e);
static void prv_delete_btn_cb(lv_event_t* e);			//Handler for the delete button on the graph page.

/**
 * prv_convert_csv_to_array
 * 		desc: converts a column from a CSV file to a C array of floats so it can
 * 			be displayed on a LVGL chart.
 * 		params:
 * 			file_path: the file path of the CSV.
 * 			column: the number of the column in the CSV to convert to an array (0 indexed).
 * 			output: the pointer to the array that the function creates (the function will allocate the memory).
 * 		returns:
 * 			the number of elements in the array.
 */
static uint32_t prv_convert_csv_to_array(const char* file_path, uint8_t column, float** output);

/**********		STATIC FUNCTION DEFINITIONS		**********/
static void prv_file_mngr_task(void* args)
{
	/* Set the UI callbacks. */
	if (lv_port_take_lvgl_mutex(portMAX_DELAY) != pdPASS)
	{
		vTaskDelete(NULL);
	}

	ui_graph_set_delete_btn_cb(prv_delete_btn_cb);
	ui_graph_set_file_list_event_cb(prv_file_selected_cb);
	lv_port_give_lvgl_mutex();

	while (1)
	{
		/**
		 * 1. Compare the files in the data logs folder against the items in the list
		 * 		and remove items from the list that are no longer in the folder and
		 * 		add items in the folder that are not in the list.
		 */
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);			//Task will run when notified.
		prv_delete_file();
		prv_update_files_list();
		prv_display_data();
	}
}

static void prv_update_files_list()
{
	FRESULT res;
	DIR dir;
	FILINFO dir_file_info;
	res = f_opendir(&dir, PRV_DATA_LOGS_DIR_PATH);
	if (res != FR_OK)
	{
		return;
	}

	if (lv_port_take_lvgl_mutex(5000) != pdPASS)
	{
		f_closedir(&dir);
		return;
	}

	/* Check how many files are in the list box. */
	uint32_t num_files_in_list =  ui_graph_get_number_of_list_items();
	uint32_t num_files_in_dir = prv_get_number_of_log_files();

	/* Both lists have no files in them, were done and can return. */
	if ((num_files_in_list == 0) && (num_files_in_dir == 0))
	{
		f_closedir(&dir);
		lv_port_give_lvgl_mutex();
		return;
	}

	/* The directory has none but the UI list has some, delete all from the UI and rtn. */
	if (num_files_in_dir == 0)
	{
		for (uint32_t i = 0; i < num_files_in_list; i++)
		{
			ui_graph_delete_file_from_list(i);
		}
		f_closedir(&dir);
		lv_port_give_lvgl_mutex();
		return;
	}

	/* Go through all the files in the directory and make sure they've been added. */
	bool already_in_list = false;
	char* dir_file_name;
	for(uint32_t x = 0; x < num_files_in_dir; x++)
	{
		/* Get an item from the firmware folder. */
		f_readdir(&dir, &dir_file_info);
		dir_file_name = dir_file_info.fname;
		already_in_list = false;

		/* Go through the list items. */
		for (uint32_t i = 0; i < num_files_in_list; i++)
		{
			const char* list_file_name = ui_graph_get_file_list_item(i);		//File name from the UI list.
			if (strcmp(list_file_name, dir_file_name) == 0)
			{
				already_in_list = true;		//This dir file is already in the UI list, go to the next dir file.
				break;
			}
		}
		if (already_in_list == false)
		{
			ui_graph_add_file_to_list(dir_file_name);
			num_files_in_list++;		//Make a copy of this or smth so it doesnt cause more than necessary loops in the next step.
		}
	}

	f_closedir(&dir);

	/* Go through all the file names in the list and make sure none of them have been deleted. */
	for (int32_t i = 0; i < num_files_in_list; i++)
	{
		/* Create a couple fresh directory objects. */
		DIR temp_dir;
		FILINFO list_file_info;

		/* Get a file name from the list. */
		const char* list_file_name = ui_graph_get_file_list_item(i);

		/* Check if the directory for the file name from the list. */
		f_findfirst(&temp_dir, &list_file_info, PRV_DATA_LOGS_DIR_PATH, list_file_name);
		if (strlen(list_file_info.fname) == 0)		//If the file name strlen is zero the file is not in the directory, delete it.
		{
			ui_graph_delete_file_from_list(i);
			i--;
			num_files_in_list--;
		}
		f_closedir(&temp_dir);
	}
	lv_port_give_lvgl_mutex();
}

static void prv_delete_file()
{
	/* If the file name to delete is NULL there isn't anything to delete rn. */
	if (prv_file_name_to_delete == NULL)
	{
		return;
	}
	vTaskDelay(1000);		//This is just for aesthetics so the spinny wheel actually has enough time to be visible.
	uint32_t file_name_length = strlen(prv_file_name_to_delete);
	uint32_t dir_path_length = strlen(PRV_DATA_LOGS_DIR_PATH);
	char* file_path = (char*)malloc(file_name_length + dir_path_length + 1);
	FRESULT res = FR_DISK_ERR;
	if (file_path != NULL)
	{
		snprintf(file_path, file_name_length + dir_path_length + 1, "%s%s", PRV_DATA_LOGS_DIR_PATH, prv_file_name_to_delete);
		res = f_unlink(file_path);
		free(file_path);
	}

	if (res != FR_OK)
	{
		error_show_msgbox("Unable to delete file.");
	}
	if (lv_port_take_lvgl_mutex(5000) == pdPASS)
	{
		lv_obj_delete(prv_loading_wheel);
		lv_port_give_lvgl_mutex();
	}
	prv_file_name_to_delete = NULL;
	prv_file_name_to_display = NULL;
}

static void prv_display_data()
{
	/* If the file to display is NULL or if the file to display is the one that's already displayed, return. */
	if ((prv_file_name_to_display == NULL) || (prv_file_name_to_display == prv_displayed_file))
	{
		return;
	}

	if (lv_port_take_lvgl_mutex(500) != pdPASS)
	{
		return;
	}
	ui_graph_clear_all_chart_data();
	lv_port_give_lvgl_mutex();

	uint32_t file_name_length = strlen(prv_file_name_to_display);
	uint32_t dir_path_length = strlen(PRV_DATA_LOGS_DIR_PATH);
	char* file_path = (char*)malloc(file_name_length + dir_path_length + 1);
	snprintf(file_path, file_name_length + dir_path_length + 1, "%s%s", PRV_DATA_LOGS_DIR_PATH, prv_file_name_to_display);
    FIL file;
    FRESULT res = f_open(&file, file_path, FA_READ);
    char* line = (char*)malloc(400);					//There's no way each column name is more than 100 characters but still TODO: make this so it allocates more memory if the gets call fails.
    line = f_gets(line, 400, &file);					//line holds the header.
	char* line_cpy = (char*)malloc(strlen(line) + 1);	//Make a copy of line for the following strtok_r procedures.
    uint8_t num_cols = file_mngr_csv_get_num_cols(line);

    /* Start at 1 bc the first column should always just be "Time (ms)". */
    const lv_color_t color_lut[4] = { UI_COLOR_RED, UI_COLOR_BLUE, UI_COLOR_WHITE, UI_COLOR_LIGHT_RED };
    float* data_arr;		//*data_arr has to be freed after prv_convert_csv_to_array.
    uint32_t num_rows;
    /* Set the time base. */
    num_rows = prv_convert_csv_to_array(file_path, 0, &data_arr);
    if (num_rows == 0)
    {
        free(line);
        free(line_cpy);
        free(file_path);
    	return;
    }
    uint32_t max_time_ms = data_arr[num_rows - 1];
    uint32_t increment_ms = max_time_ms / num_rows;
    ui_graph_set_timebase(max_time_ms, increment_ms);

    for (uint8_t i = 1; i < num_cols; i++)
    {
    	strcpy(line_cpy, line);
    	char* series_label = file_mngr_csv_split(line_cpy, i);

    	num_rows = prv_convert_csv_to_array(file_path, i, &data_arr);
	    if (lv_port_take_lvgl_mutex(500) == pdPASS)
	    {
	    	ui_graph_add_series_data(data_arr, num_rows, series_label, color_lut[i - 1]);
	    	lv_port_give_lvgl_mutex();
	    }
		free(data_arr);
    }


    prv_displayed_file = prv_file_name_to_display;
    free(line);
    free(line_cpy);
    free(file_path);
}

static uint32_t prv_get_number_of_log_files()
{
	FRESULT res;
	DIR dir;
	FILINFO dir_file_info;
	uint32_t rtn_val = 0;
	res = f_opendir(&dir, PRV_DATA_LOGS_DIR_PATH);
	if (res != FR_OK) { return rtn_val; }

	f_readdir(&dir, &dir_file_info);			// Read a line.
	while (strlen(dir_file_info.fname) != 0)	// If it's good increment the counter and read another.
	{
		rtn_val++;
		f_readdir(&dir, &dir_file_info);
	}
	f_closedir(&dir);
	return rtn_val;
}

static void prv_file_selected_cb(lv_event_t* e)
{
	prv_file_name_to_display = ui_graph_get_selected_file();
	xTaskNotifyGive(prv_file_mngr_task_handle);					//Notify the task so it can update the graph.
}

static void prv_delete_btn_cb(lv_event_t* e)
{
	prv_file_name_to_delete = ui_graph_get_selected_file();		//Get the selected file name.
	prv_loading_wheel = ui_helpers_show_loading_wheel(lv_layer_top(), "Deleting file...");	//Dont need lv_port mutex bc this is running in LVGL timer handler.
	xTaskNotifyGive(prv_file_mngr_task_handle);					//Notify the task so it can update the list and delete the file.
}

static uint32_t prv_convert_csv_to_array(const char* file_path, uint8_t column, float** output)
{
	/* Do a NULL check. */
    if (file_path == NULL || output == NULL)
    {
        return 0;
    }

	*output = NULL;
    float* data_arr;

    /* Try to open the file, return 0 if we cant. */
    FIL file;
    FRESULT res = f_open(&file, file_path, FA_READ);
    if (res != FR_OK)
    {
    	return 0;
    }

    uint32_t line_len = 200;								//Start with room for a 200 character string.
    char* line = (char*)malloc(line_len);
	if (line == NULL)
	{
		//TODO: Something.
		f_close(&file);
		return 0;
	}
    uint32_t data_len = 200;								//Start with 200 floats of data for the data output.
    data_arr = (float*)malloc(200 * sizeof(float));
	if (data_arr == NULL)
	{
		f_close(&file);
		free(line);
		return 0;
	}
    uint32_t row = 0;
    while (1)
    {
    	char* gets_res = f_gets(line, line_len, &file);		//Read a line from the file into 'line'.
    	if (gets_res == NULL)
    	{
			//TODO: Check why f_gets would fail and maybe change the behavior here.
			free(line);
			free(data_arr);
			f_close(&file);
			return 0;
    	}

    	char* val_str = file_mngr_csv_split(line, column);	//Get the data from the column requested.
		if (val_str == NULL)
		{
			free(line);
			free(data_arr);
			f_close(&file);
			return 0;	
		}
    	char* end_ptr;
    	float val_f = strtof(val_str, &end_ptr);			//Convert the string to a float.
    	/* If there were no numbers found, go to the next. It was probably the header line that we read. */
    	if (end_ptr == val_str)
    	{
    		continue;
    	}

    	data_arr[row] = val_f;								//Write the data to the output array.
    	row++;												//Increment row counter.
    	/* Check if row exceeds the amount of memory we've allocated for the data array. */
    	if (row >= data_len)
    	{
    		/* Allocate more memory. */
			if (data_len > 0x3FFFFFF)		//Check if were overflowing a uint32_t.
			{
				//TODO: Something.
				free(line);
				free(data_arr);
				f_close(&file);
				return 0;
			}
    		float* new_data_arr = realloc(data_arr, (data_len + 100) * sizeof(float));
    		if (new_data_arr == NULL)
    		{
    			//TODO: Something.
				free(line);
				free(data_arr);
				f_close(&file);
				return 0;
    		}
    		/* Free the old and reassign pointers. */
    		data_arr = new_data_arr;
    		data_len += 100;								//Keep track of how much memory is allocated.
    	}
    	if (f_eof(&file) != 0)
    	{
			f_close(&file);
			free(line);
    		*output = data_arr;
    		return row;
    	}
    }
}

/**********		GLOBAL FUNCTION DEFINITIONS		**********/
bool file_mngr_run()
{
	bool rtn_val = xTaskCreate(prv_file_mngr_task, "FILE_MNGR", 4000 / 4, NULL, 1, &prv_file_mngr_task_handle);
	return rtn_val;
}

bool file_mngr_stop()
{
	/* TODO: Make this cleaner. Or ensure that nothing gets left dangling. */
	vTaskSuspend(prv_file_mngr_task_handle);
	return true;
}

void file_mngr_notify()
{
	xTaskNotifyGive(prv_file_mngr_task_handle);
}

uint32_t file_mngr_get_config_data(const char* data_to_get, char* data_buf)
{
	uint32_t rtn_val = 0;
	/* Try to open the config file. */
	FIL config_file;
	FRESULT res;
	res = f_open(&config_file, FILE_MNGR_CONFIG_FILE_PATH, FA_READ | FA_WRITE);
	if (res == FR_OK) //The file exists.
	{
		char* line = calloc(250, 1);
		char* line_copy = calloc(250, 1);
		while (f_eof(&config_file) == 0)
		{
			/* Read a line. */
			f_gets(line, 250, &config_file);
			strcpy(line_copy, line);

			/* Split it with ",". */
			char* split;		//Hold the strings from the config file.
			char* sv_ptr;		//For strtok_r.
			split = strtok_r(line_copy, ",", &sv_ptr);

			/* Check if it's a match. */
			if (strcmp(split, data_to_get) == 0)
			{
				/* Copy the line to data_buf. */
				uint32_t line_str_len = strlen(line);
				memcpy(data_buf, line, line_str_len);
				rtn_val = line_str_len;
				break;
			}
		}
		free(line);
		free(line_copy);
		f_close(&config_file);
	}
	if (rtn_val == 0)
	{
		//Do something if the file fails to open.
		file_mngr_create_default_config_file();
	}
	return rtn_val;
}

void file_mngr_set_config_data(char* data)
{
	FIL current_file;
	FIL new_file;
	FRESULT res;
	char* data_copy = calloc(250, 1);		//Make a copy of the data were writing to figure out what the header is.
	strcpy(data_copy, data);

	/* Split it with ",". */
	char* data_header;	//Hold the strings from the config file.
	char* sv_ptr;		//For strtok_r.a
	data_header = strtok_r(data_copy, ",", &sv_ptr);

	/* Figure out what the index of this header is, if it even exists. */
	uint32_t line_counter = 0;
	char* line = calloc(250, 1);
	res = f_open(&current_file, FILE_MNGR_CONFIG_FILE_PATH, FA_READ | FA_WRITE);
	while (f_eof(&current_file) == 0)
	{
		/* Read a line. */
		f_gets(line, 250, &current_file);

		/* Split it with ",". */
		char* split;		//Hold the strings from the config file.
		char* sv_ptr;		//For strtok_r.
		split = strtok_r(line, ",", &sv_ptr);

		/* Check if it's a match. */
		if (strcmp(split, data_header) == 0)
		{
			break;
		}
		line_counter++;
	}

	uint32_t line_to_skip = line_counter;
	line_counter = 0;

	/* Now copy everything from the old file to a new file except for the line we're overwriting. */
	res = f_open(&new_file, "0:/temp", FA_WRITE | FA_CREATE_ALWAYS);
	f_lseek(&current_file, 0);
	while (f_eof(&current_file) == 0)
	{
		f_gets(line, 250, &current_file);
		uint32_t line_len = strlen(line);
		if (line_counter != line_to_skip && (line_len > 1))
		{
			f_puts(line, &new_file);
		}
		line_counter++;
	}
	/* Add the new data line to the end of the new file. */
	f_putc('\n', &new_file);
	f_puts(data, &new_file);

	f_close(&current_file);
	f_close(&new_file);
	f_unlink(FILE_MNGR_CONFIG_FILE_PATH);				//Unlink the current config file.
	f_rename("0:/temp", FILE_MNGR_CONFIG_FILE_PATH);	//Remane the temp file as the new config file.

	free(data_copy);
	free(line);
}

FRESULT file_mngr_create_default_config_file()
{
	FIL config_file;
	FRESULT res;
	res = f_unlink(FILE_MNGR_CONFIG_FILE_PATH);		//Unlink the old one incase it's still there.
	res = f_open(&config_file, FILE_MNGR_CONFIG_FILE_PATH, FA_CREATE_ALWAYS | FA_WRITE);
	if (res != FR_OK) { return res; }

	const char* const config_str = "LAST GAUGES STATE,0,0,0,0,\n"
									"BRIGHTNESS,65535,\n"
									"PRESSURE UNITS,kPa,\n"
									"TEMPERATURE UNITS,C,\n"
									"SPEED UNITS,kph,\n"
									"TORQUE UNITS,Nm\n"
									"DATA LOG RATE,100,\n";
	uint32_t len = strlen(config_str);
	uint32_t bw = 0;
	res = f_write(&config_file, config_str, (UINT)len, (UINT*)&bw);
	f_close(&config_file);
	return res;
}

FRESULT file_mngr_create_default_vin_file()
{
	FIL vin_file;
	FRESULT res;
	f_unlink(prv_vin_info_file_path);		//Unlink the old one incase it's still there.
	res = f_open(&vin_file, prv_vin_info_file_path, FA_CREATE_ALWAYS | FA_WRITE);
	if (res != FR_OK) { return res; }

	uint32_t len = strlen(prv_vin_file_header);
	uint32_t bw = 0;
	res = f_write(&vin_file, prv_vin_file_header, (UINT)len, (UINT*)&bw);
	f_close(&vin_file);
	return res;
}

FRESULT file_mngr_save_vin_to_file(char* vin)
{
	if (vin == NULL)
	{
		return FR_DENIED;
	}
	/* Check to see if the VIN file exists or not. */
	FIL vin_file;
	FRESULT res;
	res = f_open(&vin_file, prv_vin_info_file_path, FA_READ | FA_WRITE);
	/* If the file doesnt exist... */
	if (res != FR_OK)
	{
		/* If the file doesnt exist, create a new one. */
		f_close(&vin_file);
		res = file_mngr_create_default_vin_file();
		res = f_open(&vin_file, prv_vin_info_file_path, FA_READ | FA_WRITE);
		if (res != FR_OK)
		{
			assert( lv_port_take_lvgl_mutex(500) );
			lv_obj_t* msg_box = ui_helpers_show_msgbox("Failed to create new VIN file.", NULL, NULL);
			ui_helpers_add_msgbox_close_btn(msg_box, NULL);
			lv_port_give_lvgl_mutex();
			return res;
		}
		else
		{
			assert( lv_port_take_lvgl_mutex(500) );
			lv_obj_t* msg_box = ui_helpers_show_msgbox("Created new VIN file.", NULL, NULL);
			ui_helpers_add_msgbox_close_btn(msg_box, NULL);
			lv_port_give_lvgl_mutex();
		}
	}
	/* If the file does already exist. */
	/* Check to see if this VIN is already saved. */
	f_lseek(&vin_file, 0);			//Move pointer to start of file.
	const uint32_t alloc_length = 250;
	char* line = calloc(alloc_length, 1);		//Allocate memory to read from the file.
	if (line == NULL) { rcc_sw_reset(); }
	/* Read the header line (make sure the header is correct and that the file is valid). */
	f_gets(line, alloc_length, &vin_file);
	if (strcmp(line, prv_vin_file_header) != 0)
	{
		assert( lv_port_take_lvgl_mutex(500) );
		lv_obj_t* msg_box = ui_helpers_show_msgbox("VIN file header corrupt.", NULL, NULL);
		ui_helpers_add_msgbox_close_btn(msg_box, NULL);
		lv_port_give_lvgl_mutex();
		f_lseek(&vin_file, 0);
		f_truncate(&vin_file);
		int val = f_puts(prv_vin_file_header, &vin_file);			//Write the header.
		if (val != strlen(prv_vin_file_header))
		{
			assert( lv_port_take_lvgl_mutex(500) );
			lv_obj_t* msg_box = ui_helpers_show_msgbox("Failed to write header to VIN file.", NULL, NULL);
			ui_helpers_add_msgbox_close_btn(msg_box, NULL);
			lv_port_give_lvgl_mutex();
			return FR_DISK_ERR;
		}

	}

	/* Start reading the VINs and see if this one has been saved yet. */
	while (f_eof(&vin_file) != true)
	{
		f_gets(line, alloc_length, &vin_file);		//Read a VIN line.
		char* split = NULL;
		char* sv_ptr = NULL;
		split = strtok_r(line,",", &sv_ptr);	//Split the string with "," to get the VIN.
		if (strlen(split) != 17)
		{
			f_unlink(prv_vin_info_file_path);
			assert( lv_port_take_lvgl_mutex(500) );
			lv_obj_t* msg_box = ui_helpers_show_msgbox("VIN file corrupt.", NULL, NULL);
			ui_helpers_add_msgbox_close_btn(msg_box, NULL);
			lv_port_give_lvgl_mutex();
			return FR_DISK_ERR;
		}
		if (strcmp(split, vin) == 0)
		{
			free(line);
			return FR_OK;//This VIN is already saved.
		}
	}
	free(line);
	/* If we reached this point, this VIN is not saved and the file pointer is
	 * at the end of the file so we can save this VIN and data. */
	if (strlen(vin) != 17)
	{
		assert( lv_port_take_lvgl_mutex(500) );
		lv_obj_t* msg_box = ui_helpers_show_msgbox("No VIN found. Nothing to save.", NULL, NULL);
		ui_helpers_add_msgbox_close_btn(msg_box, NULL);
		lv_port_give_lvgl_mutex();
		return FR_DISK_ERR;
	}
	unsigned int avail_pids_current_data[6];
	unsigned int avail_pids_info_type[4];
	char* save_str = calloc(alloc_length, 1);
	for (uint8_t i = 0; i < 6; i++)
	{
		avail_pids_current_data[i] = can_uds_get_raw_current_data(0x20 * i, 0, 4);
	}
	for (uint8_t i = 0; i < 4; i++)
	{
		avail_pids_info_type[i] = can_uds_get_raw_infotype_data(0x20 * i, 0, 4);
	}
	uint32_t str_len = sprintf(save_str, "%s,0x%X,0x%X,0x%X,0x%X,0x%X,0x%X,0x%X,0x%X,0x%X,0x%X,\n",
								vin, avail_pids_current_data[0], avail_pids_current_data[1],
								avail_pids_current_data[2], avail_pids_current_data[3],
								avail_pids_current_data[4], avail_pids_current_data[5],
								avail_pids_info_type[0], avail_pids_info_type[1],
								avail_pids_info_type[2], avail_pids_info_type[3]);
	res = f_puts(save_str, &vin_file);
	assert( lv_port_take_lvgl_mutex(500) );
	if (res == strlen(save_str))
	{
		lv_obj_t* msg_box = ui_helpers_show_msgbox(save_str, NULL, NULL);
		ui_helpers_add_msgbox_close_btn(msg_box, NULL);
	}
	else
	{
		lv_obj_t* msg_box = ui_helpers_show_msgbox("Failed to save VIN to file.", NULL, NULL);
		ui_helpers_add_msgbox_close_btn(msg_box, NULL);
	}
	lv_port_give_lvgl_mutex();
	free(save_str);
	f_close(&vin_file);
}

FRESULT file_mngr_config_file_exists()
{
	FRESULT rtn_val = f_stat(FILE_MNGR_CONFIG_FILE_PATH, NULL);
	return rtn_val;
}

char* file_mngr_csv_split(char* str, uint32_t index)
{
	char* sv_ptr = NULL;
	char* split = NULL;

	split = strtok_r(str, ",", &sv_ptr);
	for (uint32_t i = 0; i < index; i++)
	{
		split = strtok_r(NULL, ",", &sv_ptr);
	}
	return split;
}

uint32_t file_mngr_csv_get_num_cols(char* str)
{
	uint32_t str_len = strlen(str);
	char* str_copy = (char*)malloc(str_len);
	if (str_copy == NULL)
	{
		return 0;
	}
	strcpy(str_copy, str);
	char* sv_ptr = NULL;
	char* split = NULL;

	uint8_t counter = 0;
	split = strtok_r(str_copy, ",", &sv_ptr);
	while (split != NULL)
	{
		counter++;
		split = strtok_r(NULL, ",", &sv_ptr);
	}
	free(str_copy);
	return counter;
}

