/**********     INCLUDES        **********/
#include "app_can_sniffer_cm7.h"
#include "common/app_shared_mem.h"
#include "ui/ui_can_sniffer.h"

#include "drivers/stm32_canbus.h"
#include "drivers/stm32_hsem.h"

#include "system/system_cm7.h"

#include <string.h>		//For memcpy
/**********		DEFINES		**********/

/**********		EXTERNAL VARIABLE DEFINITIONS		**********/

/**********		STATIC VARIABLES		**********/

/**********		STATIC FUNCTION DECLRATIONS		**********/
static void _update_baud_rate_lbls();							//
static void _update_rx_table();									//Updates the received data.
static void _run_get_baud_rate();
static void _tgl_cm4_can_sniffer();

static void _connect_to_can1_btn_hanlder(lv_event_t* e);		//Tell CM4 to attempt to connect to CAN1 and start receiving data at the specified baud rate.
static void _disconnect_from_can1_btn_handler(lv_event_t* e);
static void _connect_to_can2_btn_hanlder(lv_event_t* e);		//Tell CM4 to attempt to connect to CAN2 and start receiving data at the specified baud rate.
static void _back_btn_handler(lv_event_t* e);					//Shut down the task.
static void _tx_ctrl_btn_handler(lv_event_t* e);

/**********		STATIC FUNCTION DEFINITIONS		**********/
static void _update_baud_rate_lbls()
{
	switch (shared_get_can_baud_rate(FDCAN1))
	{
		case CAN_BAUD_125K:
			lv_label_set_text(ui_can_sniffer_can1_baud_lbl, "CAN1 Baud: 125K");
			break;
		case CAN_BAUD_250K:
			lv_label_set_text(ui_can_sniffer_can1_baud_lbl, "CAN1 Baud: 250K");
			break;
		case CAN_BAUD_500K:
			lv_label_set_text(ui_can_sniffer_can1_baud_lbl, "CAN1 Baud: 500K");
			break;
		case CAN_BAUD_1M:
			lv_label_set_text(ui_can_sniffer_can1_baud_lbl, "CAN1 Baud: 1M");
			break;
		case CAN_BAUD_ERROR:
			lv_label_set_text(ui_can_sniffer_can1_baud_lbl, "CAN1 Baud: Disconnected");
			break;
	}

	switch (shared_get_can_baud_rate(FDCAN2))
	{
		case CAN_BAUD_125K:
			lv_label_set_text(ui_can_sniffer_can2_baud_lbl, "CAN2 Baud: 125K");
			break;
		case CAN_BAUD_250K:
			lv_label_set_text(ui_can_sniffer_can2_baud_lbl, "CAN2 Baud: 250K");
			break;
		case CAN_BAUD_500K:
			lv_label_set_text(ui_can_sniffer_can2_baud_lbl, "CAN2 Baud: 500K");
			break;
		case CAN_BAUD_1M:
			lv_label_set_text(ui_can_sniffer_can2_baud_lbl, "CAN2 Baud: 1M");
			break;
		case CAN_BAUD_ERROR:
			lv_label_set_text(ui_can_sniffer_can2_baud_lbl, "CAN2 Baud: Disconnected");
			break;
	}
}

static void _update_rx_table()
{
	/*If there hasn't been any data received we dont need to update the table at all.*/
	if (shared_get_can_unique_ids(FDCAN1) == 0)
	{
		return;
	}

	/*Wait for the LVGL mutex to become available.*/
	if (xSemaphoreTake(sys_mutex_lvgl, 0) == pdPASS)
	{
		for (uint8_t i = 0; i < shared_get_can_unique_ids(FDCAN1); i++)
		{
			lv_table_set_cell_value(ui_can_sniffer_table, i, 0, shared_get_can_str_id(FDCAN1, i));
			lv_table_set_cell_value(ui_can_sniffer_table, i, 1, shared_get_can_str_period(FDCAN1, i));
			lv_table_set_cell_value(ui_can_sniffer_table, i, 2, shared_get_can_str_data(FDCAN1, i));
		}
		xSemaphoreGive(sys_mutex_lvgl);
	}
}

