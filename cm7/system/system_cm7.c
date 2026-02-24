/*
 * system.c
 *
 *  Created on: May 31, 2024
 *      Author: awjpp
 */


/**********		INCLUDES		**********/
#include "application/applications_cm7.h"
#include "system_cm7.h"

#include "drivers/drivers.h"

#include "ui/ui_helpers.h"
#include "ui/ui_car_menu.h"



/**********		DEFINES		**********/
#define SCB_CPACR_CP10_FULL_ACCESS			0x3 << 20
#define SCB_CPACR_CP11_FULL_ACCESS			0x3 << 22
#define TEST_LED_PORT						GPIOB
#define TEST_LED_PIN						GPIO_PIN15_Msk
#define SYS_GET_WATERMARK					system_stack_watermark = uxTaskGetStackHighWaterMark(NULL);

#define EVENT_BITS_BLINK_TASK_STOPPED		(EventBits_t)0x01	//Bit is set when blink is stopped, clear when task is created.

/**********		GLOBAL VARIABLE DEFINITIONS		**********/

/**********     STATIC VARIABLES     **********/
volatile UBaseType_t prv_system_stack_watermark;
static TaskHandle_t prv_task_handle_blink = NULL;
static uint32_t prv_blink_delay_on = 0;
static uint32_t prv_blink_delay_off = 0;
static bool prv_run_blink = false;
static EventGroupHandle_t prv_event_group = NULL;

/**********     STATIC FUNCTION DECLARATIONS     **********/

static void prv_task_blink();
static void prv_lcd_bl_init();

/**********     STATIC FUNCTION DEFINITIONS     **********/
void prv_task_blink(const uint32_t delay_time_ms)
{
	TickType_t last_run_time;
	last_run_time = xTaskGetTickCount();
	prv_blink_delay_off = delay_time_ms;
	prv_blink_delay_on = delay_time_ms;

	xEventGroupClearBits(prv_event_group, EVENT_BITS_BLINK_TASK_STOPPED);
	prv_run_blink = true;
	while(prv_run_blink)
	{
		io_test_led_on();
		vTaskDelayUntil(&last_run_time, pdMS_TO_TICKS(prv_blink_delay_on));
		io_test_led_off();
		vTaskDelayUntil(&last_run_time, pdMS_TO_TICKS(prv_blink_delay_off));
	}
	xEventGroupSetBits(prv_event_group, EVENT_BITS_BLINK_TASK_STOPPED);
	vTaskDelete(NULL);
}

static void prv_lcd_bl_init()
{
	io_init();
	//io_set_pin_dir_out(GPIOB, GPIO_PIN14_Msk);
	//io_pin_out_set(GPIOB, GPIO_PIN14_Msk);
	//TODO: Double check this PWM code.
	io_set_pin_mux(GPIOB, GPIO_PIN14_Msk, GPIO_AFR_AF2);
	timer_init(TIM12);
	timer_enable_pwm_output(TIM12, 1);
	timer_set_pwm_freq(TIM12, 100);
	timer_set_pwm_duty_cycle(TIM12, 0xFFFF, 1);
	timer_enable(TIM12);

}

/**********     GLOBAL FUNCTION DEFINITIONS     **********/
void system_task_init()
{
	/* Stop the scheduler. */
	vTaskSuspendAll();

	/*Enable all the IO clocks.*/
	io_init();

	/*Turn on the test LED.*/
	io_init_test_led(TEST_LED_PORT, TEST_LED_PIN);
	io_test_led_on();

	/* LCD backlight power supply and CAN transceivers enable pin. */
	io_set_pin_dir_out(GPIOK, GPIO_PIN2_Msk);
	io_pin_out_clr(GPIOK, GPIO_PIN2_Msk);
	prv_lcd_bl_init();

	/*Enable the caches.*/
	SCB_EnableDCache();
	SCB_EnableICache();

	/* Create the private event group. */
	prv_event_group = xEventGroupCreate();
	xEventGroupSetBits(prv_event_group, EVENT_BITS_BLINK_TASK_STOPPED);

	/**** TESTING USB CONFIGURATION *****/
	/*
	io_set_pin_mux(GPIOA, GPIO_PIN10_Msk, GPIO_AFR_AF10);
	io_set_pin_mux(GPIOA, GPIO_PIN11_Msk, GPIO_AFR_AF10);
	io_set_pin_mux(GPIOA, GPIO_PIN12_Msk, GPIO_AFR_AF10);

	RCC->CR |= RCC_CR_HSI48ON;
	while ((RCC->CR & RCC_CR_HSI48RDY) == 0) {}

	RCC->D2CCIP2R |= 0x3 << RCC_D2CCIP2R_USBSEL_Pos;	//Enable kernel clock.
	RCC->AHB1ENR |= RCC_AHB1ENR_USB2OTGFSEN | RCC_AHB1ENR_USB2OTGFSULPIEN;	//Enable PHY and peripheral clocks.

	USB2_OTG_FS->GUSBCFG |= USB_OTG_GUSBCFG_FDMOD;

*/
	/***********************************/

	system_blink_run(1000);

	pwr_monitor_run(4);

	xTaskResumeAll();
	
	vTaskDelete(NULL);
}

void system_init_fpu()
{
	SCB->CPACR = SCB_CPACR_CP10_FULL_ACCESS | SCB_CPACR_CP11_FULL_ACCESS;		//enables the FPU.
}

void system_blink_run(const uint32_t delay_time_ms)
{
	if (prv_task_handle_blink != NULL)
	{
		vTaskResume(prv_task_handle_blink);
		return;
	}
	xTaskCreate((TaskFunction_t)prv_task_blink, "SYS_BLINK", 50, delay_time_ms, 4, &prv_task_handle_blink);

}

void system_blink_set_delay(uint32_t on_ms, uint32_t off_ms)
{
	if (on_ms > 0)
	{
		prv_blink_delay_on = on_ms;
	}
	if (off_ms > 0)
	{
		prv_blink_delay_off = off_ms;
	}
}

bool system_blink_stop(uint32_t block_time_ms)
{
	prv_run_blink = false;
	uint32_t rtn = xEventGroupWaitBits(prv_event_group, EVENT_BITS_BLINK_TASK_STOPPED,
										pdFALSE, pdTRUE, block_time_ms);
	return rtn & EVENT_BITS_BLINK_TASK_STOPPED;
}



void vApplicationTickHook()
{
	lv_tick_inc(pdTICKS_TO_MS(1));
}

void system_set_lcd_backlight(bool on)
{
	if (on)
	{
		io_set_pin_mux(GPIOB, GPIO_PIN14_Msk, GPIO_AFR_AF2);
	}
	else
	{
		io_set_pin_dir_out(GPIOB, GPIO_PIN14_Msk);
		io_pin_out_clr(GPIOB, GPIO_PIN14_Msk);
	}
}

void system_set_can_transc(bool on)
{
	if (on)
	{
		io_set_pin_dir_out(GPIOK, GPIO_PIN2_Msk);
		io_pin_out_clr(GPIOK, GPIO_PIN2_Msk);
	}
	else
	{
		io_set_pin_dir_out(GPIOK, GPIO_PIN2_Msk);
		io_pin_out_set(GPIOK, GPIO_PIN2_Msk);
	}
}
