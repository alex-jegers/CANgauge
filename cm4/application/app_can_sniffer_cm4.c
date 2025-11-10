/**********     INCLUDES        **********/
#include "app_can_sniffer_cm4.h"
#include "app_shared_mem.h"
#include "drivers/stm32_hsem.h"
#include <stdio.h>				//For sprintf.
#include <string.h>				//For memcpy.
/**********		DEFINES		**********/

/**********		EXTERNAL VARIABLE DEFINITIONS		**********/

/**********		STATIC VARIABLES		**********/
bool _run;
static can_rx_buffer_entry_t can_rx[100];
static uint32_t can_rx_timestamps[100];
static uint32_t can_rx_period[100];
static uint8_t can_rx_unique_ids;


/**********		STATIC FUNCTION DECLRATIONS		**********/
static void _task_can_transmit(FDCAN_GlobalTypeDef* canbus);
static void _task_can_receive(FDCAN_GlobalTypeDef* canbus);
static void _read_from_fifo0(FDCAN_GlobalTypeDef* canbus);
static void _fifo0_int_handler();

/**********		STATIC FUNCTION DEFINITIONS		**********/
static void _task_can_transmit(FDCAN_GlobalTypeDef* canbus)
{
	static uint32_t time_till_next = 100;
	while(_run == true)
	{
		while (shared_get_tx_unique_ids(canbus) == 0)
		{
			vTaskDelay(pdMS_TO_TICKS(500));
		}

		for (uint8_t i = 0; i < shared_get_tx_unique_ids(canbus); i++)
		{
			uint32_t interval = shared_get_tx_interval(i);
			/* If the interval = 0, it's only sent on command so skip it. */
			if (interval == 0)
			{
				continue;
			}
			uint32_t last_time_sent = shared_get_tx_last_time_sent(i);
			uint32_t time_now = pdTICKS_TO_MS(xTaskGetTickCount());
			uint32_t time_since = time_now - last_time_sent;

			/* Check to see if it should be transmitted. */
			if (time_since > interval)
			{
				can_tx(canbus, i);
				last_time_sent = time_now;
				time_since = 0;
				shared_set_tx_time_last_sent(i, time_now);
			}
			if ((interval - time_since) < time_till_next)
			{
				time_till_next = interval - time_since;
			}
		}
		vTaskDelay(time_till_next);
	}
}

static void _task_can_receive(FDCAN_GlobalTypeDef* canbus)
{
	while(_run = true)
	{
		/*While there's data in FIFO0, read it.*/
		while(can_check_for_rx_fifo0(FDCAN1))
		{
 			_read_from_fifo0(FDCAN1);
		}
		vTaskDelay(pdMS_TO_TICKS(200));
		/*
		 * Suspend task if there's no more data to read.
		 * Task is un-suspended from an interrupt.
		 * */
		//vTaskSuspend(NULL);		//Suspend task if there's no more data to read.


	}
}

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

}

/**********		GLOBAL FUNCTION DEFINITIONS		**********/
void app_can_sniffer_run()
{
	_run = true;
	while ((shared_get_can_baud_rate(FDCAN1) == CAN_BAUD_ERROR) && (shared_get_can_baud_rate_override(FDCAN1) == CAN_BAUD_ERROR))
	{
		vTaskDelay(pdMS_TO_TICKS(100));
	}

	while (can_take(FDCAN1) == false)
	{
		vTaskDelay(pdMS_TO_TICKS(100));
	}


	can_init(FDCAN1);
	if (shared_get_can_baud_rate_override(FDCAN1) != CAN_BAUD_ERROR)
	{
		can_set_baud_rate(FDCAN1, shared_get_can_baud_rate_override(FDCAN1));
		shared_set_can_baud(FDCAN1, shared_get_can_baud_rate_override(FDCAN1));
		hsem_lock(HSEM_CAN_BAUD_RATE, 0);
		hsem_signal(HSEM_CAN_BAUD_RATE, 0);
	}
	else if (shared_get_can_baud_rate(FDCAN1) != CAN_BAUD_ERROR)
	{
		can_set_baud_rate(FDCAN1, shared_get_can_baud_rate(FDCAN1));
	}
	else
	{
		shared_set_can_error(FDCAN1, true);
		vTaskDelete(NULL);
	}


	/*Assign interrupt handler and enable new RX interrupt.*/
	//can_assign_rx_rf0n_cb(FDCAN1, _fifo0_int_handler);
	//can_enable_rx_rf0n_interrupt(FDCAN1);

	can_run(FDCAN1);
	xTaskCreate(_task_can_transmit, "CAN_TX", 500, FDCAN1, 3, NULL);
	xTaskCreate(_task_can_receive, "CAN_RX", 500, FDCAN1, 3, NULL);

}

void app_can_sniffer_stop()
{
	_run = false;
	can_stop(FDCAN1);
	can_deinit(FDCAN1);
}

bool app_can_sniffer_running()
{
	return _run;
}
