
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
 * name: btldr_load
 *      desc: Jumps to the bootloader at memory address 0x081A0004.
 *      params: none
 *      returns: none
 */
void btldr_load();



#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif  //_BOOTLOADER_H_
