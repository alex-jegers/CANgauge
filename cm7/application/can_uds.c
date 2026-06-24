/**********     INCLUDES        **********/
#include "application/applications_cm7.h"
#include "application/can_uds_def.h"
#include <stdio.h>				//For sprintf.
#include <string.h>				//For memcpy.
#include <stdbool.h>

/**********		DEFINES		**********/
#define EVENT_BITS_TASK_STOPPED			0x1 << 0
#define EVENT_BITS_INIT_DONE			0x1 << 1

/**********		TYPEDEFS 		**********/
typedef enum pci_flow_ctrl_t
{
	PCI_FLOW_CTRL_SF = 0x00,		//Single frame.
	PCI_FLOW_CTRL_FF,				//First frame.
	PCI_FLOW_CTRL_CF,				//Consecutive frame.
	PCI_FLOW_CTRL_FC,				//Flow control.
}pci_flow_ctrl_t;

/**********		EXTERNAL VARIABLE DEFINITIONS		**********/

/**********		STATIC VARIABLES		**********/
static bool prv_task_run = false;
static bool prv_accept_consecutive_frames = true;	//Whether or not the gauge will accept consecutive UDS frames.

/* Task handle. */
TaskHandle_t prv_task_handle;
EventGroupHandle_t prv_event_group = NULL;
SemaphoreHandle_t prv_rx_fifo1_counter = NULL;

static uint8_t prv_can_data[176][11];
static uint32_t prv_query_can_id = 0x00;
static uint32_t prv_response_can_id = 0x00;			//The ID that the car responds with when were checking for an ECU present.
static can_id_t prv_id_type = 0x00;

static uint8_t prv_flow_ctrl_ptr = 0;
static uint8_t prv_flow_ctrl_remaining_bytes = 0;

static can_transmit_handle_t* prv_current_data_query[4] = { NULL, NULL, NULL, NULL };		//Holds pointers to the data currently being transmitted (up to 4 parameters at a time).
/**********		STATIC FUNCTION DECLRATIONS		**********/
static void prv_task_can_controller();
static pci_flow_ctrl_t prv_process_can_data(can_rx_buffer_entry_t* buf);
/**
 * prv_get_available_pids:
 * desc:
 * 		Querys for a car on CAN bus at the requested CAN ID. Returns true
 * 		if it got a response. False if there was no response.
 */
static bool prv_get_available_pids(uint32_t can_id, can_id_t id_type);
/** 
 * prv_uds_ecu_present:
 * desc:
 * 		returns true if it found an uds session at the requested CAN ID.
 * 		false if not.
 */
static bool prv_uds_ecu_present(uint32_t can_id, can_id_t id_type);
static void prv_get_extra_pids(uint8_t pid);			//Will populate a nested PIDs availability.
static bool prv_update_available_uds_data();			//Checks to see if the CAN controller task found UDS data, returns false if there's nothing there.
static void prv_fifo1_int_handler();
static pci_flow_ctrl_t prv_get_flow_ctrl_info(can_rx_buffer_entry_t* buf);

/**********		STATIC FUNCTION DEFINITIONS		**********/

