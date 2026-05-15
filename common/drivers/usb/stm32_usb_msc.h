
#ifndef _STM32_USB_MSC_H_
#define _STM32_USB_MSC_H_

#ifdef __cplusplus
extern "C" {
#endif

/**********     INCLUDES        **********/
#include "stm32h745xx.h"
#include "drivers/drivers.h"

/**********     TYPEDEFS 	**********/

/**********     DEFINES		**********/

/**********		UNIMPLEMENTED FUNCTION DECLARATIONS		**********/
/**
 * usb_msc_get_block_size (needs to be implemented by the application).
 * 		returns: the block size of the mass storage device in bytes.
 */
uint32_t usb_msc_get_block_size();

/**
 * usb_msc_get_num_blocks (implemented by application)
 * 		returns: the number of logical blocks of the mass storage device.
 */
uint32_t usb_msc_get_num_blocks();

/**********     GLOBAL VARIABLE DECLRATIONS     **********/

/**********		GLOBAL FUNCTION DECLRATIONS		**********/
void usb_msc_process_setup_packet(usb_setup_packet_t* setup_packet);
void usb_msc_handle_data(uint32_t length);
void usb_msc_ep_in_handler(uint32_t ep, uint32_t ir);

/**
 * usb_msc_set_read_cb:
 * 		desc: the function passed to this, 'func', will get called from
 * 			INSIDE the USB interrupt handler when a read command is received
 * 			for the USB mass storage device.
 */
void usb_msc_set_read_cb(void (*func)(uint32_t lba, uint32_t num_blocks));

/**
 * usb_msc_set_write_cb:
 * 		desc: the function passed to this, 'func', will get called from
 * 			INSIDE the USB interrupt handler when a write command is received
 * 			for the USB mass storage device.
 */
void usb_msc_set_write_cb(void (*func)(uint32_t lba, uint32_t num_blocks));

/**
 * usb_msc_write_complete_cb
 * 		desc: set a function to be called when all the bytes have been received
 * 			from a MSC write. Application needs to free the original buf address
 * 			after reading out all the data.
 * 		params:
 * 			buf: the address where the bytes are stored.
 * 			bytes: the number of bytes received.
 */
void usb_msc_set_write_complete_cb(bool (*func)(uint32_t length_bytes));

/**
 * usb_msc_read_cmd:
 * 		desc: the application must call this soon after the usb_msc_set_read_cb
 * 			function is called so the USB driver can transfer the data to the
 * 			USB host.
 * 		params:
 * 			buf: a pointer to the start of the data to be transmitted to the USB host.
 * 			bytes: the number of bytes in buf.
 */
void usb_msc_read_cmd(uint8_t* buf, uint32_t num_blocks);



#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif  //_STM32_USB_MSC_H_
