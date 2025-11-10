/*
 * system_shared_mem.c
 *
 *  Created on: Jun 1, 2024
 *      Author: awjpp
 */
/**********     INCLUDES        **********/
#include "app_shared_mem.h"
#include "drivers/stm32_canbus.h"
#include "string.h"					//for memcpy.

/**********		DEFINES		**********/

/**********		EXTERNAL VARIABLE DEFINITIONS		**********/

/**********		STATIC VARIABLE DECLARATIONS		**********/
struct can_str_data
{
	char ids[9];			//Only uses 8 but needs to be size 9 for the null terminator.
	char data[17];			//Only uses size 16 but needs to be 17 for the null terminator.
	char period_ms[10];
};

struct can_tx_data
{
	uint32_t interval_ms;
	uint32_t last_time_sent_ms;
};

typedef struct
{
	/* Data for the touch screen, CM4 writes, CM7 reads. */
	bool cst830_is_touched;						//True or false, the touch screen is being pressed.
	uint16_t cst830_pos_x;						//The x position of the touch screen press.
	uint16_t cst830_pos_y;						//The y position of the touch screen press.

	/* General CANbus data. CM4 writes, CM7 reads. */
	can_baud_rate_t hs_can_baud;				//speed of the HS CANbus.
	can_baud_rate_t ls_can_baud;				//speed of the LS CANbus.
	bool can_error;
	can_baud_rate_t hs_can_baud_override;		//manually request a baud rate for FDCAN1. Set to CAN_BAUD_ERROR for auto detect.
	can_baud_rate_t ls_can_baud_override;		//manually request a baud rate for FDCAN2. Set to CAN_BAUD_ERROR for auto detect.
	FDCAN_GlobalTypeDef* target_canbus;			//which CAN is being referenced for any given command.

	/* Data for CAN sniffer app. */
	can_rx_buffer_entry_t can_rx[20];			//Hold all the data being received.
	uint32_t can_rx_timestamps[20];				//Timestamps of data being received.
	struct can_str_data can1_rx_str_data[20];	//Data from FDCAN1 formatted as strings for CM7.
	uint32_t can1_rx_unique_ids;				//How many messages have been rx'd in CAN1.
	struct can_str_data can2_str_data[20];		//Copy of above for CAN2
	uint32_t can2_unique_ids;					//Copy of above for CAN2

	struct can_tx_data can1_tx[CAN1_TX_BUFFER_ELEMENTS];	//Holds the raw data to be transmitted.
	uint8_t can1_tx_unique_ids;								//How many individual messages are being used.

}shared_mem_t;

/**********		STATIC VARIABLE DEFINITIONS		**********/
//static volatile shared_mem_t* const app_shared_mem = (shared_mem_t*)0xD02A3000;
__attribute__((__section__(".shared_data"))) static volatile shared_mem_t app_shared_mem;
/**********		STATIC FUNCTION DECLRATIONS		**********/

/**********		STATIC FUNCTION DEFINITIONS		**********/

/**********		GLOBAL FUNCTION DEFINITIONS		**********/
void system_init_shared_mem()
{
	memset(&app_shared_mem, 0, sizeof(shared_mem_t));

	app_shared_mem.hs_can_baud = CAN_BAUD_ERROR;
	app_shared_mem.ls_can_baud = CAN_BAUD_ERROR;
	app_shared_mem.can_error = false;
	app_shared_mem.hs_can_baud_override = CAN_BAUD_ERROR;
	app_shared_mem.ls_can_baud_override = CAN_BAUD_ERROR;
}

void shared_mem_set_cst830_touch_data(bool pressed, uint16_t x_pos, uint16_t y_pos)
{
	app_shared_mem.cst830_is_touched = pressed;
	app_shared_mem.cst830_pos_x = x_pos;
	app_shared_mem.cst830_pos_y = y_pos;
}

void shared_set_can_baud(FDCAN_GlobalTypeDef* canbus, can_baud_rate_t baud_rate)
{
	if (canbus == HS_CAN)
	{
		app_shared_mem.hs_can_baud = baud_rate;
	}

	if (canbus == LS_CAN)
	{
		app_shared_mem.ls_can_baud = baud_rate;
	}
}

void shared_set_can_baud_override(FDCAN_GlobalTypeDef* canbus, can_baud_rate_t baud_rate)
{
	if (canbus == HS_CAN)
	{
		app_shared_mem.hs_can_baud_override = baud_rate;
	}

	if (canbus == LS_CAN)
	{
		app_shared_mem.ls_can_baud_override = baud_rate;
	}
}

void shared_set_can_str_id(FDCAN_GlobalTypeDef* canbus, uint32_t id_index, char* id)
{
	if (canbus == HS_CAN)
	{
		memcpy(&app_shared_mem.can1_rx_str_data[id_index].ids, id, sizeof(app_shared_mem.can1_rx_str_data[id_index].ids));
	}
	if (canbus == LS_CAN)
	{
		memcpy(&app_shared_mem.can2_str_data[id_index].ids, id, sizeof(app_shared_mem.can2_str_data[id_index].ids));
	}
}

