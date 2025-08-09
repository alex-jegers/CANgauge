/*
 * stm32_shared.h
 *
 *  Created on: Mar 16, 2024
 *      Author: awjpp
 */

#ifndef SRC_DRIVERS_STM32_SHARED_H_
#define SRC_DRIVERS_STM32_SHARED_H_

#include "stdint.h"
#include "string.h"

typedef struct
{
	uint16_t param_id;
	float data;
}shared_can_data_cm4_t;

typedef struct
{
	uint16_t last_clicked_param_id;
}shared_can_data_cm7_t;

typedef struct
{
	shared_can_data_cm4_t can;
	shared_can_data_cm7_t ui;
}shared_data_t;


#define SHARED_MEM			((shared_data_t*)0x30040000)

void shared_mem_init();




#endif /* SRC_DRIVERS_STM32_SHARED_H_ */
