/**********     INCLUDES        **********/
#include "runtime_stats.h"
#include "drivers/drivers.h"
#include "system/system_cm7.h"
#include "task.h"

/**********     TYPEDEFS         **********/

/**********		DEFINES		**********/

/**********		EXTERNAL VARIABLE DEFINITIONS		**********/

/**********		STATIC VARIABLES		**********/
TaskHandle_t prv_runtime_stats_handle = NULL;
/**********		STATIC FUNCTION DECLRATIONS		**********/
static void prv_high_freq_timer_tick_int();

/**********		STATIC FUNCTION DEFINITIONS		**********/
static void prv_high_freq_timer_tick_int()
{
	timer_us_inc(10);
}

TaskFunction_t prv_runtime_stats_task()
{
	static char* buf[500];
	uint32_t counter = 0;
	while (1)
	{
		vTaskDelay(5000);
		vTaskGetRunTimeStats(buf);
		counter++;
	}
}
/**********		GLOBAL FUNCTION DEFINITIONS		**********/
void system_run_runtime_stats_task()
{
	xTaskCreate(prv_runtime_stats_task, "RUNTIME_STATS", 1000, NULL, 4, &prv_runtime_stats_handle);
}

void system_config_runtime_stats()
{
	timer_init(TIM13);
	timer_set_pwm_freq(TIM13, 10000);
	timer_set_pwm_duty_cycle(TIM13, 65535, 1);
	timer_enable_compare_interrupt(TIM13);
	timer_enable(TIM13);
	NVIC_SetPriority(TIM8_UP_TIM13_IRQn, 0xa);
	NVIC_EnableIRQ(TIM8_UP_TIM13_IRQn);
	timer_set_tim13_int_handler(prv_high_freq_timer_tick_int);
}
uint32_t system_get_runtime_value()
{
	return timer_get_time_us();
}