static void prv_task_can_controller(FDCAN_GlobalTypeDef* canbus)
{
	/* Zero out the CAN data array, this is where incoming raw data is stored. */
	memset(prv_can_data, 0xFF, 1760);

	/* Create counting semaphores to count how many CAN messages have been receieved. */
	prv_rx_fifo1_counter = xSemaphoreCreateCounting(64, 0);

	/*Assign interrupt handler and enable new RX interrupt.*/
	can_assign_rx_rf1n_cb(FDCAN1, prv_fifo1_int_handler);
	can_enable_rx_rf1n_interrupt(FDCAN1);

	/* Create a filter to store SAE J1979 messages in fifo1. */
	can_std_id_filter_t saej1979_std_filter;
	saej1979_std_filter.S0.bit.SFEC = CAN_SFEC_STORE_FIFO1;
	saej1979_std_filter.S0.bit.SFT = CAN_SFT_RANGE;
	saej1979_std_filter.S0.bit.SFID1 = 0x7e0;
	saej1979_std_filter.S0.bit.SFID2 = 0x7e8;
	can_set_std_id_filter(FDCAN1, 0, &saej1979_std_filter);

	can_ext_id_filter_t saej1979_ext_filter;
	saej1979_ext_filter.F0.bit.EFEC = CAN_EFEC_STORE_FIFO1;
	saej1979_ext_filter.F0.bit.EFID1 = 0x18DAF100;
	saej1979_ext_filter.F1.bit.EFT = CAN_EFT_RANGE_NO_MSK;
	saej1979_ext_filter.F1.bit.EFID2 = 0x18DAF1FF;
	can_set_ext_id_filter(FDCAN1, 0, &saej1979_ext_filter);

	vTaskDelay(pdMS_TO_TICKS(2000));		//Why is this here? Dont know that i need this.

	/* Check for an ECU at CAN ID 0x7DF. */
	if (prv_uds_ecu_present(0x7DF, CAN_ID_STD))
	{
		prv_query_can_id = 0x7DF;
		prv_id_type = CAN_ID_STD;
		
	}
	/* Check for an ECU at CAN ID 0x18DB33F1. */
	else if (prv_uds_ecu_present(0x18DB33F1, CAN_ID_XTD))
	{
		prv_query_can_id = 0x18DB33F1;
		prv_id_type = CAN_ID_XTD;
	}
	else
	{
		prv_query_can_id = 0x00;
		prv_task_run = false;
	}

	/* If the CAN ID isn't 0x00 (meaning it found an ECU, ask for all the other "available PIDs" PIDs.)*/
	if (prv_query_can_id != 0x00)
	{
		can_id_t can_id_type = (prv_query_can_id == 0x7DF) ? CAN_ID_STD : CAN_ID_XTD;
		prv_get_available_pids(prv_query_can_id, can_id_type);		//This just requests the "available PIDs" PIDs.
		prv_update_available_uds_data();						//This organizes the raw data into the array in can_uds_def.h and sets available to true where applicable.
	}

	/* Set the event bits that initialization is done. */
	xEventGroupSetBits(prv_event_group, EVENT_BITS_INIT_DONE);

	while(prv_task_run == true)
	{
		if (xSemaphoreTake(prv_rx_fifo1_counter, pdMS_TO_TICKS(1000)) == pdTRUE)
		{
			can_rx_buffer_entry_t buf;
			can_read_from_fifo1(FDCAN1, &buf);
			prv_process_can_data(&buf);	
		}
	}
	can_stop(FDCAN1);
	can_deinit(FDCAN1);
	xEventGroupSetBits(prv_event_group, EVENT_BITS_TASK_STOPPED);
	vTaskDelete(NULL);
}

static pci_flow_ctrl_t prv_process_can_data(can_rx_buffer_entry_t* buf)
{
			/* Determine if it's a single a frame, first frame, or consecutive frame. */
			pci_flow_ctrl_t frame_type = prv_get_flow_ctrl_info(buf);

			if (frame_type == PCI_FLOW_CTRL_SF)
			{
				uint8_t pid = buf->data[0x2];
				uint8_t a = buf->data[0x3];
				uint8_t b = buf->data[0x4];
				uint8_t c = buf->data[0x5];
				uint8_t d = buf->data[0x6];
				uint8_t e = buf->data[0x7];
				prv_can_data[pid][0] = a;
				prv_can_data[pid][1] = b;
				prv_can_data[pid][2] = c;
				prv_can_data[pid][3] = d;
				prv_can_data[pid][4] = e;
			}

			if (frame_type == PCI_FLOW_CTRL_FF)
			{
				uint32_t id = can_get_can_id(buf);
				id -= 8;
				prv_flow_ctrl_remaining_bytes = ((buf->data[0] & 0x0F) << 8) | buf->data[1];
				prv_flow_ctrl_ptr = buf->data[2];
				uint8_t pid = buf->data[0x3];
				prv_flow_ctrl_ptr = pid;
				uint8_t a = buf->data[0x4];
				uint8_t b = buf->data[0x5];
				uint8_t c = buf->data[0x6];
				uint8_t d = buf->data[0x7];
				prv_can_data[pid][0] = a;
				prv_can_data[pid][1] = b;
				prv_can_data[pid][2] = c;
				prv_can_data[pid][3] = d;

				can_tx_buffer_entry_t fc_continue_sending_frame =
				{
					.T0.bit.ID = id, .T0.bit.XTD = prv_id_type, .T0.bit.RTR = CAN_RTR_DATA_FRAME,
					.T1.bit.DLC = 8, .T1.bit.EFC = 0, .T1.bit.BRS = 0, .T1.bit.FDF = 0,
					.data[0] = 0x30, .data[1] = 0xFF, .data[2] = 0x00, .data[3] = 0xCC,
					.data[4] = 0xCC, .data[5] = 0xCC, .data[6] = 0xCC, .data[7] = 0xCC,
				};
				if (prv_accept_consecutive_frames == false)
				{
					fc_continue_sending_frame.data[0] = 0x32;
				}
				can_transmit_handle_t* tx_hndl = can_transmit_create_high_priority_msg();
				if (tx_hndl == NULL) { return frame_type; }	//TODO: Handle this better, this would indicate an error.
				can_transmit_set_msg_data(tx_hndl, &fc_continue_sending_frame);
				can_transmit_set_active(tx_hndl);
			}

			if (frame_type == PCI_FLOW_CTRL_CF)
			{
				uint8_t cfsn = buf->data[0] & 0xF;
				if (cfsn > 1)
				{
					return;
				}
				uint8_t e = buf->data[0x1];
				uint8_t f = buf->data[0x2];
				uint8_t g = buf->data[0x3];
				uint8_t h = buf->data[0x4];
				uint8_t i = buf->data[0x5];
				uint8_t j = buf->data[0x6];
				uint8_t k = buf->data[0x7];
				prv_can_data[prv_flow_ctrl_ptr][4] = e;
				prv_can_data[prv_flow_ctrl_ptr][5] = f;
				prv_can_data[prv_flow_ctrl_ptr][6] = g;
				prv_can_data[prv_flow_ctrl_ptr][7] = h;
				prv_can_data[prv_flow_ctrl_ptr][8] = i;
				prv_can_data[prv_flow_ctrl_ptr][9] = j;
				prv_can_data[prv_flow_ctrl_ptr][10] = k;	
			}
			return frame_type;
}

