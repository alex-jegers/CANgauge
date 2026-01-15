/**********     INCLUDES        **********/
#include <application/app_can_controller_cm4.h>
#include <shared_mem.h>
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
TaskHandle_t _process_rx_task_handle = NULL;

SemaphoreHandle_t _rx_fifo0_counter = NULL;
SemaphoreHandle_t _rx_fifo1_counter = NULL;

struct can_rx_raw_data
{
	can_rx_buffer_entry_t buf;
	uint32_t timestamp;
	uint32_t period;
	uint8_t unique_ids;		//Only use index 0.
};

static struct can_rx_raw_data can_raw_rx0[CAN1_RX_FIFO0_ELEMENTS];
static struct can_rx_raw_data can_raw_rx1[CAN1_RX_FIFO1_ELEMENTS];




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
static void _read_from_fifo(FDCAN_GlobalTypeDef* canbus, struct can_rx_raw_data* can_raw_rx);
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

	/* Create a filter to store SAE J1979 messages in fifo1. */
	can_std_id_filter_t _saej1979_filter;
	_saej1979_filter.S0.bit.SFEC = CAN_SFEC_STORE_FIFO1;
	_saej1979_filter.S0.bit.SFT = CAN_SFT_RANGE;
	_saej1979_filter.S0.bit.SFID1 = 0x7E8;
	_saej1979_filter.S0.bit.SFID2 = 0x7EF;
	can_set_std_id_filter(FDCAN1, 0, &_saej1979_filter);

	while(_run == true)
	{

		bool empty = true;
		
		/* If there's data in FIFO0, read it. */
		if (xSemaphoreTake(_rx_fifo0_counter, 0) == pdPASS)
		{
 			_read_from_fifo(FDCAN1, can_raw_rx0);
			empty = false;
		}
		
		/* If there's data in FIFO1, read it. */
		if (xSemaphoreTake(_rx_fifo1_counter, 0) == pdPASS)
		{
			_read_from_fifo(FDCAN1, can_raw_rx1);
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
		memset(&can_raw_rx0, 0, sizeof(can_raw_rx0));
	}
	for (uint8_t i = 0; i < shared_get_can_rx1_unique_ids(canbus); i++)
	{
		memset(&can_raw_rx1, 0, sizeof(can_raw_rx1));
	}

	shared_set_can_rx0_unique_ids(canbus, 0);
	shared_set_can_rx1_unique_ids(canbus, 0);
	vTaskDelete(NULL);
}

static void _task_can_process(FDCAN_GlobalTypeDef* canbus)
{
	while (_run)
	{
		/* Check to see if any are expired in FIFO0. */
		for (uint8_t i = 0; i < can_raw_rx0[0].unique_ids; i++)
		{
			BaseType_t current_time = pdTICKS_TO_MS(xTaskGetTickCount());
			uint32_t time_since_last = current_time - can_raw_rx0[i].timestamp;
			if (time_since_last > 3000)
			{
				for (uint8_t x = i; x < can_raw_rx0[0].unique_ids; x++)
				{
					can_raw_rx0[x + 1].unique_ids = can_raw_rx0[x].unique_ids;	//Need this line in case x = 0 we wont overwrite unique IDs.
					can_raw_rx0[x] = can_raw_rx0[x + 1];
				}
				can_raw_rx0[0].unique_ids--;
			}
		}

		/* Check to see if any are expired in FIFO1. */
		for (uint8_t i = 0; i < can_raw_rx1[0].unique_ids; i++)
		{
			BaseType_t current_time = pdTICKS_TO_MS(xTaskGetTickCount());
			uint32_t time_since_last = current_time - can_raw_rx1[i].timestamp;
			if (time_since_last > 3000)
			{
				for (uint8_t x = i; x < can_raw_rx1[0].unique_ids; x++)
				{
					can_raw_rx1[x + 1].unique_ids = can_raw_rx1[x].unique_ids;	//Need this line in case x = 0 we wont overwrite unique IDs.
					can_raw_rx1[x] = can_raw_rx1[x + 1];
				}
				can_raw_rx1[0].unique_ids--;
			}
		}

		/* Process FIFO0 data into shared memory. */
		for (uint8_t i = 0; i < can_raw_rx0[0].unique_ids; i++)
		{
			// TODO: Delete this below? Already checking for expiration above.
			/* Check to see if any are expired (havent been RX'd in 3 seconds or longer). */
			BaseType_t current_time = pdTICKS_TO_MS(xTaskGetTickCount());
			uint32_t time_since_last = current_time - can_raw_rx0[i].timestamp;
			if (time_since_last > 3000)
			{
				for (uint8_t x = i; x < can_raw_rx0[0].unique_ids; x++)
				{
					can_raw_rx0[x] = can_raw_rx0[x + 1];
				}
				can_raw_rx0[0].unique_ids--;
			}

			/* Convert period to a string and write to shared memory. */
			char _temp_period_ms[10];
			sprintf(_temp_period_ms, "%d", can_raw_rx0[i].period);
			shared_set_can_rx0_str_period(canbus, i, _temp_period_ms);

			/* Convert the ID to a string and write to shared memory. */
			char _temp_id[8] = {0,0,0,0,0,0,0,0};
			sprintf(_temp_id, "%X", can_raw_rx0[i].buf.R0.bit.ID);		//Convert the ID into a hex string.
			shared_set_can_rx0_str_id(canbus, i, _temp_id);				//Write that string into shared mem for CM7 to get.

			/*Convert all the data bytes into hex strings.*/
			char _temp_data[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};	//Need to write to _temp first because sprintf writes a null terminator which overwrites data in can_data_str.
			uint8_t _str_counter = 0;
			for (uint8_t x = 0; x < can_raw_rx0[i].buf.R1.bit.DLC; x++)
			{
				_str_counter += sprintf(&_temp_data[_str_counter], "%X", can_raw_rx0[i].buf.data[x]);
				shared_set_can_rx0_str_data(canbus, i, _temp_data);
			}

			/* Save the raw CAN data to shared memory. */
			shared_set_can_rx0_buffer_entry(canbus, i, &can_raw_rx0[i].buf);
			shared_set_can_rx0_time_stamp(canbus, i, can_raw_rx0[i].timestamp);
			shared_set_can_rx0_unique_ids(canbus, can_raw_rx0[0].unique_ids);
		}	

		/* Process FIFO1 data into shared memory. */
		for (uint8_t i = 0; i < can_raw_rx1[0].unique_ids; i++)
		{
			char _temp_period_ms[10];
			sprintf(_temp_period_ms, "%d", can_raw_rx1[i].period);
			shared_set_can_rx1_str_period(canbus, i, _temp_period_ms);

			char _temp_id[8] = {0,0,0,0,0,0,0,0};
			sprintf(_temp_id, "%X", can_raw_rx0[i].buf.R0.bit.ID);				//Convert the ID into a hex string.
			shared_set_can_rx1_str_id(canbus, i, _temp_id);						//Write that string into shared mem for CM7 to get.

			/*Convert all the data bytes into hex strings.*/
			char _temp_data[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};	//Need to write to _temp first because sprintf writes a null terminator which overwrites data in can_data_str.
			uint8_t _str_counter = 0;
			for (uint8_t x = 0; x < can_raw_rx1[i].buf.R1.bit.DLC; x++)
			{
				_str_counter += sprintf(&_temp_data[_str_counter], "%X", can_raw_rx1[i].buf.data[x]);
				shared_set_can_rx1_str_data(canbus, i, _temp_data);
			}

			/* Save the raw CAN data to shared memory. */
			shared_set_can_rx1_buffer_entry(canbus, i, &can_raw_rx1[i].buf);
			shared_set_can_rx1_time_stamp(canbus, i, can_raw_rx1[i].timestamp);
			shared_set_can_rx1_unique_ids(canbus, can_raw_rx1[0].unique_ids);
		}
		vTaskDelay(25);	
	}

}

