/**********     INCLUDES        **********/
#include <application/app_can_controller.h>
#include <cangauge_common.h>
#include "drivers/stm32_hsem.h"
#include <stdio.h>				//For sprintf.
#include <string.h>				//For memcpy.
#include <stdbool.h>

/**********		DEFINES		**********/
typedef enum
{
	PCI_FLOW_CTRL_SF = 0x00,		//Single frame.
	PCI_FLOW_CTRL_FF,				//First frame.
	PCI_FLOW_CTRL_CF,				//Consecutive frame.
	PCI_FLOW_CTRL_FC,				//Flow control.
}pci_flow_ctrl_t;

/**********		EXTERNAL VARIABLE DEFINITIONS		**********/

/**********		STATIC VARIABLES		**********/
static bool prv_task_run = false;
static bool prv_init_done = false;

/* Task handle. */
TaskHandle_t prv_task_handle;

SemaphoreHandle_t prv_rx_fifo1_counter = NULL;

static uint8_t (*prv_can_data)[176][10];

static uint8_t prv_flow_ctrl_ptr = 0;
static uint8_t prv_flow_ctrl_remaining_bytes = 0;
/**********		STATIC FUNCTION DECLRATIONS		**********/
static void prv_task_can_controller();
		
static void prv_get_available_pids();
static void prv_get_extra_pids();
static void prv_fifo1_int_handler();
static pci_flow_ctrl_t prv_get_flow_ctrl_info(can_rx_buffer_entry_t* buf);

/**********		STATIC FUNCTION DEFINITIONS		**********/

static void prv_task_can_controller(FDCAN_GlobalTypeDef* canbus)
{
	/* Create a filter to store SAE J1979 messages in fifo1. */
	can_std_id_filter_t _saej1979_filter;
	_saej1979_filter.S0.bit.SFEC = CAN_SFEC_STORE_FIFO1;
	_saej1979_filter.S0.bit.SFT = CAN_SFT_RANGE;
	_saej1979_filter.S0.bit.SFID1 = 0x7e0;
	_saej1979_filter.S0.bit.SFID2 = 0x7e8;
	can_set_std_id_filter(FDCAN1, 0, &_saej1979_filter);

	prv_get_available_pids();
	vTaskDelay(pdMS_TO_TICKS(50));
	prv_init_done = true;
	//prv_get_extra_pids();

	while(prv_task_run == true)
	{
		if (xSemaphoreTake(prv_rx_fifo1_counter, pdMS_TO_TICKS(1000)) == pdTRUE)
		{
			can_rx_buffer_entry_t buf;
			can_read_from_fifo1(FDCAN1, &buf);

			/* Determine if it's a single a frame, first frame, or consecutive frame. */
			pci_flow_ctrl_t frame_type = prv_get_flow_ctrl_info(&buf);

			if (frame_type == PCI_FLOW_CTRL_SF)
			{
				uint8_t pid = buf.data[0x2];
				uint8_t a = buf.data[0x3];
				uint8_t b = buf.data[0x4];
				uint8_t c = buf.data[0x5];
				uint8_t d = buf.data[0x6];
				uint8_t e = buf.data[0x7];
				(*prv_can_data)[pid][0] = a;
				(*prv_can_data)[pid][1] = b;
				(*prv_can_data)[pid][2] = c;
				(*prv_can_data)[pid][3] = d;
				(*prv_can_data)[pid][4] = e;
			}

			if (frame_type == PCI_FLOW_CTRL_FF)
			{
				uint32_t id = buf.R0.bit.ID >> 18;
				id -= 8;
				prv_flow_ctrl_remaining_bytes = ((buf.data[0] & 0x0F) << 8) | buf.data[1];
				prv_flow_ctrl_ptr = buf.data[2];
				uint8_t pid = buf.data[0x3];
				prv_flow_ctrl_ptr = pid;
				uint8_t a = buf.data[0x4];
				uint8_t b = buf.data[0x5];
				uint8_t c = buf.data[0x6];
				uint8_t d = buf.data[0x7];
				(*prv_can_data)[pid][0] = a;
				(*prv_can_data)[pid][1] = b;
				(*prv_can_data)[pid][2] = c;
				(*prv_can_data)[pid][3] = d;

				can_tx_buffer_entry_t fc_continue_sending_frame =
				{
					.T0.bit.ID = id << 18, .T0.bit.XTD = CAN_ID_STD, .T0.bit.RTR = CAN_RTR_DATA_FRAME,
					.T1.bit.DLC = 8, .T1.bit.EFC = 0, .T1.bit.BRS = 0, .T1.bit.FDF = 0,
					.data[0] = 0x30, .data[1] = 0xFF, .data[2] = 0x00, .data[3] = 0xCC,
					.data[4] = 0xCC, .data[5] = 0xCC, .data[6] = 0xCC, .data[7] = 0xCC,
				};
				can_add_tx_buffer(FDCAN1, &fc_continue_sending_frame, 31);
				can_tx(FDCAN1, 31);
			}

			if (frame_type == PCI_FLOW_CTRL_CF)
			{
				uint8_t cfsn = buf.data[0] & 0xF;
				if (cfsn > 1)
				{
					return;
				}
				uint8_t e = buf.data[0x1];
				uint8_t f = buf.data[0x2];
				uint8_t g = buf.data[0x3];
				uint8_t h = buf.data[0x4];
				uint8_t i = buf.data[0x5];
				uint8_t j = buf.data[0x6];
				uint8_t k = buf.data[0x7];
				(*prv_can_data)[prv_flow_ctrl_ptr][4] = e;
				(*prv_can_data)[prv_flow_ctrl_ptr][5] = f;
				(*prv_can_data)[prv_flow_ctrl_ptr][6] = g;
				(*prv_can_data)[prv_flow_ctrl_ptr][7] = h;
				(*prv_can_data)[prv_flow_ctrl_ptr][8] = i;
				(*prv_can_data)[prv_flow_ctrl_ptr][9] = j;
				(*prv_can_data)[prv_flow_ctrl_ptr][10] = k;
			}
		}
	}

	vTaskDelete(NULL);
}