static bool prv_get_available_pids(uint32_t can_id, can_id_t id_type)
{
	can_tx_buffer_entry_t tx_buf =
	{
		.T0.bit.ID = can_id, .T0.bit.XTD = id_type, .T0.bit.RTR = CAN_RTR_DATA_FRAME,
		.T1.bit.DLC = 8, .T1.bit.EFC = 0, .T1.bit.BRS = 0, .T1.bit.FDF = 0,
		.data[0] = 0x02, .data[1] = 0x01, .data[2] = 0x00, .data[3] = 0xCC,
		.data[4] = 0xCC, .data[5] = 0xCC, .data[6] = 0xCC, .data[7] = 0xCC,
	};

	/* Request PIDs 0x00, 0x20, 0x40, 0x60, 0x80. */
	for (uint8_t i = 0; i < 0xA0; i += 0x20)
	{
		tx_buf.data[2] = i;
		can_transmit_handle_t* avail_pids = can_transmit_create_msg();
		can_transmit_set_msg_data(avail_pids, &tx_buf);
		can_transmit_set_period(avail_pids, CAN_TRANSMIT_PERIOD_ONE_SHOT);
		can_transmit_set_active(avail_pids);
	}

	/* Wait for up to a second to see if anything gets recieved. */
	for (uint8_t i = 0; i < 5; i++)
	{
		if (xSemaphoreTake(prv_rx_fifo1_counter, pdMS_TO_TICKS(1000)) == pdTRUE)
		{
			can_rx_buffer_entry_t rx_buf;
			can_read_from_fifo1(FDCAN1, &rx_buf);
			prv_process_can_data(&rx_buf);
		}
		else if (i == 0)	//Only return false if we dont get a response to 0x00.
		{
			return false;
		}	
	}
	return true;
}

