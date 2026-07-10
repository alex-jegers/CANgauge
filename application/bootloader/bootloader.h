
#ifndef _BOOTLOADER_H_
#define _BOOTLOADER_H_

#ifdef __cplusplus
extern "C" {
#endif

/**********     INCLUDES        **********/
#include "system/system_cm7.h"

/**********     TYPEDEFS         **********/

/**********     DEFINES      **********/

/**********     GLOBAL VARIABLE DECLRATIONS     **********/

/**********		GLOBAL FUNCTION DECLRATIONS		**********/
/**
 * name:
 *      desc:
 *      params:
 *      returns:
 */
TaskFunction_t btldr_task();
void btldr_init();



#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif  //_BOOTLOADER_H_
