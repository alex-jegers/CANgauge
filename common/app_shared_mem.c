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
struct can_rx_data
{
	char ids[9];			//Only uses 8 but needs to be size 9 for the null terminator.
	char data[17];			//Only uses size 16 but needs to be 17 for the null terminator.
	char period_ms[10];

	can_rx_buffer_entry_t buf;	//Numeric version of the data.
	uint32_t timestamp;			//Timestamp of last time recieved.
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
	struct can_rx_data can1_rx0_data[20];		//Data from FDCAN1 formatted as strings for CM7.
	uint32_t can1_rx0_unique_ids;				//How many messages have been rx'd in CAN1.
	struct can_rx_data can1_rx1_data[20];		//Data from FDCAN1 formatted as strings for CM7.
	uint32_t can1_rx1_unique_ids;				//How many messages have been rx'd in CAN1.
	
	struct can_rx_data can2_rx0_data[20];		//Copy of above for CAN2
	uint32_t can2_rx0_unique_ids;					//Copy of above for CAN2
	struct can_rx_data can2_rx1_data[20];		//Copy of above for CAN2
	uint32_t can2_rx1_unique_ids;					//Copy of above for CAN2

	struct can_tx_data can1_tx[CAN1_TX_BUFFER_ELEMENTS];	//Holds the raw data to be transmitted.
	uint8_t can1_tx_unique_ids;								//How many individual messages are being used.

}shared_mem_t;

/**********		STATIC VARIABLE DEFINITIONS		**********/
//static volatile shared_mem_t* const app_shared_mem = (shared_mem_t*)0xD02A3000;
__attribute__((__section__(".shared_data"))) static volatile shared_mem_t p;
/**********		STATIC FUNCTION DECLRATIONS		**********/

/**********		STATIC FUNCTION DEFINITIONS		**********/

/**********		GLOBAL FUNCTION DEFINITIONS		**********/
void system_init_shared_mem()
{
	memset(&p, 0, sizeof(shared_mem_t));

	p.hs_can_baud = CAN_BAUD_ERROR;
	p.ls_can_baud = CAN_BAUD_ERROR;
	p.can_error = false;
	p.hs_can_baud_override = CAN_BAUD_ERROR;
	p.ls_can_baud_override = CAN_BAUD_ERROR;
}

void shared_mem_set_cst830_touch_data(bool pressed, uint16_t x_pos, uint16_t y_pos)
{
	p.cst830_is_touched = pressed;
	p.cst830_pos_x = x_pos;
	p.cst830_pos_y = y_pos;
}

void shared_set_can_baud(FDCAN_GlobalTypeDef* canbus, can_baud_rate_t baud_rate)
{
	if (canbus == HS_CAN)
	{
		p.hs_can_baud = baud_rate;
	}

	if (canbus == LS_CAN)
	{
		p.ls_can_baud = baud_rate;
	}
}

void shared_set_can_baud_override(FDCAN_GlobalTypeDef* canbus, can_baud_rate_t baud_rate)
{
	if (canbus == HS_CAN)
	{
		p.hs_can_baud_override = baud_rate;
	}

	if (canbus == LS_CAN)
	{
		p.ls_can_baud_override = baud_rate;
	}
}

void shared_set_can_rx0_str_id(FDCAN_GlobalTypeDef* canbus, uint32_t id_index, char* id)
{
	if (canbus == HS_CAN)
	{
		memcpy(&p.can1_rx0_data[id_index].ids, id, sizeof(p.can1_rx0_data[id_index].ids));
	}
	if (canbus == LS_CAN)
	{
		memcpy(&p.can2_rx0_data[id_index].ids, id, sizeof(p.can2_rx0_data[id_index].ids));
	}
}

void shared_set_can_rx1_str_id(FDCAN_GlobalTypeDef* canbus, uint32_t id_index, char* id)
{
	if (canbus == HS_CAN)
	{
		memcpy(&p.can1_rx1_data[id_index].ids, id, sizeof(p.can1_rx1_data[id_index].ids));
	}
	if (canbus == LS_CAN)
	{
		memcpy(&p.can2_rx1_data[id_index].ids, id, sizeof(p.can2_rx1_data[id_index].ids));
	}
}

void shared_set_can_rx0_str_data(FDCAN_GlobalTypeDef* canbus, uint32_t id_index, char* data)
{
	if (canbus == HS_CAN)
	{
		memcpy(&p.can1_rx0_data[id_index].data, data, sizeof(p.can1_rx0_data[id_index].data));
	}
	if (canbus == LS_CAN)
	{
		memcpy(&p.can2_rx0_data[id_index].data, data, sizeof(p.can2_rx0_data[id_index].data));
	}
}