static bool prv_uds_ecu_present(uint32_t can_id, can_id_t id_type)
{
	static uint8_t recursions = 0;
	can_tx_buffer_entry_t tx_buf =
	{
		.T0.bit.ID = can_id, .T0.bit.XTD = id_type, .T0.bit.RTR = CAN_RTR_DATA_FRAME,
		.T1.bit.DLC = 8, .T1.bit.EFC = 0, .T1.bit.BRS = 0, .T1.bit.FDF = 0,
		.data[0] = 0x02, .data[1] = 0x01, .data[2] = 0x00, .data[3] = 0xCC,
		.data[4] = 0xCC, .data[5] = 0xCC, .data[6] = 0xCC, .data[7] = 0xCC,
	};

	/* Send a request for PID 0x00. */
	tx_buf.data[2] = 0x00;
	can_transmit_handle_t* avail_pids = can_transmit_create_msg();
	can_transmit_set_msg_data(avail_pids, &tx_buf);
	can_transmit_set_period(avail_pids, CAN_TRANSMIT_PERIOD_ONE_SHOT);
	can_transmit_set_active(avail_pids);

	/* Wait a second to see if we get a response.*/
	if (xSemaphoreTake(prv_rx_fifo1_counter, pdMS_TO_TICKS(1000)) == pdTRUE)
	{
		/* Response received. Process it and return true. */
		can_rx_buffer_entry_t rx_buf;
		can_read_from_fifo1(FDCAN1, &rx_buf);
		pci_flow_ctrl_t frame_type = prv_process_can_data(&rx_buf);
		if (frame_type == PCI_FLOW_CTRL_SF)
		{
			prv_response_can_id = can_get_can_id(&rx_buf);
			return true;
		}
	}
	/* Only can reach here if rtn_val still == false. */
	if (recursions < 2)
	{
		recursions++;
		return prv_uds_ecu_present(can_id, id_type);
	}

	/* No valid response after 2 attempts. Return false. */
	recursions = 0;
	return false;

}
static void prv_get_extra_pids(uint8_t pid)
{
	can_tx_buffer_entry_t tx_buf =
	{
		.T0.bit.ID = prv_query_can_id, .T0.bit.XTD = prv_id_type, .T0.bit.RTR = CAN_RTR_DATA_FRAME,
		.T1.bit.DLC = 8, .T1.bit.EFC = 0, .T1.bit.BRS = 0, .T1.bit.FDF = 0,
		.data[0] = 0x02, .data[1] = 0x01, .data[2] = 0x00, .data[3] = 0xCC,
		.data[4] = 0xCC, .data[5] = 0xCC, .data[6] = 0xCC, .data[7] = 0xCC,
	};

	prv_accept_consecutive_frames = false;		//This is probably going to cause a multi frame message but we only want the first.

	tx_buf.data[2] = pid;
	can_transmit_handle_t* x = can_transmit_create_msg();
	can_transmit_set_msg_data(x, &tx_buf);
	can_transmit_set_period(x, CAN_TRANSMIT_PERIOD_ONE_SHOT);
	can_transmit_set_active(x);

	/* Wait a second to see if we get a response.*/
	if (xSemaphoreTake(prv_rx_fifo1_counter, pdMS_TO_TICKS(1000)) == pdTRUE)
	{
		/* Response received. Process it and return true. */
		can_rx_buffer_entry_t rx_buf;
		can_read_from_fifo1(FDCAN1, &rx_buf);
		prv_process_can_data(&rx_buf);

		saej1979_current_data_t* x = saej1979_get_current_data(pid);
		uint8_t bit_mask = can_controller_get_data(pid, 0, 1);
		for (uint8_t i = 0; i < 8; i++)
		{
			if ((1 << i) & bit_mask)
			{
				saej1979_current_data_t* y = x->nested[i];
				if (y != NULL) { y->available = true; }
			}
		}
	}
	prv_accept_consecutive_frames = true;		//Put this back to how it was.
}

static bool prv_update_available_uds_data()
{
	uint8_t num_params = 0;

	/* Check the available PID parameters (0x00, 0x20, 0x40, 0x60, 0x80.) */
	for (uint8_t x = 0; x < 0x80; x += 0x20)
	{
		uint32_t available_pids_1 = can_controller_get_data(x, 0, 4);
		for (int8_t i = 31; i >= 0; i--)
		{
			uint8_t this_pid = 32 - i + x;
			saej1979_current_data_t* y = saej1979_get_current_data(this_pid);
			if ((available_pids_1 & (1 << i)) != 0)
			{
				y->available = true;
				if (y->nested != NULL)
				{
					prv_get_extra_pids(this_pid);
				}
				num_params++;
			}
			else
			{
				y->available = false;
			}
		}
	}


	return num_params;
}

static void prv_fifo1_int_handler()
{
	xSemaphoreGiveFromISR(prv_rx_fifo1_counter, pdFALSE);
}

static pci_flow_ctrl_t prv_get_flow_ctrl_info(can_rx_buffer_entry_t* buf)
{
	return (buf->data[0] & 0xF0) >> 4;
}

/**********		GLOBAL FUNCTION DEFINITIONS		**********/
BaseType_t app_can_controller_run(uint8_t (*data_storage)[176][10])
{
	prv_task_run = true;

	if (prv_event_group == NULL)
	{
		prv_event_group = xEventGroupCreate();	
	}
	xEventGroupClearBits(prv_event_group, EVENT_BITS_TASK_STOPPED
											| EVENT_BITS_INIT_DONE);

	/* Create the task. */
	return xTaskCreate(prv_task_can_controller, "CAN_CONTROLLER", 1000 / 4, FDCAN1, 3, prv_task_handle);
	
}

bool app_can_controller_stop(uint32_t block_time_ms)
{
    prv_task_run = false;

    /* If the event group is NULL, the task was never even created in the first place. */
    if (prv_event_group == NULL)
    {
    	return pdTRUE;
    }

    uint32_t rtn = xEventGroupWaitBits(prv_event_group, EVENT_BITS_TASK_STOPPED,    //Bits to wait for.
                                        pdFALSE,        //Dont clear the bits on exit.
                                        pdTRUE,         //wait for all the bits (it's only 1)
                                        block_time_ms); //Block time.

    return rtn & EVENT_BITS_TASK_STOPPED;
}

