
#ifndef _USB_TASK_H_
#define _USB_TASK_H_

#ifdef __cplusplus
extern "C" {
#endif

/**********     INCLUDES        **********/
#include "system/system_cm7.h"

/**********     TYPEDEFS         **********/
typedef enum
{
	USB_FS_RAM,		//used for the bootloader.
	USB_FS_EEPROM	//used for saving settings and data logging.
}usb_fs_t;

/**********     DEFINES      **********/

/**********     GLOBAL VARIABLE DECLRATIONS     **********/

/**********		GLOBAL FUNCTION DECLRATIONS		**********/
/**
 * usb_disconnect:
 * 		desc: disables the USB peripheral and stops both tasks.
 */
void usb_disconnect();

/**
 * usb_connect:
 * 		desc: starts both USB tasks, links the file_sys indicated to the MSC device.
 */
void usb_connect(usb_fs_t file_sys);

/**
 * usb_watchdog_task
 *      desc: FreeRTOS task that monitors the activity on the USB bus and reset the peripheral if
 *      	it detects the USB plug was disconnected.
 *      params: none
 *      returns: none
 */
void usb_watchdog_task();
void usb_watchdog_run();

/**
 * usb_msc_task
 * 		desc: the task responsible for handing data from the USB mass storage class driver.
 */
void usb_msc_task();
void usb_msc_task_run();

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif  //_USB_TASK_H_
