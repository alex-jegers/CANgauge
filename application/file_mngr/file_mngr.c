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

/**********		STATIC FUNCTION DEFINITIONS		**********/
static void prv_file_mngr_task(void* args)
{
	while (1)
	{
		/**
		 * 1. Compare the files in the data logs folder against the items in the list
		 * 		and remove items from the list that are no longer in the folder and
		 * 		add items in the folder that are not in the list.
		 */
		prv_update_files_list();
		vTaskDelay(500);
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
	uint32_t num_file_names =  ui_graph_get_number_of_list_items();
	for(;;)
	{
		/* Get an item from the firmware folder. */
		res = f_readdir(&dir, &dir_file_info);
		/* If it's zero there's no more files in the directory, were done and break. */
		if (strlen(dir_file_info.fname) == 0)
		{
			break;
		}

		/* If there's no files in the list, this one has to be added. */
		if (num_file_names == 0)
		{
			ui_graph_add_file_to_list(dir_file_info.fname);
			continue;
		}

		/* If it's not zero, check and see if it's already in the list box. */
		bool already_in_list = false;
		for (uint32_t i = 0; i < num_file_names; i++)
		{
			const char* list_txt = ui_graph_get_file_list_item(i);
			/* Check if it still exists. */
			DIR temp_dir;
			FILINFO list_file_info;

			/* Check if we can find the file that's in the list. */
			res = f_findfirst(&temp_dir, &list_file_info, PRV_DATA_LOGS_FILE_PATH, list_txt);
			/* If not, delete the item from the list. */
			if (strlen(list_file_info.fname) == 0)
			{
				ui_graph_delete_file_from_list(i);
				continue;
			}

			/* If it does exist, see if it's the same name as what we just read from the drive.
			 * and if it is skip adding the name from the drive to the list. */
			if (strcmp(dir_file_info.fname, list_file_info.fname) == 0)
			{
				already_in_list = true;
			}
		}
		if (already_in_list == false)
		{
			ui_graph_add_file_to_list(dir_file_info.fname);
		}

	}
}

/**********		GLOBAL FUNCTION DEFINITIONS		**********/
bool file_mngr_run()
{
	bool rtn_val = xTaskCreate(prv_file_mngr_task, "FILE_MNGR", 2000 / 4, NULL, 1, &prv_file_mngr_task_handle);
}