static void prv_get_available_pids()
{
	/* Request PIDs 0x00, 0x20, 0x40. */
	can_tx_buffer_entry_t tx_buf =
	{
		.T0.bit.ID = 0x7DF << 18, .T0.bit.XTD = CAN_ID_STD, .T0.bit.RTR = CAN_RTR_DATA_FRAME,
		.T1.bit.DLC = 8, .T1.bit.EFC = 0, .T1.bit.BRS = 0, .T1.bit.FDF = 0,
		.data[0] = 0x02, .data[1] = 0x01, .data[2] = 0x00, .data[3] = 0xCC,
		.data[4] = 0xCC, .data[5] = 0xCC, .data[6] = 0xCC, .data[7] = 0xCC,
	};
	tx_buf.data[2] = 0x00;
	can_add_tx_buffer(FDCAN1, &tx_buf, 31);
	can_tx(FDCAN1, 31);
	vTaskDelay(pdMS_TO_TICKS(50));

	tx_buf.data[2] = 0x20;
	can_add_tx_buffer(FDCAN1, &tx_buf, 30);
	can_tx(FDCAN1, 30);
	vTaskDelay(pdMS_TO_TICKS(50));

	tx_buf.data[2] = 0x40;
	can_add_tx_buffer(FDCAN1, &tx_buf, 29);
	can_tx(FDCAN1, 29);
	vTaskDelay(pdMS_TO_TICKS(50));

	tx_buf.data[2] = 0x60;
	can_add_tx_buffer(FDCAN1, &tx_buf, 28);
	can_tx(FDCAN1, 28);
	vTaskDelay(pdMS_TO_TICKS(50));
}

static void prv_get_extra_pids()
{
	can_tx_buffer_entry_t tx_buf =
	{
		.T0.bit.ID = 0x7DF << 18, .T0.bit.XTD = CAN_ID_STD, .T0.bit.RTR = CAN_RTR_DATA_FRAME,
		.T1.bit.DLC = 8, .T1.bit.EFC = 0, .T1.bit.BRS = 0, .T1.bit.FDF = 0,
		.data[0] = 0x02, .data[1] = 0x01, .data[2] = 0x00, .data[3] = 0xCC,
		.data[4] = 0xCC, .data[5] = 0xCC, .data[6] = 0xCC, .data[7] = 0xCC,
	};

	tx_buf.data[2] = 0x70;
	can_add_tx_buffer(FDCAN1, &tx_buf, 28);
	can_tx(FDCAN1, 28);
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
void app_can_controller_run(uint8_t (*data_storage)[176][10])
{
	prv_task_run = true;
	prv_init_done = false;

	prv_can_data = data_storage;
	memset(prv_can_data, 0, 1760);

	/* Create counting semaphores to count how many CAN messages have been receieved. */
	prv_rx_fifo1_counter = xSemaphoreCreateCounting(CAN1_RX_FIFO1_ELEMENTS, 0);

	/*Assign interrupt handler and enable new RX interrupt.*/
	can_assign_rx_rf1n_cb(FDCAN1, prv_fifo1_int_handler);
	can_enable_rx_rf1n_interrupt(FDCAN1);

	/* Check if something already started CAN, and start it if not. */
	if(hsem_lock(31,0) == true)
	{
        can_init(FDCAN1);
        can_set_baud_rate(FDCAN1, CAN_BAUD_500K);
        can_run(FDCAN1);
	}

	/* Create the task. */
	xTaskCreate(prv_task_can_controller, "CAN_CONTROLLER", 500, FDCAN1, 3, prv_task_handle);
	
}

void app_can_controller_stop()
{
	prv_task_run = false;
	while (prv_task_handle != NULL)
	{
		vTaskDelay(pdMS_TO_TICKS(50));
	}
	can_stop(FDCAN1);
	can_deinit(FDCAN1);
}

bool app_can_sniffer_running()
{
	return prv_task_run;
}

bool app_can_controller_is_init()
{
	return prv_init_done;
}
