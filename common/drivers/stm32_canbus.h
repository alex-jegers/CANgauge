/*
 * stm32_canbus.h
 *
 *  Created on: Mar 9, 2024
 *      Author: awjpp
 */

#ifndef SRC_DRIVERS_STM32_CANBUS_H_
#define SRC_DRIVERS_STM32_CANBUS_H_

/***********	INCLUDES	************/
#include "stm32h745xx.h"
#include "stm32_canbus_def.h"
#include "stdbool.h"

/***********	TYPEDEFS	************/

/***********	DEFINES		************/

/***********	VARIABLE DEFINITIONS	************/

/***********	GLOBAL FUNCTION DECLARATIONS	************/
/**
 * can_init:
 * desc: sets up the IO pins. Sets up the filter to store non-matching standard and extended
 * 		frames in FIFO0. Configures the address pointers to the standard and extended filter
 * 		lists based on CAN1_xxx_ID_FILTER_ADDR macro. Configures the buffer address pointers
 * 		to the RX and TX FIFOs. Clears interrupts.
 */
void can_init(FDCAN_GlobalTypeDef* canbus);			

/**
 * can_deinit:
 * desc: disables interrupts. Turns off clocks, resets both peripheral blocks 
 * 		(FDCAN1 and 2). 
 */
void can_deinit();
/**
 * can_stop:
 * desc: sets the INIT and CCE bits so the CAN registers can be modified.
 */
void can_stop(FDCAN_GlobalTypeDef* canbus);
/**
 * can_run: 
 * desc: clears the INIT and CCE bits locking CAN registers.
 */
void can_run(FDCAN_GlobalTypeDef* canbus);
/**
 * can_set_baud_rate:
 * desc: sets the values of the NBTP registers, assumes a 48MHz kernel clock.
 */
void can_set_baud_rate(FDCAN_GlobalTypeDef* canbus, can_baud_rate_t baud_rate);
/**
 * can_get_baud_rate:
 * desc: samples the bus activity in listen only mode and will 
 *      return the baud rate assuming it's an option in can_baud_rate_t.
 */
can_baud_rate_t can_get_baud_rate(FDCAN_GlobalTypeDef* canbus);

/**
 * can_get_last_error_code:
 * desc: returns whatever the last error code stored in the PSR register was.
 *      Saves it's state even after LEC has switched to NO_CHANGE.
 */
can_error_code_t can_get_last_error_code(FDCAN_GlobalTypeDef* canbus);

uint32_t can_get_rx_error_counter(FDCAN_GlobalTypeDef* canbus);
uint32_t can_get_tx_error_counter(FDCAN_GlobalTypeDef* canbus);

/**
 * can_add_tx_buffer:
 * desc: Adds a TX buffer entry to the CAN message RAM TX buffers.
 * params:
 *      canbus: Either FDCAN1 or FDCAN2.
 *      new_message: A pointer to the tx_buffer_entry_t that will be copied to message RAM.
 *      index: What index in message RAM to place the buffer (will overwrite previous data written to said index).
**/
int8_t can_add_tx_buffer(FDCAN_GlobalTypeDef* canbus, can_tx_buffer_entry_t* new_message, uint8_t index);
/**
 * can_get_tx_buffer:
 * desc: returns a pointer to a TX buffer specified by index.
 */
can_tx_buffer_entry_t* can_get_tx_buffer(FDCAN_GlobalTypeDef* canbus, uint8_t index);
/**
 * can_tx: 
 * desc: sets the TXBAR bit requesting a message be sent.
 */
void can_tx(FDCAN_GlobalTypeDef* canbus, uint8_t index);

/**
 * can_check_for_rx_fifo0:
 * returns: true if there is a message(s) in FIFO0.
 */
bool can_check_for_rx_fifo0(FDCAN_GlobalTypeDef* canbus);
/**
 * can_read_from_fifo0:
 * desc: copys the next message in the FIFO into the message param.
 * returns: the number of items remaining in the FIFO.
 */
uint8_t can_read_from_fifo0(FDCAN_GlobalTypeDef* canbus, can_rx_buffer_entry_t* message);

/**
 * can_check_for_rx_fifo1:
 * returns: true if there is a message(s) in FIFO1.
 */
bool can_check_for_rx_fifo1(FDCAN_GlobalTypeDef* canbus);
/**
 * can_read_from_fifo1:
 * desc: copys the next message in the FIFO into the message param.
 * returns: the number of items remaining in the FIFO.
 */
uint8_t can_read_from_fifo1(FDCAN_GlobalTypeDef* canbus, can_rx_buffer_entry_t* message);

uint32_t can_get_can_id(can_rx_buffer_entry_t* message);

/*Filter control*/
void can_set_std_id_filter(FDCAN_GlobalTypeDef* canbus, uint8_t index, can_std_id_filter_t* filter);
void can_set_ext_id_filter(FDCAN_GlobalTypeDef* canbus, uint8_t index, can_ext_id_filter_t* filter);		//returns 1 if list is full, 2 if ID is not a valid ID, 0 if no error.


void can_assign_rx_rf0n_cb(FDCAN_GlobalTypeDef* canbus, void (*func)());		//Assign a callback for a new message in FIFO 0 interrupt.
void can_assign_rx_rf1n_cb(FDCAN_GlobalTypeDef* canbus, void (*func)());		//Assign a callback for a new message in FIFO 1 interrupt.
void can_assign_rx_rf0f_cb(FDCAN_GlobalTypeDef* canbus, void (*func)());		//RX FIFO0 full interrupt.

void can_enable_rx_rf0n_interrupt(FDCAN_GlobalTypeDef* canbus);     //Enable new message in FIFO 0 interrupt.
void can_enable_rx_rf1n_interrupt(FDCAN_GlobalTypeDef* canbus);     //Enable new message in FIFO 1 interrupt.





#endif /* SRC_DRIVERS_STM32_CANBUS_H_ */
