/*
 * system.c
 *
 *  Created on: May 31, 2024
 *      Author: awjpp
 */

#ifdef CORE_CM7

/**********		INCLUDES		**********/
#include "system_cm7.h"
#include "assert.h"

#include "drivers/stm32_io.h"
#include "drivers/stm32_lcd.h"
#include "drivers/stm32_fmc.h"
#include "drivers/stm32_rcc.h"
#include "drivers/stm32_hsem.h"

#include "application/app_ui_test_cm7.h"
#include "application/app_can_sniffer_cm7.h"

#include "touch_screen/iic_touch.h"
#include "touch_screen/cst830_touch_cm7.h"
#include "lvgl/lvgl.h"

#include "ui/ui_helpers.h"
#include "ui/ui_car_menu.h"

#include "app_shared_mem.h"


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
static void _init_fpu();
static void _can_sniffer_btn_hanlder(lv_event_t* e);
static void _task_test();

/**********     STATIC FUNCTION DEFINITIONS     **********/
static void _init_fpu()
{
	SCB->CPACR = SCB_CPACR_CP10_FULL_ACCESS | SCB_CPACR_CP11_FULL_ACCESS;		//enables the FPU.
}

static void _can_sniffer_btn_hanlder(lv_event_t* e)
{
	assert(xTaskCreate(app_can_sniffer_cm7, "CAN_SNIFFER", 500, NULL, 0, NULL));
}

static void _task_test()
{
	while (1)
	{
		if (LTDC_Layer1->CFBAR == 0xd0000000)
		{
			LTDC_Layer1->CFBAR = 0xd00A8C00;
			LTDC->SRCR = LTDC_SRCR_VBR;
		}
		else if (LTDC_Layer1->CFBAR == 0xd00A8C00)
		{
			LTDC_Layer1->CFBAR = 0xd0000000;
			LTDC->SRCR = LTDC_SRCR_VBR;
		}
		vTaskDelay(pdMS_TO_TICKS(33));
	}
}

/**********     GLOBAL FUNCTION DEFINITIONS     **********/
void system_task_init()
{
	/*Non-time dependent initializations.*/
	hsem_init_clk();
	/*Taking HSEM 1 to hold CM4 in place.*/
	hsem_lock(HSEM_INIT, HSEM_ID_INIT_CM7);
	io_init();
	io_init_test_led(TEST_LED_PORT, TEST_LED_PIN);
	io_test_led_on();
	_init_fpu();

	system_init_shared_mem();

	/*Time dependent initializations.*/
	lcd_init();
	#ifdef TARGET_HARDWARE_CANGAUGE
	cst830_init();
	#endif
	#ifdef TARGET_HARDWARE_STM32H745DISCO
	touch_init();
	#endif

	SCB_EnableDCache();
	SCB_EnableICache();

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
		xTaskCreate(system_task_blink, "SYS_BLINK", 50, NULL, 4, NULL);
	}
	else
	{
		ui_car_load_menu_screen();
		ui_car_set_can_sniffer_btn_clicked_cb(_can_sniffer_btn_hanlder);
		xTaskCreate(system_task_lvgl_timer_update, "LVGL_TASK_HANDLER", 1500, NULL, 1, NULL);
		//xTaskCreate(_task_test, "TASK_TEST", 1500, NULL, 1, NULL);
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

void system_task_blink()
{
	TickType_t last_run_time;
	last_run_time = xTaskGetTickCount();
	while(1)
	{
		io_test_led_tgl();
		vTaskDelayUntil(&last_run_time, 250);
	}
}

void vApplicationTickHook()
{
	lv_tick_inc(pdTICKS_TO_MS(1));
}

#endif	//CORE_CM7
