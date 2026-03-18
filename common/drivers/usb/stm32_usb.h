
#ifndef _TEMPLATE_H_
#define _TEMPLATE_H_

#ifdef __cplusplus
extern "C" {
#endif

/**********     INCLUDES        **********/
#include "stm32h745xx.h"
#include "drivers/drivers.h"

/**********     TYPEDEFS         **********/

/**********     DEFINES      **********/

/**********     GLOBAL VARIABLE DECLRATIONS     **********/

/**********		GLOBAL FUNCTION DECLRATIONS		**********/
/**
 * usb_init:
 *      desc: turns on the clocks and IO for USB FS. Does not do anything
 *          with the USB core. usb_core_reset has to be called next.
 */
void usb_init();

/**
 * usb_core_reset:
 *      desc: resets the USB core. Must wait 3 USB PHY clock cycles to call any USB functions
 *          after this funciton returns.
 */
void usb_core_reset();

/**
 * usb_init_core:
 *      desc: initializes the USB core and device descriptors. Call after usb_core_reset.
 */
void usb_init_core();


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif  //_TEMPLATE_H_