
#ifndef _INDEV_DEF_H_
#define _INDEV_DEF_H_

#ifdef __cplusplus
extern "C" {
#endif

/**********     INCLUDES        **********/
#include <stdint.h>

/**********     DEFINES      **********/

/**********     GLOBAL VARIABLE DECLRATIONS     **********/
typedef struct
{
	uint8_t touch_num;
	uint16_t touch1_x;
	uint16_t touch1_y;
}touch_info_t;

/**********		GLOBAL FUNCTION DECLRATIONS		**********/




#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif  //_INDEV_DEF_H_