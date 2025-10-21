/**********     INCLUDES        **********/
#include "app_can_sniffer_cm4.h"
#include "app_shared_mem.h"
#include <stdio.h>				//For sprintf.
#include <string.h>				//For memcpy.
/**********		DEFINES		**********/

/**********		EXTERNAL VARIABLE DEFINITIONS		**********/
TaskHandle_t app_can_sniffer_task_handle = NULL;

/**********		STATIC VARIABLES		**********/
static can_rx_buffer_entry_t can_rx[100];
static uint32_t can_rx_timestamps[100];
static uint32_t can_rx_period[100];
static uint8_t can_rx_unique_ids;


/**********		STATIC FUNCTION DECLRATIONS		**********/
static void _read_from_fifo0(FDCAN_GlobalTypeDef* canbus);
static void _fifo0_int_handler();

/**********		STATIC FUNCTION DEFINITIONS		**********/
static void _read_from_fifo0(FDCAN_GlobalTypeDef* canbus)
{
	can_rx_buffer_entry_t _new_rx;
	can_read_from_fifo0(canbus, &_new_rx);

	/*Check if it's a std length ID and shift it right if it is.*/
	if (_new_rx.R0.bit.XTD == CAN_ID_STD)
	{
		_new_rx.R0.bit.ID = _new_rx.R0.bit.ID >> 18;
	}
	/*
	 * Check to see if the ID already exists, if it does
	 * update it and return.
	 * If not, create a new entry and increment the
	 * unique IDs count.
	 */
	uint32_t current_time = xTaskGetTickCount();
	for (uint8_t i = 0; i < can_rx_unique_ids; i++)					//Iterate through however many unique IDs have been received.
	{

		if (_new_rx.R0.bit.ID == can_rx[i].R0.bit.ID)				//Check the recieeved ID against all the IDs that have been rx'd.
		{
			can_rx[i] = _new_rx;									//Keep a local numerical copy.
			can_rx_period[i] = current_time - can_rx_timestamps[i];	//Calculate the period.
			can_rx_timestamps[i] = current_time;					//Save the current time.

			char _temp_period_ms[10];
			sprintf(_temp_period_ms, "%d", can_rx_period[i]);
			shared_set_can_str_period(canbus, i, _temp_period_ms);

			char _temp_id[8] = {0,0,0,0,0,0,0,0};
			sprintf(_temp_id, "%X", _new_rx.R0.bit.ID);				//Convert the ID into a hex string.
			shared_set_can_str_id(canbus, i, _temp_id);				//Write that string into shared mem for CM7 to get.

			/*Convert all the data bytes into hex strings.*/
			char _temp_data[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};	//Need to write to _temp first because sprintf writes a null terminator which overwrites data in can_data_str.
			uint8_t _str_counter = 0;
			for (uint8_t x = 0; x < _new_rx.R1.bit.DLC; x++)
			{
				_str_counter += sprintf(&_temp_data[_str_counter], "%X", _new_rx.data[x]);
				shared_set_can_str_data(canbus, i, _temp_data);
			}
			return;
		}
	}
	can_rx[can_rx_unique_ids] = _new_rx;
	can_rx_timestamps[can_rx_unique_ids] = current_time;
	can_rx_unique_ids++;
	shared_set_can_unique_ids(canbus, can_rx_unique_ids);
}

static void _fifo0_int_handler()
{
	xTaskResumeFromISR(&app_can_sniffer_task_handle);
}

/**********		GLOBAL FUNCTION DEFINITIONS		**********/
void app_can_sniffer_run()
{
	while (shared_mem_get_can_baud_rate(FDCAN1) == CAN_BAUD_ERROR)
	{
		vTaskDelay(1000);
	}

	while (can_take(FDCAN1) == false)
	{
		vTaskDelay(1000);
	}


	can_init(FDCAN1);
	can_set_baud_rate(FDCAN1, shared_mem_get_can_baud_rate(FDCAN1));

	/*Assign interrupt handler and enable new RX interrupt.*/
	//can_assign_rx_rf0n_cb(FDCAN1, _fifo0_int_handler);
	//can_enable_rx_rf0n_interrupt(FDCAN1);

	can_run(FDCAN1);

	while(1)
	{
		/*While there's data in FIFO0, read it.*/
		while(can_check_for_rx_fifo0(FDCAN1))
		{
 			_read_from_fifo0(FDCAN1);
		}
		/*
		 * Suspend task if there's no more data to read.
		 * Task is un-suspended from an interrupt.
		 * */
		//vTaskSuspend(NULL);		//Suspend task if there's no more data to read.
	}
}
