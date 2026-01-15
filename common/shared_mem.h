/*
 * system_shared_mem.h
 *
 *  Created on: Jun 1, 2024
 *      Author: awjpp
 */

#ifndef CORE_SYSTEM_SYSTEM_SHARED_MEM_H_
#define CORE_SYSTEM_SYSTEM_SHARED_MEM_H_

#include "stm32h745xx.h"
#include "drivers/stm32_canbus.h"
#include "stdbool.h"

#include "FreeRTOS.h"
#include "queue.h"

#define HS_CAN				FDCAN1
#define LS_CAN				FDCAN2

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

	void* p_touch_data;

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

__attribute__((__section__(".shared_data"))) extern volatile shared_mem_t p;

/**********     GLOBAL FUNCTION PROTOTYPES     **********/
void system_init_shared_mem();

/* Touch screen setters. */
void shared_mem_set_cst830_touch_data(bool pressed, uint16_t x_pos, uint16_t y_pos);

/* Misc CAN setters. */
void shared_set_can_baud(FDCAN_GlobalTypeDef* canbus, can_baud_rate_t baud_rate);
void shared_set_can_baud_override(FDCAN_GlobalTypeDef* canbus, can_baud_rate_t baud_rate);
void shared_set_target_can(FDCAN_GlobalTypeDef* canbus);
void shared_set_can_error(FDCAN_GlobalTypeDef* canbus, bool status);

/* CAN1 & 2 FIFO0 setters. */
void shared_set_can_rx0_str_id(FDCAN_GlobalTypeDef* canbus, uint32_t id_index, char* id);
void shared_set_can_rx0_str_data(FDCAN_GlobalTypeDef* canbus, uint32_t id_index, char* data);
void shared_set_can_rx0_str_period(FDCAN_GlobalTypeDef* canbus, uint32_t id_index, char* period);
void shared_set_can_rx0_unique_ids(FDCAN_GlobalTypeDef* canbus, uint8_t num_ids);
void shared_set_can_rx0_buffer_entry(FDCAN_GlobalTypeDef* canbus, uint32_t id_index, can_rx_buffer_entry_t* buf);
void shared_set_can_rx0_time_stamp(FDCAN_GlobalTypeDef* canbus, uint32_t id_index, uint32_t time);

/* CAN1 & 2 FIFO1 setters. */
void shared_set_can_rx1_str_id(FDCAN_GlobalTypeDef* canbus, uint32_t id_index, char* id);
void shared_set_can_rx1_str_data(FDCAN_GlobalTypeDef* canbus, uint32_t id_index, char* data);
void shared_set_can_rx1_str_period(FDCAN_GlobalTypeDef* canbus, uint32_t id_index, char* period);
void shared_set_can_rx1_unique_ids(FDCAN_GlobalTypeDef* canbus, uint8_t num_ids);
void shared_set_can_rx1_buffer_entry(FDCAN_GlobalTypeDef* canbus, uint32_t id_index, can_rx_buffer_entry_t* buf);
void shared_set_can_rx1_time_stamp(FDCAN_GlobalTypeDef* canbus, uint32_t id_index, uint32_t time);

/* CAN1 & 2 TX setters. */
void shared_set_can_tx_unique_ids(FDCAN_GlobalTypeDef* canbus, uint32_t ids);
void shared_set_can_tx_timing_data(uint32_t interval, uint8_t index);
void shared_set_tx_time_last_sent(uint8_t index, uint32_t time_ms);


/* Touch screen info getters. */
bool shared_mem_get_cst830_is_pressed();
uint16_t shared_mem_get_cst830_pos_x();
uint16_t shared_mem_get_cst830_pos_y();

/* Misc CAN getters. */
can_baud_rate_t shared_get_can_baud_rate(FDCAN_GlobalTypeDef* canbus);
can_baud_rate_t shared_get_can_baud_rate_override(FDCAN_GlobalTypeDef* canbus);
FDCAN_GlobalTypeDef* shared_get_target_can();
bool shared_get_can_error();

/* CAN1 & 2 FIFO0 getters. */
char* shared_get_can_rx0_str_id(FDCAN_GlobalTypeDef* canbus, uint8_t id_index);
char* shared_get_can_rx0_str_data(FDCAN_GlobalTypeDef* canbus, uint8_t id_index);
char* shared_get_can_rx0_str_period(FDCAN_GlobalTypeDef* canbus, uint8_t id_index);
void shared_get_can_rx0_buffer_entry(FDCAN_GlobalTypeDef* canbus, uint32_t id_index, can_rx_buffer_entry_t* buf);
uint32_t shared_get_can_rx0_timestamp(FDCAN_GlobalTypeDef* canbus, uint32_t id_index);
uint8_t shared_get_can_rx0_unique_ids(FDCAN_GlobalTypeDef* canbus);

/* CAN1 & 2 FIFO1 getters. */
char* shared_get_can_rx1_str_id(FDCAN_GlobalTypeDef* canbus, uint8_t id_index);
char* shared_get_can_rx1_str_data(FDCAN_GlobalTypeDef* canbus, uint8_t id_index);
char* shared_get_can_rx1_str_period(FDCAN_GlobalTypeDef* canbus, uint8_t id_index);
void shared_get_can_rx1_buffer_entry(FDCAN_GlobalTypeDef* canbus, uint32_t id_index, can_rx_buffer_entry_t* buf);
uint32_t shared_get_can_rx1_timestamp(FDCAN_GlobalTypeDef* canbus, uint32_t id_index);
uint8_t shared_get_can_rx1_unique_ids(FDCAN_GlobalTypeDef* canbus);


/* CAN1 & 2 TX getters. */
uint32_t shared_get_tx_unique_ids(FDCAN_GlobalTypeDef* canbus);
uint32_t shared_get_tx_interval(uint32_t index);
uint32_t shared_get_tx_last_time_sent(uint32_t index);

#endif /* CORE_SYSTEM_SYSTEM_SHARED_MEM_H_ */
