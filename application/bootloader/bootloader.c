/**********     INCLUDES        **********/
#include "bootloader.h"
#include "ui/ui_settings.h"

/**********     TYPEDEFS         **********/

/**********		DEFINES		**********/

/**********		EXTERNAL VARIABLE DEFINITIONS		**********/

/**********		STATIC VARIABLES		**********/
/**
 * //The address of the reset handler of the bootloader is at this address.
 * As determined by the linker file in the bootloader (separate program, not to
 * be confused with this bootloader.c file).
 */
void (**prv_btldr_jmp)() = (void (**)())0x081A0004;

/**********		STATIC FUNCTION DECLRATIONS		**********/
static void btldr_firmware_btn_cb(lv_event_t* e);

/**********		STATIC FUNCTION DEFINITIONS		**********/
static void btldr_firmware_btn_cb(lv_event_t* e)
{
	/*
	 *  Stop the power monitor.
	 * TODO: Test without some of these lines.
	 * */
	pwr_monitor_suspend();
	portENTER_CRITICAL();
	__disable_irq();		//Disable all interrupts.
	SysTick->CTRL = 0;		//Disable SysTick.

	/* Clear all enabled and pending interrupts. */
	for (uint32_t i = 0; i < 8; i++)
	{
		NVIC->ICER[i] = 0xFFFFFFFF;
		NVIC->ICPR[i] = 0xFFFFFFFF;
	}
	__enable_irq();			//Enable interrupts. FreeRTOS does this on startup in the bootloader but it's probs good practice to leave it in the default state.

	(*prv_btldr_jmp)();		//Jump to the bootloader.
}

/**********		GLOBAL FUNCTION DEFINITIONS		**********/
void btldr_init()
{
	ui_add_settings_firmware_update_btn_event_cb(btldr_firmware_btn_cb);
}


