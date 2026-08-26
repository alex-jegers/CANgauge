/**********     INCLUDES        **********/
#include "data_logger.h"

/**********     TYPEDEFS         **********/

/**********		DEFINES		**********/

/**********		EXTERNAL VARIABLE DEFINITIONS		**********/

/**********		STATIC VARIABLES		**********/

/**********		STATIC FUNCTION DECLRATIONS		**********/
void prv_data_logger_task_function(void* data_logger_info_struct);

/**********		STATIC FUNCTION DEFINITIONS		**********/
void prv_data_logger_task_function(void* data_logger_info_struct_ptr)
{
	data_logger_handle_t* hndl = data_logger_info_struct_ptr;

	/*** Determine how many parameters were recording. ***/
	uint8_t num_params = 0;
	for (uint8_t i = 0; i < 4; i++)
	{
		if (hndl->data[i] == NULL)
		{
			num_params = i;
			break;
		}
		num_params = i + 1;
	}

	FRESULT res;		//This is used to check the return value of the coming FatFS function calls.

	/*** Change to and/or create the directory. ***/
	res = f_chdir("0:/Data Logs/");
	if (res == FR_NO_PATH)
	{
		f_mkdir("0:/Data Logs/");
		res = f_chdir("0:/Data Logs/");
		if (res != FR_OK)
		{
			hndl->error_cb(DATA_LOGGER_ERROR_FILE_SYS_ERR);
			vTaskDelete(NULL);
		}
	}

	/*** Create the file name. ***/
	char file_name[51];		//51 is 10 characters of each parameter, 6 for the _x.csv, 3 commas, 1 null terminator
	memset(file_name, 0, sizeof(file_name));
	for (uint8_t i = 0; i < num_params; i++)
	{
		memcpy(file_name + (i * 11), hndl->data[i]->name, 10);
		if (i != num_params - 1)
		{
			strcat(file_name, ",");
		}
	}
	strcat(file_name, "_1.csv");		//Add the file type.

	/*** Check if the file name exists and change it if it does. ***/
	FILINFO f_info;
	res = f_stat(file_name, &f_info);
	uint8_t counter = 1;
	while (res == FR_OK)
	{
		counter++;
		uint32_t str_len = strlen(file_name);
		char file_name_new[60];
		file_name[str_len - 6] = '\0';
		sprintf(file_name_new, "%s_%u.csv", file_name, counter);
		res = f_stat(file_name_new, &f_info);
		memcpy(file_name, file_name_new, str_len);
	}
	/* *
	 * End of creating file name. File name is stored in "file_name".
	 * At this point the file name is sure to not exist so we can create
	 * a new file using it.
	 *  */

	/*** Create the file. ***/
	FIL file;
	res = f_open(&file, file_name, FA_CREATE_NEW | FA_WRITE);
	if (res != FR_OK)
	{
		hndl->error_cb(DATA_LOGGER_ERROR_FILE_SYS_ERR);
		f_close(&file);		//Close the file.
		f_chdir("0:/");		//Change the working directory back.
		vTaskDelete(NULL);
	}

	/*** Based on the amount of parameters we are recording figure out how much data we can
	 * record given the amount of space left in EEPROM (or whatever storage device, at the time
	 * of writing this the first time, it's EEPROM).
	 ***/
	uint32_t remaining_space_bytes = filesys_get_free_space("0:/");				//Remaing bytes in EEPROM.
	res = f_expand(&file, remaining_space_bytes, 0);							//Try to allocate it.
	if (res != FR_OK)
	{
		uint32_t bytes_to_allocate = remaining_space_bytes / 2;
		uint32_t largest_available_contiguous_size_bytes = 0;
		for (uint8_t i = 0; i < 11; i++)
		{
			res = f_expand(&file, bytes_to_allocate, 0);							//Try to allocate it.
			if (res != FR_OK)
			{
				bytes_to_allocate /= 2;
			}
			else if (res == FR_OK)
			{
				if (bytes_to_allocate > largest_available_contiguous_size_bytes)
				{
					largest_available_contiguous_size_bytes = bytes_to_allocate;
				}
				bytes_to_allocate = ((remaining_space_bytes - bytes_to_allocate) / 2) + bytes_to_allocate;
			}
		}
		remaining_space_bytes = largest_available_contiguous_size_bytes;
	}

	uint32_t remaining_space_floats = remaining_space_bytes / sizeof(float);	//Remaining space for floats (the data were saving are floats).
	uint32_t number_of_columns = num_params + 1;								//Plus one because we need an extra column for time.
	uint32_t num_rows = remaining_space_floats / (number_of_columns);			//Plus 1 to num params because we have to record the time too.
	float* data_arr = malloc(number_of_columns * num_rows * sizeof(float));		//Plus 1 to num params because we have to record the time too.


	/*** Time to start recording data. ***/
	uint32_t starting_time_ms = pdTICKS_TO_MS(xTaskGetTickCount());		//Obv the start time for the recording so this can be referenced and the first entry is at 0ms.
	uint32_t rows_written = 0;		//This keeps track of how many rows we've written total so we know when we are writing to the file.
	uint32_t row = 0;				//This keeps track of the current row being written to index the array.

	/* The main loop. */
	while (hndl->run == true)
	{
		uint32_t current_time_ms = pdTICKS_TO_MS(xTaskGetTickCount());

		for (uint8_t i = 0; i < num_params; i++)
		{
			/* Need these 3 things to get the proper raw data. */
			uint8_t pid = hndl->data[i]->pid_code;
			uint8_t first_byte = hndl->data[i]->first_byte;
			uint8_t data_bytes = hndl->data[i]->data_bytes;

			/* Need scale and offset to calculate the actual value. */
			float scale = hndl->data[i]->scale;
			float offset = hndl->data[i]->offset;

			uint32_t raw_data = can_uds_get_raw_current_data(pid, first_byte, data_bytes);	// Get the raw data.
			float processed_data = (raw_data * scale) + offset;								// Calculate the actual value.
			data_arr[(row * number_of_columns) + (i + 1)] = processed_data;					// Save it into the data array. Adding 1 to i because time has to go in the first column. This equation is ((row * columns) + column)
		}
		data_arr[(row * number_of_columns) + 0] = (float)(current_time_ms - starting_time_ms);
		row++;
		if (row >= num_rows)
		{
			hndl->run = false;
		}
		rows_written = row;
		vTaskDelay(hndl->period_ms);
	}

	/* *
	 * At this point the file is created successfully and we can start writing data to it.
	 * */

	/*** Make the header for the CSV file. ***/
	const char* time_header_str = "Time (ms),";
	f_puts(time_header_str, &file);			//Write the "Time" column header with a comma.
	f_puts(hndl->data[0]->name, &file);		//Write the first data parameter column header.

	if (num_params == 1)					//If this is the only parameter put a newline.
	{
		f_putc('\n', &file);
	}
	for (uint8_t i = 1; i < num_params; i++)
	{
		f_putc(',', &file);
		f_puts(hndl->data[i]->name, &file);
		if (i == num_params - 1)		//This is the last one.
		{
			f_putc('\n', &file);
		}
	}

	/*** Start writing the collected data to the file. ***/
	for (uint32_t current_row = 0; current_row < rows_written; current_row++)
	{
		f_printf(&file, "%.4f,%.4f", data_arr[(current_row * number_of_columns) + 0], data_arr[(current_row * number_of_columns) + 1]);
		for (uint8_t additional_pids = 2; additional_pids < num_params + 1; additional_pids++)		//Plus 1 because the first column is time.
		{
			f_printf(&file, ",%.4f", data_arr[(current_row * number_of_columns) + additional_pids]);
		}
		int32_t chars_written = f_putc('\n', &file);
		if (chars_written != 1)
		{
			hndl->error_cb(DATA_LOGGER_ERROR_NO_MEM);
			break;
		}
	}

	free(data_arr);
	f_close(&file);		//Close the file.
	f_chdir("0:/");		//Change the working directory back.
	vTaskDelete(NULL);	//Delete the task.
}

/**********		GLOBAL FUNCTION DEFINITIONS		**********/
bool data_logger_start_recording(data_logger_handle_t* handle)
{
	if (handle->period_ms == 0) { return false; }
	if (handle->data[0] == NULL) { return false; }
	handle->run = true;
	bool rtn_val = xTaskCreate(prv_data_logger_task_function, "LOGGER", 2000 / 4, handle, 4, &handle->task_handle);
	return rtn_val;
}

void data_logger_stop_recording(data_logger_handle_t* hndl)
{
	hndl->run = false;
}

void data_logger_set_data(data_logger_handle_t* hndl, saej1979_current_data_t* data_to_record[4])
{
	memcpy(hndl->data, data_to_record, sizeof(saej1979_current_data_t*) * 4);
}
void data_logger_set_period(data_logger_handle_t* hndl, uint32_t period_ms)
{
	hndl->period_ms = period_ms;
}

void data_logger_set_error_cb(data_logger_handle_t* hndl, void (*func)(data_logger_error_code_t ec))
{
	hndl->error_cb = func;
}

bool data_logger_recording(data_logger_handle_t* hndl)
{
	return hndl->run;
}
