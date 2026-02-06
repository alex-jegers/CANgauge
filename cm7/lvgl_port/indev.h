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
#include "lvgl_port/lvgl_port_def.h"

/**********     DEFINES      **********/

/**********     GLOBAL VARIABLE DECLRATIONS     **********/

/**********		GLOBAL FUNCTION DECLRATIONS		**********/
/*
 * indev_init:
 * params:
 * p_touch_data: a pointer to a pointer where the touch data is located.
 *		The double pointer is used in case the pointer hasn't been assigned
 * 		at the time this function is called.
 *
 * desc: binds the input device callback to LVGL.
 *
 * */
void indev_init(touch_info_t** p_touch_data);


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif  //_CST830_TOUCH_H_


