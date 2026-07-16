
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
 * name: btldr_init
 *      desc: Assigns the function call back for the update firmware button.
 *      params: none
 *      returns: none
 */
void btldr_init();



#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif  //_BOOTLOADER_H_
