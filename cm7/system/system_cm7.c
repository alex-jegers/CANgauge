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
#include "ui/ui_gauges.h"


/**********		DEFINES		**********/
#define SCB_CPACR_CP10_FULL_ACCESS			0x3 << 20
#define SCB_CPACR_CP11_FULL_ACCESS			0x3 << 22
#define SYS_GET_WATERMARK					system_stack_watermark = uxTaskGetStackHighWaterMark(NULL);
#define TEST_LED_PORT						GPIOB
#define TEST_LED_PIN						GPIO_PIN15_Msk


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
	/* Create the private event group if it hasnt been created yet. */
	if (prv_event_group == NULL)
	{
		prv_event_group = xEventGroupCreate();
	}
	xEventGroupSetBits(prv_event_group, EVENT_BITS_BLINK_TASK_STOPPED);

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
	//portENTER_CRITICAL();	//Have to use this instead of vTaskSuspendAll because we need to use a delay after resetting USB.
	vTaskSuspendAll();
/*
	i2c_exit_code_t present_code = eeprom_present();
	static uint8_t test_data_wr[8] = { 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0 };
	eeprom_write(0x0200, &test_data_wr, 8);
	static uint8_t test_data_rd[8] = { 0,0,0,0,0,0,0,0 };
	i2c_exit_code_t status = eeprom_status();
	while (status != I2C_EXIT_CODE_TC) {status = eeprom_status();}
	i2c_exit_code_t read_code = eeprom_read(&test_data_rd, 0x0200, 8);
*/

	i2c_bus_reset(I2C4);
	xTaskResumeAll();
	//portEXIT_CRITICAL();

	vTaskDelete(NULL);
}

void system_init()
{
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

	/**** TESTING USB CONFIGURATION *****/
	usb_init();
	usb_core_reset();
	usb_init_core();
	/***********************************/

	/* Configure the IO pins for I2C. */
	io_set_output_type(GPIOD, GPIO_PIN12_Msk, IO_OUTPUT_TYPE_OPEN_DRAIN);
	io_set_output_type(GPIOD, GPIO_PIN13_Msk, IO_OUTPUT_TYPE_OPEN_DRAIN);
	io_set_pin_mux(GPIOD, GPIO_PIN12_Msk, GPIO_AFR_AF4);
	io_set_pin_mux(GPIOD, GPIO_PIN13_Msk, GPIO_AFR_AF4);

	/* Initialize the I2C interface. Used by the LCD screen and EEPROM. */
	i2c_init_clk(I2C4);
	i2c_set_clk_speed(I2C4, I2C_CLK_400K);
	i2c_disable_analog_filt(I2C4);
	i2c_enable_timeout_detection(I2C4);
	i2c_enable(I2C4);
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
	timer_inc(1);
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
