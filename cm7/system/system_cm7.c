/*
 * system.c
 *
 *  Created on: May 31, 2024
 *      Author: awjpp
 */


/**********		INCLUDES		**********/
#include "application/applications_cm7.h"
#include "cangauge_common.h"
#include "system_cm7.h"

#include "drivers/drivers.h"
#include "drivers/stm32_canbus.h"

#include "lvgl_port/indev.h"
#include "lvgl_port/disp.h"

#include "ui/ui_helpers.h"
#include "ui/ui_car_menu.h"



/**********		DEFINES		**********/
#define SCB_CPACR_CP10_FULL_ACCESS			0x3 << 20
#define SCB_CPACR_CP11_FULL_ACCESS			0x3 << 22
#define TEST_LED_PORT						GPIOB
#define TEST_LED_PIN						GPIO_PIN15_Msk
#define SYS_GET_WATERMARK					system_stack_watermark = uxTaskGetStackHighWaterMark(NULL);

/**********		GLOBAL VARIABLE DEFINITIONS		**********/
SemaphoreHandle_t sys_mutex_lvgl = NULL;

/**********     STATIC VARIABLES     **********/
volatile UBaseType_t system_stack_watermark;


/**********     STATIC FUNCTION DECLARATIONS     **********/
static void prv_init_fpu();

/**********     STATIC FUNCTION DEFINITIONS     **********/
static void prv_init_fpu()
{
	SCB->CPACR = SCB_CPACR_CP10_FULL_ACCESS | SCB_CPACR_CP11_FULL_ACCESS;		//enables the FPU.
}

/**********     GLOBAL FUNCTION DEFINITIONS     **********/
void system_task_init()
{
	/*Set up and enable all the clocks.*/
	hsem_init_clk();
	/*Taking HSEM 1 to hold CM4 in place.*/
	hsem_lock(HSEM_INIT, HSEM_ID_INIT_CM7);

	/*Enable all the IO clocks.*/
	io_init();

	/*Turn on the test LED.*/
	io_init_test_led(TEST_LED_PORT, TEST_LED_PIN);
	io_test_led_on();
	
	/*Initialize the FPU.*/
	prv_init_fpu();

	/*LCD and LVGL.*/
	lcd_init();						//The LTDC.

	/* LCD backlight power supply and CAN transceivers enable pin. */
	io_set_pin_dir_out(GPIOK, GPIO_PIN2_Msk);
	io_pin_out_clr(GPIOK, GPIO_PIN2_Msk);

	/*Enable the caches.*/
	SCB_EnableDCache();
	SCB_EnableICache();

	/**** TESTING USB CONFIGURATION *****/
	io_set_pin_mux(GPIOA, GPIO_PIN10_Msk, GPIO_AFR_AF10);
	io_set_pin_mux(GPIOA, GPIO_PIN11_Msk, GPIO_AFR_AF10);
	io_set_pin_mux(GPIOA, GPIO_PIN12_Msk, GPIO_AFR_AF10);

	RCC->CR |= RCC_CR_HSI48ON;
	while ((RCC->CR & RCC_CR_HSI48RDY) == 0) {}

	RCC->D2CCIP2R |= 0x3 << RCC_D2CCIP2R_USBSEL_Pos;	//Enable kernel clock.
	RCC->AHB1ENR |= RCC_AHB1ENR_USB2OTGFSEN | RCC_AHB1ENR_USB2OTGFSULPIEN;	//Enable PHY and peripheral clocks.

	USB2_OTG_FS->GUSBCFG |= USB_OTG_GUSBCFG_FDMOD;


	/***********************************/


	/*
	 * Signal to CM4 that were done with system init.
	 * Then wait for CM4 to be done initializing.
	 */
	hsem_signal(HSEM_INIT, HSEM_ID_INIT_CM7);
	hsem_wait_void(HSEM_INIT, HSEM_ID_INIT_CM4);
	hsem_clear_int(1);

	sys_mutex_lvgl = xSemaphoreCreateMutex();
	if (sys_mutex_lvgl == NULL)
	{
		xTaskCreate(system_task_blink, "SYS_BLINK", 50, 100, 4, NULL);
	}
	else
	{

		xTaskCreate((TaskFunction_t)system_task_blink, "SYS_BLINK", 50, 1000, 4, NULL);
		app_menu_run();
		xTaskCreate(system_task_lvgl_timer_update, "LVGL_TASK_HANDLER", 1500, NULL, 2, NULL);
		xTaskCreate(app_battery_monitor_task, "BATT_MON", 32, NULL, 4, &app_battery_monitor_task_handle);
	}

	vTaskDelete(NULL);
}

void system_task_lvgl_timer_update()
{
	while (1)
	{
		if (xSemaphoreTake(sys_mutex_lvgl, portMAX_DELAY) == pdPASS)
		{
			uint32_t time_till_next = lv_task_handler();
			xSemaphoreGive(sys_mutex_lvgl);
			vTaskDelay(pdMS_TO_TICKS(time_till_next));
		}
	}
}

void system_task_blink(const uint32_t delay_time_ms)
{
	TickType_t last_run_time;
	last_run_time = xTaskGetTickCount();
	while(1)
	{
		io_test_led_tgl();
		vTaskDelayUntil(&last_run_time, delay_time_ms);
	}
}

void vApplicationTickHook()
{
	lv_tick_inc(pdTICKS_TO_MS(1));
}

