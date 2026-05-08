
#ifndef _TEMPLATE_H_
#define _TEMPLATE_H_

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
 * usb_watchdog_task
 *      desc: FreeRTOS task that monitors the activity on the USB bus and reset the peripheral if
 *      	it detects the USB plug was disconnected.
 *      params: none
 *      returns: none
 */
void usb_watchdog_task();


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif  //_TEMPLATE_H_