static void _read_from_fifo(FDCAN_GlobalTypeDef* canbus, struct can_rx_raw_data* can_raw_rx)
{
	can_rx_buffer_entry_t _new_rx;
	if (can_raw_rx == can_raw_rx0)
	{
		can_read_from_fifo0(canbus, &_new_rx);
	}
	if (can_raw_rx == can_raw_rx1)
	{
		can_read_from_fifo1(canbus, &_new_rx);
	}


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
	for (uint8_t i = 0; i < can_raw_rx[0].unique_ids; i++)				//Iterate through however many unique IDs have been received.
	{

		if (_new_rx.R0.bit.ID == can_raw_rx[i].buf.R0.bit.ID)				//Check the recieeved ID against all the IDs that have been rx'd.
		{
			can_raw_rx[i].buf = _new_rx;									//Keep a local numerical copy.
			can_raw_rx[i].period = current_time - can_raw_rx[i].timestamp;	//Calculate the period.
			can_raw_rx[i].timestamp = current_time;							//Save the current time.
			return;
		}
	}
	uint8_t unique_ids = can_raw_rx[0].unique_ids;
	can_raw_rx[unique_ids].buf = _new_rx;
	can_raw_rx[unique_ids].timestamp = current_time;
	can_raw_rx[0].unique_ids++;
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
void app_can_controller_run()
{
	_run = true;

	/* If a baud rate hasnt been determined yet, wait. */
	while ((shared_get_can_baud_rate(FDCAN1) == CAN_BAUD_ERROR) && (shared_get_can_baud_rate_override(FDCAN1) == CAN_BAUD_ERROR))
	{
		vTaskDelay(pdMS_TO_TICKS(100));
	}

	/* If something is already using CAN, wait. */
	while (can_take(FDCAN1) == false)
	{
		vTaskDelay(pdMS_TO_TICKS(100));
	}

	/* Initialize the CAN driver. */
	can_init(FDCAN1);

	/* Set the baud rate. */
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
	/* Should never reach here. */
	else
	{
		shared_set_can_error(FDCAN1, true);
		vTaskDelete(NULL);
	}

	/* Create counting semaphores to count how many CAN messages have been receieved. */
	_rx_fifo0_counter = xSemaphoreCreateCounting(CAN1_RX_FIFO0_ELEMENTS, 0);
	_rx_fifo1_counter = xSemaphoreCreateCounting(CAN1_RX_FIFO1_ELEMENTS, 0);

	/* Set unique IDs to zero. */
	can_raw_rx0[0].unique_ids = 0;
	can_raw_rx1[0].unique_ids = 0;

	/*Assign interrupt handler and enable new RX interrupt.*/
	can_assign_rx_rf0n_cb(FDCAN1, _fifo0_int_handler);
	can_assign_rx_rf1n_cb(FDCAN1, _fifo1_int_handler);
	can_enable_rx_rf0n_interrupt(FDCAN1);
	can_enable_rx_rf1n_interrupt(FDCAN1);

	/* Start the CAN driver. */
	can_run(FDCAN1);

	/* Create the tasks. */
	xTaskCreate(_task_can_transmit, "CAN_TX", 500, FDCAN1, 3, _tx_task_handle);
	xTaskCreate(_task_can_receive, "CAN_RX", 500, FDCAN1, 3, _rx_task_handle);
	xTaskCreate(_task_can_process, "CAN_PROCESS_RX", 500, FDCAN1, 3, _process_rx_task_handle);
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
