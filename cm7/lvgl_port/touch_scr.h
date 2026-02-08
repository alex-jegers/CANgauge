


#ifndef _CST830_TOUCH_H_
#define _CST830_TOUCH_H_

#ifdef __cplusplus
extern "C" {
#endif

/**********     INCLUDES        **********/
#include "stm32h745xx.h"
#include "lvgl_port_def.h"

/**********     DEFINES      **********/

/**********     GLOBAL VARIABLE DECLRATIONS     **********/

/**********		GLOBAL VARIABLE DEFINITIONS		**********/

/**********		GLOBAL FUNCTION DECLRATIONS		**********/
/*
 * touch_scr_run:
 *
 * desc: takes a pointer to a touch_info_t struct where it writes touch data to
 * every so often as specified by CST830_REFRESH_RATE.
 * */
void touch_scr_run(touch_info_t* p_touch_data);

void touch_scr_stop();





#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif  //_CST830_TOUCH_H_