uint32_t can_controller_get_data(uint8_t pid, uint8_t first_byte, uint8_t num_params)
{
	uint32_t raw_value = 0;
	for (uint8_t i = 0; i < num_params; i++)
	{
		raw_value |= prv_can_data[pid][i + first_byte] << ((num_params - (i+1)) * 8);
	}
	return raw_value;
}

bool app_can_sniffer_running()
{
	return prv_task_run;
}

bool app_can_controller_is_init(uint32_t block_time_ms )
{
	uint32_t rtn = xEventGroupWaitBits(prv_event_group, EVENT_BITS_INIT_DONE,
										pdFALSE,		//Clear on exit.
										pdTRUE,			//Wait for all.
										block_time_ms);	//Block time.
	return rtn & EVENT_BITS_INIT_DONE;
}

uint32_t app_can_controller_get_query_can_id()
{
	return prv_query_can_id;
}

uint32_t app_can_controller_get_response_can_id()
{
	return prv_response_can_id;
}

bool can_uds_set_current_data_query(uint8_t pid1, uint8_t pid2, uint8_t pid3, uint8_t pid4)
{
	bool rtn_val = false;
	/* Data byte 2 needs to be changed depending on the data being requested. */
	can_tx_buffer_entry_t iso15765_query =
	{
		.T0.bit.ID = prv_query_can_id, .T0.bit.XTD = prv_id_type, .T0.bit.RTR = CAN_RTR_DATA_FRAME,
		.T1.bit.DLC = 8, .T1.bit.EFC = 0, .T1.bit.BRS = 0, .T1.bit.FDF = 0,
		.data[0] = 0x02, .data[1] = 0x01, .data[2] = 0x00, .data[3] = 0xCC,
		.data[4] = 0xCC, .data[5] = 0xCC, .data[6] = 0xCC, .data[7] = 0xCC,
	};

	/* Create a CAN TX message if it hasnt been created already. */
	if (prv_current_data_query[0] == NULL)
	{
		prv_current_data_query[0] = can_transmit_create_msg();
		prv_current_data_query[1] = can_transmit_create_msg();
		prv_current_data_query[2] = can_transmit_create_msg();
		prv_current_data_query[3] = can_transmit_create_msg();
	}

	/* Check if there's a duplicate PID and remove it if there is. */
	uint8_t pid_arr[4] = { pid1, pid2, pid3, pid4 };
	for (uint8_t i = 0; i < 4; i++)
	{
		if (pid_arr[i] == 0)
		{
			continue;
		}
		for (uint8_t j = i + 1; j < 4; j++)
		{
			if (pid_arr[i] == pid_arr[j])
			{
				pid_arr[j] = 0;

				/* Move everything down and start again. */
				for (;j < 3; j++)
				{
					pid_arr[j] = pid_arr[j + 1];
				}
				pid_arr[3] = 0;
				j = i;
			}
		}
	}

	/* Determine how many PIDs are being passed in. */
	int8_t num_pids = 4;
	if (pid_arr[0] == 0) { return; }
	else if (pid_arr[1] == 0) { num_pids = 1; }
	else if (pid_arr[2] == 0) { num_pids = 2; }
	else if (pid_arr[3] == 0) { num_pids = 3; }

	/* These set the data field. */
	for (;num_pids > 0; num_pids--)
	{
		switch (num_pids - 1)
		{
		case 0:
			iso15765_query.data[2] = pid_arr[0];
			break;
		case 1:
			iso15765_query.data[2] = pid_arr[1];
			break;
		case 2:
			iso15765_query.data[2] = pid_arr[2];
			break;
		case 3:
			iso15765_query.data[2] = pid_arr[3];
			break;
		}
		/* Add the CAN message, set its transmit period, and activate it. */
		can_transmit_set_msg_data(prv_current_data_query[num_pids - 1], &iso15765_query);
		can_transmit_set_period(prv_current_data_query[num_pids - 1], 25);
		can_transmit_set_active(prv_current_data_query[num_pids - 1]);
		rtn_val = true;
	}

	return rtn_val;
}

void can_uds_stop_query()
{
	/* Stop transmitting the requestor on CAN. */
	can_transmit_set_inactive(prv_current_data_query[0]);
	can_transmit_set_inactive(prv_current_data_query[1]);
	can_transmit_set_inactive(prv_current_data_query[2]);
	can_transmit_set_inactive(prv_current_data_query[3]);
}
