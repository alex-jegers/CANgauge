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

#define HS_CAN				FDCAN1
#define LS_CAN				FDCAN2

typedef struct
{
	can_baud_rate_t hs_can_baud;				//speed of the HS CANbus.
	can_baud_rate_t ls_can_baud;				//speed of the LS CANbus.

	/*Data that CM7 writes to.*/
	can_tx_buffer_entry_t can_tx;				//the data that CM7 is requested CM4 to send on over CAN.
	uint32_t tx_interval_ms;					//the frequency that CM4 should be sending the aforementioned data.
	uint32_t ext_id_filter;						//the extended ID filter for CM4 to apply.
	uint32_t std_id_filter;						//the standard ID filter for CM4 to apply.
	void (*set_gauge_val)(uint32_t val);

	/*Data that CM4 writes to.*/
	uint32_t can_rx_can_id;						//the CAN ID that the CM4 received from CANbus.
	uint8_t can_rx_data[8];						//the data that CM4 received from CANbus.
	bool can_rx_is_new;							//reads true if the data in can_rx is new since the last time.
	int32_t converted_value;					//the raw can data converted to a number that can be displayed on the active gauge.
}shared_mem_t;

extern shared_mem_t* const app_shared_mem;

/**********     GLOBAL FUNCTION PROTOTYPES     **********/
void system_init_shared_mem();
void shared_mem_call_set_gauge_val(uint32_t val);

/*Setters*/
void shared_mem_set_can_tx(const can_tx_buffer_entry_t* tx);
void shared_mem_set_can_tx_interval(uint32_t tx_interval_ms);
void shared_mem_set_can_tx_data(uint8_t data[], uint8_t data_length);
void shared_mem_set_can_ext_id_filter(uint32_t id);
void shared_mem_set_can_rx_id(uint32_t id);
void shared_mem_set_can_rx_data(uint8_t data[8]);
void shared_mem_set_gauge_val_ptr(void (*func)(uint32_t val));
void shared_mem_set_conversion_value(float val);
void shared_mem_set_converted_value(int32_t converted_val);

/*Getters*/
can_baud_rate_t shared_mem_get_can_baud_rate(FDCAN_GlobalTypeDef* canbus);
can_tx_buffer_entry_t* shared_mem_get_can_tx();
uint32_t shared_mem_get_can_tx_interval();
void shared_mem_get_can_tx_data(uint8_t data_destination[]);
uint32_t shared_mem_get_ext_id_filter();
float shared_mem_get_convserion_value();
int32_t shared_mem_get_converted_value();


#endif /* CORE_SYSTEM_SYSTEM_SHARED_MEM_H_ */
