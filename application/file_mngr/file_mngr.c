/**********     INCLUDES        **********/
#include "file_mngr.h"
#include "ui/ui_graph.h"

/**********     TYPEDEFS         **********/

/**********		DEFINES		**********/
#define PRV_DATA_LOGS_FILE_PATH		"0:/Data Logs/"
/**********		EXTERNAL VARIABLE DEFINITIONS		**********/

/**********		STATIC VARIABLES		**********/
static TaskHandle_t prv_file_mngr_task_handle;

/**********		STATIC FUNCTION DECLRATIONS		**********/
static void prv_file_mngr_task(void* args);
static void prv_update_files_list();
static uint32_t prv_get_number_of_log_files();

/**********		STATIC FUNCTION DEFINITIONS		**********/
static void prv_file_mngr_task(void* args)
{
	vTaskDelay(4000);
	while (1)
	{
		/**
		 * 1. Compare the files in the data logs folder against the items in the list
		 * 		and remove items from the list that are no longer in the folder and
		 * 		add items in the folder that are not in the list.
		 */
		prv_update_files_list();
		vTaskDelay(1500);
	}
}

static void prv_update_files_list()
{
	FRESULT res;
	DIR dir;
	FILINFO dir_file_info;
	res = f_opendir(&dir, PRV_DATA_LOGS_FILE_PATH);
	if (res != FR_OK)
	{
		return;
	}

	/* Check how many files are in the list box. */
	uint32_t num_files_in_list =  ui_graph_get_number_of_list_items();
	uint32_t num_files_in_dir = prv_get_number_of_log_files();

	/* Both lists have no files in them, were done and can return. */
	if ((num_files_in_list == 0) && (num_files_in_dir == 0)) { return; }

	/* The directory has none but the UI list has some, delete all from the UI and rtn. */
	if (num_files_in_dir == 0)
	{
		for (uint32_t i = 0; i < num_files_in_list; i++)
		{
			ui_graph_delete_file_from_list(i);
		}
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

	}
	if (already_in_list == false)
	{
		ui_graph_add_file_to_list(dir_file_name);
		num_files_in_list++;		//Make a copy of this or smth so it doesnt cause more than necessary loops in the next step.
	}
	f_closedir(&dir);

	/* Go through all the file names in the list and make sure none of them have been deleted. */
	for (uint32_t i = 0; i < num_files_in_list; i++)
	{
		/* Create a couple fresh directory objects. */
		DIR temp_dir;
		FILINFO list_file_info;

		/* Get a file name from the list. */
		const char* list_file_name = ui_graph_get_file_list_item(i);

		/* Check if the directory for the file name from the list. */
		f_findfirst(&temp_dir, &list_file_info, PRV_DATA_LOGS_FILE_PATH, list_file_name);
		if (strlen(list_file_info.fname) == 0)		//If the file name strlen is zero the file is not in the directory, delete it.
		{
			ui_graph_delete_file_from_list(i);
			continue;								//Go to next file in the UI list.
		}
	}
}

static uint32_t prv_get_number_of_log_files()
{
	FRESULT res;
	DIR dir;
	FILINFO dir_file_info;
	uint32_t rtn_val = 0;
	res = f_opendir(&dir, PRV_DATA_LOGS_FILE_PATH);
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

/**********		GLOBAL FUNCTION DEFINITIONS		**********/
bool file_mngr_run()
{
	bool rtn_val = xTaskCreate(prv_file_mngr_task, "FILE_MNGR", 2000 / 4, NULL, 1, &prv_file_mngr_task_handle);
	return rtn_val;
}

bool file_mngr_stop()
{
	/* TODO: Make this cleaner. Or ensure that nothing gets left dangling. */
	vTaskSuspend(prv_file_mngr_task_handle);
	return true;
}
