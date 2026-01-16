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
#include "indev_def.h"

/**********     DEFINES      **********/

/**********     GLOBAL VARIABLE DECLRATIONS     **********/

/**********		GLOBAL FUNCTION DECLRATIONS		**********/
void indev_init(touch_info_t** p_touch_data);


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif  //_CST830_TOUCH_H_