void shared_set_can_rx1_str_data(FDCAN_GlobalTypeDef* canbus, uint32_t id_index, char* data)
{
	if (canbus == HS_CAN)
	{
		memcpy(&p.can1_rx1_data[id_index].data, data, sizeof(p.can1_rx1_data[id_index].data));
	}
	if (canbus == LS_CAN)
	{
		memcpy(&p.can2_rx1_data[id_index].data, data, sizeof(p.can2_rx1_data[id_index].data));
	}
}

void shared_set_can_rx0_str_period(FDCAN_GlobalTypeDef* canbus, uint32_t id_index, char* period)
{
	if (canbus == HS_CAN)
	{
		memcpy(&p.can1_rx0_data[id_index].period_ms, period, sizeof(p.can1_rx0_data[id_index].period_ms));
	}
	if (canbus == LS_CAN)
	{
		memcpy(&p.can2_rx0_data[id_index].period_ms, period, sizeof(p.can2_rx0_data[id_index].period_ms));
	}
}

void shared_set_can_rx1_str_period(FDCAN_GlobalTypeDef* canbus, uint32_t id_index, char* period)
{
	if (canbus == HS_CAN)
	{
		memcpy(&p.can1_rx1_data[id_index].period_ms, period, sizeof(p.can1_rx1_data[id_index].period_ms));
	}
	if (canbus == LS_CAN)
	{
		memcpy(&p.can2_rx1_data[id_index].period_ms, period, sizeof(p.can2_rx1_data[id_index].period_ms));
	}
}

void shared_set_can_rx0_unique_ids(FDCAN_GlobalTypeDef* canbus, uint8_t num_ids)
{
	if (canbus == HS_CAN)
	{
		p.can1_rx0_unique_ids = num_ids;
	}
	if (canbus == LS_CAN)
	{
		p.can2_rx0_unique_ids = num_ids;
	}
}

void shared_set_can_rx1_unique_ids(FDCAN_GlobalTypeDef* canbus, uint8_t num_ids)
{
	if (canbus == HS_CAN)
	{
		p.can1_rx1_unique_ids = num_ids;
	}
	if (canbus == LS_CAN)
	{
		p.can2_rx1_unique_ids = num_ids;
	}
}

void shared_set_can_rx0_buffer_entry(FDCAN_GlobalTypeDef* canbus, uint32_t id_index, can_rx_buffer_entry_t* buf)
{
	p.can1_rx0_data[id_index].buf = *buf;
}

void shared_set_can_rx1_buffer_entry(FDCAN_GlobalTypeDef* canbus, uint32_t id_index, can_rx_buffer_entry_t* buf)
{
	p.can1_rx1_data[id_index].buf = *buf;
}

void shared_set_can_rx0_time_stamp(FDCAN_GlobalTypeDef* canbus, uint32_t id_index, uint32_t time)
{
	p.can1_rx0_data->timestamp = time;
}

void shared_set_can_rx1_time_stamp(FDCAN_GlobalTypeDef* canbus, uint32_t id_index, uint32_t time)
{
	p.can1_rx1_data->timestamp = time;
}

void shared_set_target_can(FDCAN_GlobalTypeDef* canbus)
{
	p.target_canbus = canbus;
}

void shared_set_can_error(FDCAN_GlobalTypeDef* canbus, bool status)
{
	p.can_error = status;
}

void shared_set_can_tx_unique_ids(FDCAN_GlobalTypeDef* canbus, uint32_t ids)
{
	p.can1_tx_unique_ids = ids;
}

void shared_set_tx_time_last_sent(uint8_t index, uint32_t time_ms)
{
	p.can1_tx[index].last_time_sent_ms = time_ms;
}

/************************************************************************************/
can_baud_rate_t shared_get_can_baud_rate(FDCAN_GlobalTypeDef* canbus)
{
	if (canbus == HS_CAN)
	{
		return p.hs_can_baud;
	}

	if (canbus == LS_CAN)
	{
		return p.ls_can_baud;
	}

	return CAN_BAUD_ERROR;
}

can_baud_rate_t shared_get_can_baud_rate_override(FDCAN_GlobalTypeDef* canbus)
{
	if (canbus == HS_CAN)
	{
		return p.hs_can_baud_override;
	}

	if (canbus == LS_CAN)
	{
		return p.ls_can_baud_override;
	}

	return CAN_BAUD_ERROR;
}

bool shared_mem_get_cst830_is_pressed()
{
	return p.cst830_is_touched;
}

uint16_t shared_mem_get_cst830_pos_x()
{
	return p.cst830_pos_x;
}

uint16_t shared_mem_get_cst830_pos_y()
{
	return p.cst830_pos_y;
}

char* shared_get_can_rx0_str_id(FDCAN_GlobalTypeDef* canbus, uint8_t id_index)
{
	if (canbus == HS_CAN)
	{
		return p.can1_rx0_data[id_index].ids;
	}
	if (canbus == LS_CAN)
	{
		return p.can2_rx0_data[id_index].ids;
	}
	return NULL;
}

