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




/**********     GLOBAL FUNCTION PROTOTYPES     **********/
void system_init_shared_mem();

/*Setters*/
void shared_mem_set_cst830_touch_data(bool pressed, uint16_t x_pos, uint16_t y_pos);
void shared_set_can_baud(FDCAN_GlobalTypeDef* canbus, can_baud_rate_t baud_rate);
void shared_set_can_baud_override(FDCAN_GlobalTypeDef* canbus, can_baud_rate_t baud_rate);
void shared_set_can_str_id(FDCAN_GlobalTypeDef* canbus, uint32_t id_index, char* id);
void shared_set_can_str_data(FDCAN_GlobalTypeDef* canbus, uint32_t id_index, char* data);
void shared_set_can_str_period(FDCAN_GlobalTypeDef* canbus, uint32_t id_index, char* period);
void shared_set_can_unique_ids(FDCAN_GlobalTypeDef* canbus, uint8_t num_ids);
void shared_set_target_can(FDCAN_GlobalTypeDef* canbus);
void shared_set_can_error(FDCAN_GlobalTypeDef* canbus, bool status);
void shared_set_tx_unique_ids(FDCAN_GlobalTypeDef* canbus, uint32_t ids);
void shared_set_can_tx_data(uint32_t interval, uint8_t index);

/*
 * Sets the time last sent of a TX message in ms.
 */
void shared_set_tx_time_last_sent(uint8_t index, uint32_t time_ms);

/*Getters*/
can_baud_rate_t shared_get_can_baud_rate(FDCAN_GlobalTypeDef* canbus);
can_baud_rate_t shared_get_can_baud_rate_override(FDCAN_GlobalTypeDef* canbus);
bool shared_mem_get_cst830_is_pressed();
uint16_t shared_mem_get_cst830_pos_x();
uint16_t shared_mem_get_cst830_pos_y();
char* shared_get_can_str_id(FDCAN_GlobalTypeDef* canbus, uint8_t id_index);
char* shared_get_can_str_data(FDCAN_GlobalTypeDef* canbus, uint8_t id_index);
char* shared_get_can_str_period(FDCAN_GlobalTypeDef* canbus, uint8_t id_index);
uint8_t shared_get_can_unique_ids(FDCAN_GlobalTypeDef* canbus);
FDCAN_GlobalTypeDef* shared_get_target_can();
bool shared_get_can_error();
uint32_t shared_get_tx_unique_ids(FDCAN_GlobalTypeDef* canbus);

/*
 * Returns the requested transmission interval of CAN TX buffer number index.
 */
uint32_t shared_get_tx_interval(uint32_t index);

/*
 * Returns the time last sent of CAN TX buffer number index.
 */
uint32_t shared_get_tx_last_time_sent(uint32_t index);

#endif /* CORE_SYSTEM_SYSTEM_SHARED_MEM_H_ */
