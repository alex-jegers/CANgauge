/**********     INCLUDES        **********/
#include "pwr_monitor.h"
#include "application/applications_cm7.h"
#include "common/drivers/drivers.h"
#include "lvgl_port/lvgl_port_def.h"
#include <assert.h>

/**********		DEFINES		**********/
/*This is GPIOB, GPIO_PIN5_Msk for prod hw.*/
#define BATT_MON_io             GPIOB, GPIO_PIN1_Msk
#define ADC_CHANNEL_NUMBER		5

/**********		EXTERNAL VARIABLE DEFINITIONS		**********/

/**********		STATIC VARIABLES		**********/
TaskHandle_t task_handle_battery_monitor = NULL;
static uint32_t prv_measurement = 0;
static uint32_t prv_low_threshold = 0x9700;
static uint32_t prv_high_threshold = 0xA1BC;
static uint32_t prv_rolling_avg = 0;
static uint32_t prv_rolling_avg_buffer[20];

/**
 * prv_rolling_avg_valid:
 * desc: set to true when the rolling avg has accumulated >= its total
 * 		number of samples.
 */
static bool prv_rolling_avg_valid = false;

/**********		STATIC FUNCTION DECLRATIONS		**********/
static void prv_adc_interrupt_handler();
static void prv_update_rolling_average();
static void prv_pwr_monitor_task();


/**********		STATIC FUNCTION DEFINITIONS		**********/
static void prv_adc_interrupt_handler()
{
    if (adc_get_interrupt(ADC1, ADC_INT_FLAG_END_OF_CONVERSION))
    {
        prv_measurement = adc_get_conversion(ADC1);
    }
}

static void prv_update_rolling_average()
{
    const uint16_t num_samples = sizeof(prv_rolling_avg_buffer)/sizeof(uint32_t);
    static uint8_t samples_taken = 0;
    uint32_t accumulator = 0;

    if (samples_taken <= num_samples)
    {
    	if (samples_taken >= num_samples)
    	{
    		prv_rolling_avg_valid = true;
    	}
    	samples_taken++;
    }

    /* Move everything down. */
    for (uint8_t i = num_samples - 1; i > 0; i--)
    {
        prv_rolling_avg_buffer[i] = prv_rolling_avg_buffer[i - 1];
    }

    prv_rolling_avg_buffer[0] = prv_measurement;

    for (uint8_t i = 0; i < num_samples; i++)
    {
        accumulator += prv_rolling_avg_buffer[i];
    }

    prv_rolling_avg = accumulator / num_samples;
}

static void prv_pwr_monitor_task()
{
    io_set_pin_analog(BATT_MON_io);
    rcc_select_per_ck(PER_CK_SRC_HSI);
    adc_init_clk(ADC1, ADC_CK_SRC_PER_CK);
    adc12_set_clock_prescaler(ADC_PRESCALER_DIV_2);      //Sets ADC clock to 4MHz, max is 6.25MHz with no BOOST.
    adc_set_sample_time(ADC1, ADC_SAMPLE_TIME_387_CYCLES, ADC_CHANNEL_NUMBER);
    adc_set_channel(ADC1, ADC_CHANNEL_NUMBER);
    adc12_set_int_handler(prv_adc_interrupt_handler);
    adc_enable_interrupt(ADC1, ADC_INT_FLAG_END_OF_CONVERSION);
    adc12_enable_nvic_interrupts();
    adc_enable(ADC1);
    while (adc_get_interrupt(ADC1, ADC_INT_FLAG_ADC_RDY) == 0);
    adc_start_conversion(ADC1);
    
    while (!prv_rolling_avg_valid)
    {
        prv_update_rolling_average();
        adc_start_conversion(ADC1);
        vTaskDelay(250);
    }

    while (1)
    {
    	/* If we're lower than the threshold. */
    	if (prv_measurement < prv_low_threshold)
    	{
            /* Check if were in higher power mode, if yes, need to change. */
            if (pwr_get_current_vos_mode() == PWR_VOS_MODE_SCALE_0)
            {
                /* Stop all other tasks. */
                app_gauges_stop(portMAX_DELAY);
                app_can_controller_stop(  pdMS_TO_TICKS(1000)  );
                can_transmit_stop(  pdMS_TO_TICKS(1000)  );

                lv_port_stop(  pdMS_TO_TICKS(1000)  );
                assert(  touch_scr_stop(  pdMS_TO_TICKS(1000)  )  );
                system_blink_stop(  portMAX_DELAY  );

            	taskENTER_CRITICAL();

                /* Switch to the HSI to re-configure the PLLs. */
                rcc_set_sys_ck(RCC_SYS_CK_HSI);

                /* Disable the PLLs. */
                rcc_disable_all_pll();

                /* Lower the system power mode. */
                pwr_set_vos_mode(PWR_VOS_MODE_SCALE_3);
                
                /* Adjust SysTick frequency. */
                rcc_set_systick_reload(1000);



                /* Turn off the backlight and CAN transcievers. */
                system_set_lcd_backlight(false);
                system_set_can_transc(false);

                /* Turn off the clocks to all the io ports that we're not using. */
                io_deinit_gpioa();
                io_deinit_gpioc();
                io_deinit_gpioe();
                io_deinit_gpiof();
                io_deinit_gpiog();
                io_deinit_gpioh();
                io_deinit_gpioj();

                taskEXIT_CRITICAL();
            }
            else
            {

            }
    	}

    	/* If we're higher than the threshold. */
    	if (prv_measurement > prv_high_threshold)
    	{
            /* Check if were in lower power mode, if we are, reset the system. */
            if (pwr_get_current_vos_mode() == PWR_VOS_MODE_SCALE_3)
            {
            	rcc_sw_reset();
            }
    	}
        prv_update_rolling_average();
        adc_start_conversion(ADC1);
        vTaskDelay(1000);
    }
    
}

/**********		GLOBAL FUNCTION DEFINITIONS		**********/
void pwr_monitor_run(uint8_t priority)
{
	if (task_handle_battery_monitor != NULL)
	{
		/* Trying to start the task but it's already running. */
		assert(0);
	}
	xTaskCreate(prv_pwr_monitor_task, "BATT_MON", 128, NULL, priority, &task_handle_battery_monitor);
}
