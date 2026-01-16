/**********     INCLUDES        **********/
#include <application/app_can_controller_cm4.h>
#include <cangauge_common.h>
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
		while (1/*Get # of unique IDs.*/)
		{
			vTaskDelay(pdMS_TO_TICKS(500));
		}

		/*If there are IDs to transmit in shared memory, loop through them.*/
		for (uint8_t i = 0; i < 0/*To total # of IDs.*/; i++)
		{
			uint32_t interval = 0;//shared_get_tx_interval(i);

			/* If the interval = 0, it's only sent on command so skip it. */
			if (interval == 0)
			{
				continue;
			}

			uint32_t last_time_sent = 0;//shared_get_tx_last_time_sent(i);
			uint32_t time_now = 0;//pdTICKS_TO_MS(xTaskGetTickCount());
			uint32_t time_since = 0;//time_now - last_time_sent;

			/* Check to see if it should be transmitted. */
			if (time_since > interval)		//If time since the last transmission is greater than its interval...
			{
				can_tx(canbus, i);			//Transmit the message.
				last_time_sent = time_now;	//Save the current time.
				time_since = 0;				//Set time since last to 0.
				//shared_set_tx_time_last_sent(i, time_now);	//Save the last time sent to shared memory.
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

		vTaskDelay(25);	
	}

}

static void _read_from_fifo(FDCAN_GlobalTypeDef* canbus, struct can_rx_raw_data* can_raw_rx)
{

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
	/*
	xTaskCreate(_task_can_transmit, "CAN_TX", 500, FDCAN1, 3, _tx_task_handle);
	xTaskCreate(_task_can_receive, "CAN_RX", 500, FDCAN1, 3, _rx_task_handle);
	xTaskCreate(_task_can_process, "CAN_PROCESS_RX", 500, FDCAN1, 3, _process_rx_task_handle);
	*/
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
