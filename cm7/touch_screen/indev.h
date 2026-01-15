/**
 * indev.h
 * description: handles the LVGL side of the input device (touch screen).
 *          Doesn't do any I2C or IO communication, reads indev info from 
 *          shared memory.
 */


#ifndef _INDEV_H_
#define _INDEV_H_

#ifdef __cplusplus
extern "C" {
#endif

/**********     INCLUDES        **********/
#include "stm32h745xx.h"
#include "stdbool.h"

/**********     DEFINES      **********/

/**********     GLOBAL VARIABLE DECLRATIONS     **********/
typedef struct
{
	uint8_t touch_num;
	uint16_t touch1_x;
	uint16_t touch1_y;
}touch_info_t;

/**********		GLOBAL FUNCTION DECLRATIONS		**********/
void indev_init(touch_info_t** p_touch_data);


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif  //_CST830_TOUCH_H_


