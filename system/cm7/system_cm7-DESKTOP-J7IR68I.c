/*
 * system.c
 *
 *  Created on: May 31, 2024
 *      Author: awjpp
 */

#ifdef CORE_CM7

/**********		INCLUDES		**********/
#include "system_cm7.h"
#include "drivers/stm32_io.h"
#include "drivers/stm32_lcd.h"
#include "drivers/stm32_fmc.h"
#include "drivers/stm32_rcc.h"
#include "drivers/stm32_hsem.h"
#include "touch_screen/iic_touch.h"
#include "lvgl/lvgl.h"
#include "application/app_shared_mem.h"
#include "application/cm7/app_gauges_cm7.h"
#include "application/cm7/app_can_baud_rate_cm7.h"

/**********		DEFINES		**********/
#define SCB_CPACR_CP10_FULL_ACCESS			0x3 << 20
#define SCB_CPACR_CP11_FULL_ACCESS			0x3 << 22

/**********     STATIC VARIABLES     **********/
static void (*system_app_ptr)();

/**********     STATIC FUNCTION DECLARATIONS     **********/
static void system_init_fpu();

/**********     STATIC FUNCTION DEFINITIONS     **********/
static void system_init_fpu()
{
	SCB->CPACR = SCB_CPACR_CP10_FULL_ACCESS | SCB_CPACR_CP11_FULL_ACCESS;		//enables the FPU.
}

/**********     GLOBAL FUNCTION DEFINITIONS     **********/
void system_init()
{
	rcc_main_clock_config();

	rcc_init_systick();

	system_init_fpu();

	hsem_init_clk();

	fmc_init_sdram();

	system_init_shared_mem();

	io_init();

	lcd_init();

	//TODO:REMOVE COMMENT touch_init();

	can_init_clk();

	hsem_lock(1, 0);
	hsem_signal(1, 0);
	hsem_wait_void(1, 0);

}

void system_run()
{

	/*Run the gauges.*/
	//system_set_app_ptr(app_can_baud_rate_run);

	/*Enter endless loop while app_gauges runs.*/
	while (1)
	{
		if (system_app_ptr != NULL)
		{
			system_app_ptr();
		}
		//lv_timer_handler();
		lcd_solid_color_test_black();
		timer_delay_ms(500);
		lcd_solid_color_test_red();
		timer_delay_ms(500);
		lcd_solid_color_test_blue();
		timer_delay_ms(500);
		lcd_solid_color_test_green();
		timer_delay_ms(500);
	}
}

void system_set_app_ptr(void (*app_ptr)())
{
	system_app_ptr = app_ptr;
}

void system_hsem_handler()
{

}

#endif	//CORE_CM7