void shared_set_can_str_data(FDCAN_GlobalTypeDef* canbus, uint32_t id_index, char* data)
{
	if (canbus == HS_CAN)
	{
		memcpy(&app_shared_mem.can1_rx_str_data[id_index].data, data, sizeof(app_shared_mem.can1_rx_str_data[id_index].data));
	}
	if (canbus == LS_CAN)
	{
		memcpy(&app_shared_mem.can2_str_data[id_index].data, data, sizeof(app_shared_mem.can2_str_data[id_index].data));
	}
}

void shared_set_can_str_period(FDCAN_GlobalTypeDef* canbus, uint32_t id_index, char* period)
{
	if (canbus == HS_CAN)
	{
		memcpy(&app_shared_mem.can1_rx_str_data[id_index].period_ms, period, sizeof(app_shared_mem.can1_rx_str_data[id_index].period_ms));
	}
	if (canbus == LS_CAN)
	{
		memcpy(&app_shared_mem.can2_str_data[id_index].period_ms, period, sizeof(app_shared_mem.can2_str_data[id_index].period_ms));
	}
}

void shared_set_can_unique_ids(FDCAN_GlobalTypeDef* canbus, uint8_t num_ids)
{
	if (canbus == HS_CAN)
	{
		app_shared_mem.can1_rx_unique_ids = num_ids;
	}
	if (canbus == LS_CAN)
	{
		app_shared_mem.can2_unique_ids = num_ids;
	}
}

void shared_set_target_can(FDCAN_GlobalTypeDef* canbus)
{
	app_shared_mem.target_canbus = canbus;
}

void shared_set_can_error(FDCAN_GlobalTypeDef* canbus, bool status)
{
	app_shared_mem.can_error = status;
}

void shared_set_tx_unique_ids(FDCAN_GlobalTypeDef* canbus, uint32_t ids)
{
	app_shared_mem.can1_tx_unique_ids = ids;
}

void shared_set_tx_time_last_sent(uint8_t index, uint32_t time_ms)
{
	app_shared_mem.can1_tx[index].last_time_sent_ms = time_ms;
}

/************************************************************************************/
can_baud_rate_t shared_get_can_baud_rate(FDCAN_GlobalTypeDef* canbus)
{
	if (canbus == HS_CAN)
	{
		return app_shared_mem.hs_can_baud;
	}

	if (canbus == LS_CAN)
	{
		return app_shared_mem.ls_can_baud;
	}

	return CAN_BAUD_ERROR;
}

can_baud_rate_t shared_get_can_baud_rate_override(FDCAN_GlobalTypeDef* canbus)
{
	if (canbus == HS_CAN)
	{
		return app_shared_mem.hs_can_baud_override;
	}

	if (canbus == LS_CAN)
	{
		return app_shared_mem.ls_can_baud_override;
	}

	return CAN_BAUD_ERROR;
}

bool shared_mem_get_cst830_is_pressed()
{
	return app_shared_mem.cst830_is_touched;
}

uint16_t shared_mem_get_cst830_pos_x()
{
	return app_shared_mem.cst830_pos_x;
}

uint16_t shared_mem_get_cst830_pos_y()
{
	return app_shared_mem.cst830_pos_y;
}

char* shared_get_can_str_id(FDCAN_GlobalTypeDef* canbus, uint8_t id_index)
{
	if (canbus == HS_CAN)
	{
		return app_shared_mem.can1_rx_str_data[id_index].ids;
	}
	if (canbus == LS_CAN)
	{
		return app_shared_mem.can2_str_data[id_index].ids;
	}
	return NULL;
}

char* shared_get_can_str_data(FDCAN_GlobalTypeDef* canbus, uint8_t id_index)
{
	if (canbus == HS_CAN)
	{
		return app_shared_mem.can1_rx_str_data[id_index].data;
	}
	if (canbus == LS_CAN)
	{
		return app_shared_mem.can2_str_data[id_index].data;
	}
	return NULL;
}

char* shared_get_can_str_period(FDCAN_GlobalTypeDef* canbus, uint8_t id_index)
{
	if (canbus == HS_CAN)
	{
		return app_shared_mem.can1_rx_str_data[id_index].period_ms;
	}
	if (canbus == LS_CAN)
	{
		return app_shared_mem.can2_str_data[id_index].period_ms;
	}
	return NULL;
}

uint8_t shared_get_can_unique_ids(FDCAN_GlobalTypeDef* canbus)
{
	if (canbus == HS_CAN)
	{
		return app_shared_mem.can1_rx_unique_ids;
	}
	if (canbus == LS_CAN)
	{
		return app_shared_mem.can2_unique_ids;
	}
	return 0;
}

FDCAN_GlobalTypeDef* shared_get_target_can()
{
	return app_shared_mem.target_canbus;
}

bool shared_get_can_error()
{
	return app_shared_mem.can_error;
}

uint32_t shared_get_tx_unique_ids(FDCAN_GlobalTypeDef* canbus)
{
	return app_shared_mem.can1_tx_unique_ids;
}

void shared_set_can_tx_data(uint32_t interval, uint8_t index)
{
	app_shared_mem.can1_tx[index].interval_ms = interval;
}

uint32_t shared_get_tx_interval(uint32_t index)
{
	return app_shared_mem.can1_tx[index].interval_ms;
}

uint32_t shared_get_tx_last_time_sent(uint32_t index)
{
	return app_shared_mem.can1_tx[index].last_time_sent_ms;
}