static void _run_get_baud_rate()
{
	/*This signals the baud rate app to run.*/
	hsem_lock(HSEM_CAN_BAUD_RATE, HSEM_ID_CAN_BAUD_RATE_RUN);
	hsem_signal(HSEM_CAN_BAUD_RATE, HSEM_ID_CAN_BAUD_RATE_RUN);
}

static void _tgl_cm4_can_sniffer()
{
	hsem_lock(HSEM_APP_CAN_SNIFFER, HSEM_ID_APP_CAN_SNIFFER);
	hsem_signal(HSEM_APP_CAN_SNIFFER, HSEM_ID_APP_CAN_SNIFFER);
}

static void _connect_to_can1_btn_hanlder(lv_event_t* e)
{
	if (strcmp(ui_can_sniffer_get_can1_dd_list_text(), "Auto") == 0)
	{
		shared_set_target_can(FDCAN1);
		_run_get_baud_rate();
	}
	else if (strcmp(ui_can_sniffer_get_can1_dd_list_text(), "1Mbps") == 0)
	{

	}
	else if (strcmp(ui_can_sniffer_get_can1_dd_list_text(), "500kbps") == 0)
	{
		shared_set_can_baud_override(FDCAN1, CAN_BAUD_500K);
	}
	else if (strcmp(ui_can_sniffer_get_can1_dd_list_text(), "250kbps") == 0 )
	{

	}
	else if (strcmp(ui_can_sniffer_get_can1_dd_list_text(), "125kbps") == 0)
	{

	}
	else
	{
		return;
	}
	_tgl_cm4_can_sniffer();
}

static void _disconnect_from_can1_btn_handler(lv_event_t* e)
{
	_tgl_cm4_can_sniffer();
}

static void _connect_to_can2_btn_hanlder(lv_event_t* e)
{

}

static void _back_btn_handler(lv_event_t* e)
{

}

