
#ifndef _INDEV_DEF_H_
#define _INDEV_DEF_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**********     DEFINES      **********/

/* Can enable/disable these if using another core for certain tasks. */
#define LV_PORT_USE_DISP		1
#define LV_PORT_USE_INDEV		1
#define LV_PORT_USE_TOUCH_SCR	1

/**********		TYPEDEFS		**********/
typedef struct
{
	uint8_t touch_num;
	uint16_t touch1_x;
	uint16_t touch1_y;
}touch_info_t;

/**********		INCLUDES		**********/
#if LV_PORT_USE_DISP
	#include "disp.h"
#endif

#if LV_PORT_USE_INDEV
	#include "indev.h"
#endif

#if LV_PORT_USE_TOUCH_SCR
	#include "touch_scr.h"
#endif

/**********     GLOBAL VARIABLE DECLRATIONS     **********/


/**********		GLOBAL FUNCTION DECLRATIONS		**********/




#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif  //_INDEV_DEF_H_