char* shared_get_can_rx1_str_id(FDCAN_GlobalTypeDef* canbus, uint8_t id_index)
{
	if (canbus == HS_CAN)
	{
		return p.can1_rx1_data[id_index].ids;
	}
	if (canbus == LS_CAN)
	{
		return p.can2_rx1_data[id_index].ids;
	}
	return NULL;
}

char* shared_get_can_rx0_str_data(FDCAN_GlobalTypeDef* canbus, uint8_t id_index)
{
	if (canbus == HS_CAN)
	{
		return p.can1_rx0_data[id_index].data;
	}
	if (canbus == LS_CAN)
	{
		return p.can2_rx0_data[id_index].data;
	}
	return NULL;
}

char* shared_get_can_rx1_str_data(FDCAN_GlobalTypeDef* canbus, uint8_t id_index)
{
	if (canbus == HS_CAN)
	{
		return p.can1_rx1_data[id_index].data;
	}
	if (canbus == LS_CAN)
	{
		return p.can2_rx1_data[id_index].data;
	}
	return NULL;
}

char* shared_get_can_rx0_str_period(FDCAN_GlobalTypeDef* canbus, uint8_t id_index)
{
	if (canbus == HS_CAN)
	{
		return p.can1_rx0_data[id_index].period_ms;
	}
	if (canbus == LS_CAN)
	{
		return p.can2_rx0_data[id_index].period_ms;
	}
	return NULL;
}

char* shared_get_can_rx1_str_period(FDCAN_GlobalTypeDef* canbus, uint8_t id_index)
{
	if (canbus == HS_CAN)
	{
		return p.can1_rx1_data[id_index].period_ms;
	}
	if (canbus == LS_CAN)
	{
		return p.can2_rx1_data[id_index].period_ms;
	}
	return NULL;
}

void shared_get_can_rx0_buffer_entry(FDCAN_GlobalTypeDef* canbus, uint32_t id_index, can_rx_buffer_entry_t* buf)
{
	if (canbus == FDCAN1)
	{
		*buf = p.can1_rx0_data[id_index].buf;
	}
	else if (canbus == FDCAN2)
	{
		*buf = p.can2_rx0_data[id_index].buf;
	}
	return 0;
}

void shared_get_can_rx1_buffer_entry(FDCAN_GlobalTypeDef* canbus, uint32_t id_index, can_rx_buffer_entry_t* buf)
{
	if (canbus == FDCAN1)
	{
		*buf = p.can1_rx1_data[id_index].buf;
	}
	else if (canbus == FDCAN2)
	{
		*buf = p.can2_rx1_data[id_index].buf;
	}
	return 0;
}

uint32_t shared_get_can_rx0_timestamp(FDCAN_GlobalTypeDef* canbus, uint32_t id_index)
{
	if (canbus == FDCAN1)
	{
		return p.can1_rx0_data[id_index].timestamp;
	}
	else if (canbus == FDCAN2)
	{
		return p.can1_rx0_data[id_index].timestamp;
	}
	return 0;
}

uint32_t shared_get_can_rx1_timestamp(FDCAN_GlobalTypeDef* canbus, uint32_t id_index)
{
	if (canbus == FDCAN1)
	{
		return p.can1_rx1_data[id_index].timestamp;
	}
	else if (canbus == FDCAN2)
	{
		return p.can1_rx1_data[id_index].timestamp;
	}
	return 0;
}


uint8_t shared_get_can_rx0_unique_ids(FDCAN_GlobalTypeDef* canbus)
{
	if (canbus == HS_CAN)
	{
		return p.can1_rx0_unique_ids;
	}
	if (canbus == LS_CAN)
	{
		return p.can2_rx0_unique_ids;
	}
	return 0;
}

uint8_t shared_get_can_rx1_unique_ids(FDCAN_GlobalTypeDef* canbus)
{
	if (canbus == HS_CAN)
	{
		return p.can1_rx1_unique_ids;
	}
	if (canbus == LS_CAN)
	{
		return p.can2_rx1_unique_ids;
	}
	return 0;
}

FDCAN_GlobalTypeDef* shared_get_target_can()
{
	return p.target_canbus;
}

bool shared_get_can_error()
{
	return p.can_error;
}

uint32_t shared_get_tx_unique_ids(FDCAN_GlobalTypeDef* canbus)
{
	return p.can1_tx_unique_ids;
}

void shared_set_can_tx_timing_data(uint32_t interval, uint8_t index)
{
	p.can1_tx[index].interval_ms = interval;
}

uint32_t shared_get_tx_interval(uint32_t index)
{
	return p.can1_tx[index].interval_ms;
}

uint32_t shared_get_tx_last_time_sent(uint32_t index)
{
	return p.can1_tx[index].last_time_sent_ms;
}