static void _tx_ctrl_btn_handler(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t* temp_btn_matrix = lv_event_get_target_obj(e);
    if (code == LV_EVENT_VALUE_CHANGED)
    {
    	/* Get the event information. */
    	uint32_t btn_id = lv_buttonmatrix_get_selected_button(temp_btn_matrix);
        const char* btn_txt = lv_buttonmatrix_get_button_text(temp_btn_matrix, btn_id);

        /* Get the selected row information .*/
        uint32_t selected_row = ui_can_sniffer_get_tx_table_selected_row();
        uint32_t row_count = ui_can_sniffer_get_tx_table_number_of_rows();

        if (strcmp(btn_txt, "Add") == 0)
        {
        	/*Create all the variables needed.*/
        	char* id_str;
			char* interval_str;
			char* data_str;
        	uint32_t id_int;
        	uint32_t interval_int;
        	uint8_t data_int[8] = {0,0,0,0,0,0,0,0};
        	uint32_t row = row_count - 1;
        	can_tx_buffer_entry_t tx_buf;

        	/* Get the data in string form.*/
        	id_str = ui_can_sniffer_get_tx_table_cell_value(row, 0);
        	interval_str = ui_can_sniffer_get_tx_table_cell_value(row, 1);
        	data_str = ui_can_sniffer_get_tx_table_cell_value(row, 2);

        	/* Convert to numbers. */
        	id_int = strtol(id_str, NULL, 16);
        	interval_int = atoi(interval_str);

        	uint8_t len = strlen(data_str);
            for (int i = 0; i < len; i += 2) {
                // Allocate space for the pair (2 characters + null terminator)
                char* pair = (char *)malloc(3 * sizeof(char));
                if (pair == NULL) {
                    assert(0);
                    return;
                }

                // Copy the first character
                pair[0] = data_str[i];

                // Copy the second character if available, otherwise add an underscore
                if (i + 1 < len) {
                    pair[1] = data_str[i+1];
                } else {
                    pair[1] = '0'; // Handle odd length string by adding a filler character
                }
                pair[2] = '\0'; // Null-terminate the pair

                data_int[i / 2] = strtol(pair, NULL, 16);
                free(pair); // Free the allocated memory for the pair
            }


        	/* Put it all the CAN message RAM. */
        	if (id_int < 0x7FF)
        	{
        		tx_buf.T0.bit.ID = id_int << 18;
        		tx_buf.T0.bit.XTD = 0;
        	}
        	else
        	{
        		tx_buf.T0.bit.ID = id_int;
        		tx_buf.T0.bit.XTD = 1;
        	}
        	tx_buf.T0.bit.RTR = 0;
        	tx_buf.T1.bit.DLC = 8;
        	tx_buf.T1.bit.BRS = 0;
        	tx_buf.T1.bit.FDF = 0;
        	tx_buf.T1.bit.EFC = 0;
        	memcpy(tx_buf.data, data_int, sizeof(data_int));
        	can_add_tx_buffer(FDCAN1, &tx_buf, row);

        	shared_set_can_tx_data(interval_int, row);
        	shared_set_tx_unique_ids(FDCAN1, shared_get_tx_unique_ids(FDCAN1) + 1);
        }
        else if (strcmp(btn_txt, "Delete") == 0)
        {
            if (row_count <= 1)
            {
                shared_set_tx_unique_ids(FDCAN1, 0);
            }
            else
            {
                for (uint32_t i = selected_row; i < (row_count - 1); i++)
                {
                    can_add_tx_buffer(FDCAN1, can_get_tx_buffer(FDCAN1, i + 1), i);
                    shared_set_can_tx_data(shared_get_tx_interval(i + 1), i);
                }
                shared_set_tx_unique_ids(FDCAN1, shared_get_tx_unique_ids(FDCAN1) - 1);
            }

        }
        else if (strcmp(btn_txt, "Send") == 0)
        {

        }

    }
}

/**********		GLOBAL FUNCTION DEFINITIONS		**********/
void app_can_sniffer_cm7()
{
	/*Take the LVGL mutex and load the UI.*/
	if (xSemaphoreTake(sys_mutex_lvgl, portMAX_DELAY) == pdPASS)
	{
		lv_label_set_text(ui_can_sniffer_can1_baud_lbl, "CAN1 Baud: Disconnected.");
		lv_label_set_text(ui_can_sniffer_can2_baud_lbl, "CAN2 Baud: Disconnected.");
		xSemaphoreGive(sys_mutex_lvgl);
	}

	/* Set the button event call backs. */
	ui_can_sniffer_assign_can1_connect_btn_cb(_connect_to_can1_btn_hanlder);
	ui_can_sniffer_assign_can1_disconnect_btn_cb(_disconnect_from_can1_btn_handler);
	ui_can_sniffer_assign_can2_connect_btn_cb(_connect_to_can2_btn_hanlder);
	ui_can_sniffer_assign_back_to_main_btn_cb(_back_btn_handler);
	ui_can_sniffer_assign_tx_ctrl_btn_matrix_cb(_tx_ctrl_btn_handler);

	/* Lower its own priority now that it's initialized. */
	vTaskPrioritySet(NULL, 2);

	while(1)
	{
		/* Check to see if the baud rate app ran and if it did update the status labels. */
		if (hsem_get_status(HSEM_CAN_BAUD_RATE))
		{
			if (xSemaphoreTake(sys_mutex_lvgl, portMAX_DELAY) == pdPASS)
			{
				_update_baud_rate_lbls();
				hsem_clear_int(HSEM_CAN_BAUD_RATE);
				xSemaphoreGive(sys_mutex_lvgl);
			}

		}

		_update_rx_table();

		vTaskDelay(30);
	}
}
