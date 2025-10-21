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
static char _temp_id[8] = {'B', '0', '0', 'B'};

/**********		STATIC FUNCTION DECLRATIONS		**********/
static void _update_baud_rate_lbls();
static void _run_get_baud_rate();

static void _connect_to_can1_btn_hanlder(lv_event_t* e);
static void _connect_to_can2_btn_hanlder(lv_event_t* e);
static void _send_btn_hanlder(lv_event_t* e);
static void _add_btn_hanlder(lv_event_t* e);
static void _delete_btn_hanlder(lv_event_t* e);
static void _back_btn_hanlder(lv_event_t* e);

/**********		STATIC FUNCTION DEFINITIONS		**********/
static void _update_baud_rate_lbls()
{
	switch (shared_mem_get_can_baud_rate(FDCAN1))
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

	switch (shared_mem_get_can_baud_rate(FDCAN2))
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

static void _run_get_baud_rate()
{

}

/**********		GLOBAL FUNCTION DEFINITIONS		**********/
void app_can_sniffer_cm7()
{
	/*Wait for the LVGL mutex to be created.*/
	while (sys_mutex_lvgl == NULL)
	{
		vTaskDelay(500);
	}
	/*Take the LVGL mutex and load the UI.*/
	xSemaphoreTake(sys_mutex_lvgl, 5);
	ui_can_sniffer_load();
	lv_label_set_text(ui_can_sniffer_can1_baud_lbl, "CAN1 Baud: Disconnected.");
	lv_label_set_text(ui_can_sniffer_can2_baud_lbl, "CAN2 Baud: Disconnected.");
	xSemaphoreGive(sys_mutex_lvgl);

	/*This signals the baud rate app to run.*/
	hsem_lock(HSEM_CAN_BAUD_RATE, HSEM_ID_CAN_BAUD_RATE_RUN);
	hsem_signal(HSEM_CAN_BAUD_RATE, HSEM_ID_CAN_BAUD_RATE_RUN);

	while(1)
	{
		/*Check to see if the baud rate app is done running.*/
		if (hsem_wait_bool(HSEM_CAN_BAUD_RATE, HSEM_ID_CAN_BAUD_RATE_DONE))
		{
			xSemaphoreTake(sys_mutex_lvgl, 5);
			_update_baud_rate_lbls();
			xSemaphoreGive(sys_mutex_lvgl);
			hsem_clear_int(0);
		}

		xSemaphoreTake(sys_mutex_lvgl, 0);

		for (uint8_t i = 0; i < shared_get_can_unique_ids(FDCAN1); i++)
		{
			lv_table_set_cell_value(ui_can_sniffer_table, i, 0, shared_get_can_str_id(FDCAN1, i));
			lv_table_set_cell_value(ui_can_sniffer_table, i, 1, shared_get_can_str_period(FDCAN1, i));
			lv_table_set_cell_value(ui_can_sniffer_table, i, 2, shared_get_can_str_data(FDCAN1, i));
		}
		xSemaphoreGive(sys_mutex_lvgl);
		vTaskDelay(30);
	}
}
