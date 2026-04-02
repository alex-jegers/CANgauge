
#ifndef _STM32_USB_MSC_H_
#define _STM32_USB_MSC_H_

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
void usb_msc_process_setup_packet(usb_setup_packet_t* setup_packet);
void usb_msc_handle_data(uint32_t length);
void usb_msc_ep_in_handler(uint32_t ep, uint32_t ir);




#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif  //_STM32_USB_MSC_H_
