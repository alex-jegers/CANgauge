/**********     INCLUDES        **********/
#include <application/app_can_controller_cm4.h>
#include "app_shared_mem.h"
#include "drivers/stm32_hsem.h"
#include <stdio.h>				//For sprintf.
#include <string.h>				//For memcpy.
/**********		DEFINES		**********/

/**********		EXTERNAL VARIABLE DEFINITIONS		**********/

/**********		STATIC VARIABLES		**********/
bool _run;

/* Task handles. */
TaskHandle_t _tx_task_handle = NULL;
TaskHandle_t _rx_task_handle = NULL;

SemaphoreHandle_t _rx_fifo0_counter = NULL;
SemaphoreHandle_t _rx_fifo1_counter = NULL;

struct can_rx_raw_data
{
	can_rx_buffer_entry_t buf;
	uint32_t timestamp;
	uint32_t period;
	uint32_t* unique_ids_ptr;
};

struct can_rx_raw_data can_raw_rx0[CAN1_RX_FIFO0_ELEMENTS];
struct can_rx_raw_data can_raw_rx1[CAN1_RX_FIFO1_ELEMENTS];
static uint8_t can_rx0_unique_ids;
static uint8_t can_rx1_unique_ids;

static can_rx_buffer_entry_t can_rx0[CAN1_RX_FIFO0_ELEMENTS];
static uint32_t can_rx0_timestamps[CAN1_RX_FIFO0_ELEMENTS];
static uint32_t can_rx0_period[CAN1_RX_FIFO0_ELEMENTS];

static can_rx_buffer_entry_t can_rx1[CAN1_RX_FIFO1_ELEMENTS];
static uint32_t can_rx1_timestamps[CAN1_RX_FIFO1_ELEMENTS];
static uint32_t can_rx1_period[CAN1_RX_FIFO1_ELEMENTS];


/**********		STATIC FUNCTION DECLRATIONS		**********/
static void _task_can_transmit(FDCAN_GlobalTypeDef* canbus);

/**
 * Saves the incoming CAN data into local storage (scope of this app).
 * Stores data into can_rx0 (fifo0) and can_rx1 (fifo1)
 */
static void _task_can_receive(FDCAN_GlobalTypeDef* canbus);

/**
 * Takes the data stored locally and moves it to shared memory so CM7 can access it.
 * Also removes data from RX that hasnt been receieved in the last 5 seconds..?
 * The LCD only refreshes at 30fps so this needs to happen at the same frequency or faster.
 */
static void _task_can_process(FDCAN_GlobalTypeDef* canbus);		
static void _read_from_fifo0(FDCAN_GlobalTypeDef* canbus);
static void _read_from_fifo1(FDCAN_GlobalTypeDef* canbus);
static void _fifo0_int_handler();

/**********		STATIC FUNCTION DEFINITIONS		**********/
static void _task_can_transmit(FDCAN_GlobalTypeDef* canbus)
{
	static uint32_t time_till_next = 100;
	while(_run == true)
	{
		/*If there aren't any CAN IDs in shared memory to transmit, wait 500ms and check again.*/
		while (shared_get_tx_unique_ids(canbus) == 0)
		{
			vTaskDelay(pdMS_TO_TICKS(500));
		}

		/*If there are IDs to transmit in shared memory, loop through them.*/
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
			if (time_since > interval)		//If time since the last transmission is greater than its interval...
			{
				can_tx(canbus, i);			//Transmit the message.
				last_time_sent = time_now;	//Save the current time.
				time_since = 0;				//Set time since last to 0.
				shared_set_tx_time_last_sent(i, time_now);	//Save the last time sent to shared memory.
			}

			/*Calculate how long we can delay the task until the next message needs to be transmitted. */
			if ((interval - time_since) < time_till_next)
			{
				time_till_next = interval - time_since;
			}
		}

		/*Delay until the next transmission is due. */
		vTaskDelay(time_till_next);
	}
	vTaskDelete(NULL);
}

static void _task_can_receive(FDCAN_GlobalTypeDef* canbus)
{
	while(_run == true)
	{
		bool empty = true;
		
		/* If there's data in FIFO0, read it. */
		if (xSemaphoreTake(_rx_fifo0_counter, 0) == pdPASS)
		{
 			_read_from_fifo0(FDCAN1);
			empty = false;
		}
		
		/* If there's data in FIFO1, read it. */
		if (xSemaphoreTake(_rx_fifo1_counter, 0) == pdPASS)
		{
			_read_from_fifo1(FDCAN1);
			empty = false;
		}
		
		/* They both failed which means theyre both empty. */
		if (empty)
		{
			vTaskDelay(5);
		}
	}
	
	/* Done running, clean up. */
	for (uint8_t i = 0; i < shared_get_can_rx0_unique_ids(canbus); i++)
	{
		memset(&can_rx0[i], 0, sizeof(can_rx_buffer_entry_t));
	}
	shared_set_can_rx0_unique_ids(canbus, 0);
	vTaskDelete(NULL);
}

