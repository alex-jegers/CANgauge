/*
 * cangauge_common.h
 *
 * desc:
 * defines a structure in external SDRAM that holds pointers to data
 * that needs to be shared between cores.
 *
 *  Created on: Jun 1, 2024
 *      Author: awjpp
 */

#ifndef _CANGAUGE_COMMON_H_
#define _CANGAUGE_COMMON_H_

/**********		INCLUDES		**********/
#include "stm32h745xx.h"

/**********		DEFINES		**********/
#define CG_MEMORY_REGION_EXT			__attribute__((__section__(".ext_mem_ram")))

typedef struct
{
	/* A pointer to the touch screen data. */
	void* p_touch_data;

	/* A pointer to the CAN transmit time period list. */
	uint16_t* p_can_transmit_period_list;

	/* A pointer to UDS current data. */
	uint8_t (*p_can_controller_memory)[176][10];

}common_mem_t;

CG_MEMORY_REGION_EXT extern volatile common_mem_t common;

/**********     GLOBAL FUNCTION PROTOTYPES     **********/


#endif /* _CANGAUGE_COMMON_H_ */
