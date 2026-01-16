


#ifndef _CST830_TOUCH_H_
#define _CST830_TOUCH_H_

#ifdef __cplusplus
extern "C" {
#endif

/**********     INCLUDES        **********/
#include "stm32h745xx.h"

/**********     DEFINES      **********/

/**********     GLOBAL VARIABLE DECLRATIONS     **********/
typedef struct
{
	uint8_t touch_num;
	uint16_t touch1_x;
	uint16_t touch1_y;
}touch_info_t;

/**********		GLOBAL VARIABLE DEFINITIONS		**********/

/**********		GLOBAL FUNCTION DECLRATIONS		**********/
/*
 * cst830_task_update:
 *
 * desc: takes a pointer to a touch_info_t struct where it writes touch data to
 * every so often as specified by CST830_REFRESH_RATE.
 * */
void cst830_task_update(touch_info_t* p_touch_data);




#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif  //_CST830_TOUCH_H_


