/*
 * stm32_shared.c
 *
 *  Created on: Mar 16, 2024
 *      Author: awjpp
 */

#include "stm32_shared.h"


void shared_mem_init()
{
	for (uint8_t i = 0; i < sizeof(shared_data_t) / 4; i++)
	{
		*((uint32_t*)0x30040000 + i) = 0;
	}
}
