/**********     INCLUDES        **********/
#include "app_battery_monitor.h"
#include "common/drivers/stm32_adc.h"
#include "common/drivers/stm32_io.h"

/**********		DEFINES		**********/
/*This is GPIOB, GPIO_PIN5_Msk for prod hw.*/
#define BATT_MON_io             GPIOB, GPIO_PIN1_Msk
#define ADC_CHANNEL_NUMBER		5

/**********		EXTERNAL VARIABLE DEFINITIONS		**********/
TaskHandle_t app_battery_monitor_task_handle = NULL;

/**********		STATIC VARIABLES		**********/
static uint32_t prv_measurement = 0;
static uint32_t prv_low_threshold = 0x7A2A;
static uint32_t prv_high_threshold = 0xA1A8;

/**********		STATIC FUNCTION DECLRATIONS		**********/
static void prv_adc_interrupt_handler();

/**********		STATIC FUNCTION DEFINITIONS		**********/
static void prv_adc_interrupt_handler()
{
    if (adc_get_interrupt(ADC1, ADC_INT_FLAG_END_OF_CONVERSION))
    {
        prv_measurement = adc_get_conversion(ADC1);
    }
}
/**********		GLOBAL FUNCTION DEFINITIONS		**********/
void app_battery_monitor_task()
{
    io_set_pin_analog(BATT_MON_io);
    adc_init_clk(ADC1);
    adc12_set_clock_prescaler(ADC_PRESCALER_DIV_10);      //Sets ADC clock to 3.314MHz, max is 6.25MHz with no BOOST.
    adc_set_sample_time(ADC1, ADC_SAMPLE_TIME_387_CYCLES, ADC_CHANNEL_NUMBER);
    adc_set_channel(ADC1, ADC_CHANNEL_NUMBER);
    adc12_set_int_handler(prv_adc_interrupt_handler);
    adc_enable_interrupt(ADC1, ADC_INT_FLAG_END_OF_CONVERSION);
    adc12_enable_nvic_interrupts();
    adc_enable(ADC1);
    while (adc_get_interrupt(ADC1, ADC_INT_FLAG_ADC_RDY) == 0);
    adc_start_conversion(ADC1);
    
    while (1)
    {
    	if (prv_measurement < prv_low_threshold)
    	{
    		io_test_led_off();
    	}

    	if (prv_measurement > prv_high_threshold)
    	{
    		io_test_led_on();
    	}

        adc_start_conversion(ADC1);
        vTaskDelay(1000);
    }
    
}