static void _task_can_process(FDCAN_GlobalTypeDef* canbus)
{
	for (uint8_t i = 0; i < can_rx0_unique_ids; i++)
	{

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
	for (uint8_t i = 0; i < can_rx0_unique_ids; i++)					//Iterate through however many unique IDs have been received.
	{

		if (_new_rx.R0.bit.ID == can_rx0[i].R0.bit.ID)				//Check the recieeved ID against all the IDs that have been rx'd.
		{
			can_rx0[i] = _new_rx;									//Keep a local numerical copy.
			can_rx0_period[i] = current_time - can_rx0_timestamps[i];	//Calculate the period.
			can_rx0_timestamps[i] = current_time;					//Save the current time.

			char _temp_period_ms[10];
			sprintf(_temp_period_ms, "%d", can_rx0_period[i]);
			shared_set_can_rx0_str_period(canbus, i, _temp_period_ms);

			char _temp_id[8] = {0,0,0,0,0,0,0,0};
			sprintf(_temp_id, "%X", _new_rx.R0.bit.ID);				//Convert the ID into a hex string.
			shared_set_can_rx0_str_id(canbus, i, _temp_id);				//Write that string into shared mem for CM7 to get.

			/*Convert all the data bytes into hex strings.*/
			char _temp_data[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};	//Need to write to _temp first because sprintf writes a null terminator which overwrites data in can_data_str.
			uint8_t _str_counter = 0;
			for (uint8_t x = 0; x < _new_rx.R1.bit.DLC; x++)
			{
				_str_counter += sprintf(&_temp_data[_str_counter], "%X", _new_rx.data[x]);
				shared_set_can_rx0_str_data(canbus, i, _temp_data);
			}

			/* Save the raw CAN data to shared memory. */
			shared_set_can_rx0_buffer_entry(canbus, i, &can_rx0[i]);
			shared_set_can_rx0_time_stamp(canbus, i, current_time);

			return;
		}
	}
	can_rx0[can_rx0_unique_ids] = _new_rx;
	can_rx0_timestamps[can_rx0_unique_ids] = current_time;
	can_rx0_unique_ids++;
	shared_set_can_rx0_unique_ids(canbus, can_rx0_unique_ids);
}

static void _read_from_fifo1(FDCAN_GlobalTypeDef* canbus)
{
	can_rx_buffer_entry_t _new_rx;
	can_read_from_fifo1(canbus, &_new_rx);

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
	for (uint8_t i = 0; i < can_rx1_unique_ids; i++)					//Iterate through however many unique IDs have been received.
	{

		if (_new_rx.R0.bit.ID == can_rx1[i].R0.bit.ID)				//Check the recieeved ID against all the IDs that have been rx'd.
		{
			can_rx1[i] = _new_rx;									//Keep a local numerical copy.
			can_rx1_period[i] = current_time - can_rx1_timestamps[i];	//Calculate the period.
			can_rx1_timestamps[i] = current_time;					//Save the current time.

			/* Convert to and save the transmission period to a string in shared memory. */
			char _temp_period_ms[10];
			sprintf(_temp_period_ms, "%d", can_rx1_period[i]);
			shared_set_can_rx1_str_period(canbus, i, _temp_period_ms);

			/* Convert the ID to a string and save it in shared memory. */
			char _temp_id[8] = {0,0,0,0,0,0,0,0};
			sprintf(_temp_id, "%X", _new_rx.R0.bit.ID);				//Convert the ID into a hex string.
			shared_set_can_rx1_str_id(canbus, i, _temp_id);				//Write that string into shared mem for CM7 to get.

			/*Convert all the data bytes into hex strings.*/
			char _temp_data[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};	//Need to write to _temp first because sprintf writes a null terminator which overwrites data in can_data_str.
			uint8_t _str_counter = 0;
			for (uint8_t x = 0; x < _new_rx.R1.bit.DLC; x++)
			{
				_str_counter += sprintf(&_temp_data[_str_counter], "%X", _new_rx.data[x]);
				shared_set_can_rx1_str_data(canbus, i, _temp_data);
			}

			/* Save the raw CAN data to shared memory. */
			shared_set_can_rx1_buffer_entry(canbus, i, &can_rx1[i]);
			shared_set_can_rx1_time_stamp(canbus, i, current_time);

			return;
		}
	}
	can_rx1[can_rx1_unique_ids] = _new_rx;
	can_rx1_timestamps[can_rx1_unique_ids] = current_time;
	can_rx1_unique_ids++;
	shared_set_can_rx1_unique_ids(canbus, can_rx1_unique_ids);
}

static void _fifo0_int_handler()
{
	xSemaphoreGiveFromISR(_rx_fifo0_counter, pdFALSE);
}

static void _fifo1_int_handler()
{
	xSemaphoreGiveFromISR(_rx_fifo1_counter, pdFALSE);
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

	_rx_fifo0_counter = xSemaphoreCreateCounting(CAN1_RX_FIFO0_ELEMENTS, 0);
	_rx_fifo1_counter = xSemaphoreCreateCounting(CAN1_RX_FIFO1_ELEMENTS, 0);

	/*Assign interrupt handler and enable new RX interrupt.*/
	can_assign_rx_rf0n_cb(FDCAN1, _fifo0_int_handler);
	can_assign_rx_rf1n_cb(FDCAN1, _fifo1_int_handler);
	can_enable_rx_rf0n_interrupt(FDCAN1);
	can_enable_rx_rf1n_interrupt(FDCAN1);

	can_run(FDCAN1);
	xTaskCreate(_task_can_transmit, "CAN_TX", 500, FDCAN1, 3, _tx_task_handle);
	xTaskCreate(_task_can_receive, "CAN_RX", 500, FDCAN1, 3, _rx_task_handle);

}

void app_can_sniffer_stop()
{
	_run = false;
	while ((_tx_task_handle != NULL) || (_rx_task_handle != NULL))
	{
		vTaskDelay(pdMS_TO_TICKS(50));
	}
	can_stop(FDCAN1);
	can_deinit(FDCAN1);
}

bool app_can_sniffer_running()
{
	return _run;
}
