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
#include "lvgl_port/lvgl_port_def.h"
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

static TaskFunction_t prv_task_blink();
static TaskFunction_t prv_system_error_handler();
static void prv_lcd_bl_init();

/**********     STATIC FUNCTION DEFINITIONS     **********/
static TaskFunction_t prv_task_blink(const uint32_t delay_time_ms)
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
	/* Read the backlight value from the config file. */
	char backlight_str[17];
	sys_mem_get_config_data("BRIGHTNESS", &backlight_str);

	char* sv_ptr;
	char* split[2];
	split[0] = strtok_r(&backlight_str, ",", &sv_ptr);
	split[1] = strtok_r(NULL, ",", &sv_ptr);
	uint32_t backlight_int = atoi(split[1]);

	io_set_pin_mux(GPIOB, GPIO_PIN14_Msk, GPIO_AFR_AF2);
	timer_init(TIM12);
	timer_enable_pwm_output(TIM12, 1);
	timer_set_pwm_freq(TIM12, 100);
	timer_set_pwm_duty_cycle(TIM12, backlight_int, 1);
	timer_enable(TIM12);
}

/**********     GLOBAL FUNCTION DEFINITIONS     **********/
void system_task_init()
{
	/* Reset the I2C bus. */
	i2c_bus_reset(I2C4);

	/* Initialize the file systems. This has to come after I2C init bc EEPROM uses I2C. */
	sys_mem_init_file_systems();

	error_handler_run();

	/* This has to come after the file system because we save the screen brightness in the config file. */
	prv_lcd_bl_init();

	/* Start all the tasks. */
	system_blink_run(1000);

	pwr_monitor_run(3);

	/* Set up the display and input device callbacks for LVGL. */
   	static touch_info_t touch_data;						//Where the touch data will be stored.
   	static touch_info_t* p_touch_data = &touch_data;	//Pointer to the touch data for indev_init.
	lv_port_run();										//Initialize LVGL and LVGL mutex.
	disp_init();										//LVGL display bindings.
	indev_init(&p_touch_data);							//LVGL input device callback (touch screen).
	touch_scr_run(p_touch_data);						//Runs the touch screen task.

	//system_run_runtime_stats_task();
	//system_run_heap_stats_task();

	/* Load the menu screen. */
	app_gauges_run();
	//lv_demo_benchmark();

	char last_error_str[50];
	memset(last_error_str, 0, 50 * sizeof(char));
	sys_mem_get_config_data("LAST ERROR", last_error_str);
	char* error_code_str = sys_mem_csv_split(last_error_str, 1);
	if (strcmp((const char*)error_code_str, "NONE") != 0)
	{
		sys_mem_get_config_data("LAST ERROR", last_error_str);
		lv_obj_t* error_msgbox = ui_helpers_show_msgbox(last_error_str, NULL, NULL);
		ui_helpers_add_msgbox_close_btn(error_msgbox, NULL);
		sys_mem_set_config_data("LAST ERROR,NONE,\n");
	}

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

	/*Enable the caches.*/
#if SYS_ENABLE_CACHE == 1
	SCB_EnableDCache();
	SCB_EnableICache();
#endif
	/* Configure the IO pins for I2C. */
	io_set_output_type(GPIOD, GPIO_PIN12_Msk, IO_OUTPUT_TYPE_OPEN_DRAIN);
	io_set_output_type(GPIOD, GPIO_PIN13_Msk, IO_OUTPUT_TYPE_OPEN_DRAIN);
	io_set_pin_mux(GPIOD, GPIO_PIN12_Msk, GPIO_AFR_AF4);
	io_set_pin_mux(GPIOD, GPIO_PIN13_Msk, GPIO_AFR_AF4);

	/* Initialize the I2C interface. Used by the LCD screen and EEPROM. */
	i2c_init(I2C4);
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
	xTaskCreate((TaskFunction_t)prv_task_blink, "SYS_BLINK", 600 / 4, (void*)delay_time_ms, 4, &prv_task_handle_blink);

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
	timer_ms_inc(1);
}

void system_set_lcd_backlight(bool on)
{
	/* TODO: Edit this so the whole buck converter is disabled when off. */
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


