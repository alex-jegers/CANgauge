
#ifndef _STM32_ADC_H_
#define _STM32_ADC_H_

#ifdef __cplusplus
extern "C" {
#endif

/**********     INCLUDES        **********/
#include "stm32h745xx.h"
#include <stddef.h>

/**********     DEFINES      **********/
typedef enum
{
	ADC_RESOLUTION_16_BIT 	= 0x00,
	ADC_RESOLUTION_14_BIT 	= 0x05,
	ADC_RESOLUTION_12_BIT 	= 0x06,
	ADC_RESOLUTION_10_BIT 	= 0x03,
	ADC_RESOLUTION_8_BIT 	= 0x07,
}adc_resolution_t;

typedef enum
{
	ADC_SAMPLE_TIME_1_CYCLES	= 0x00,
	ADC_SAMPLE_TIME_2_CYCLES,
	ADC_SAMPLE_TIME_8_CYCLES,
	ADC_SAMPLE_TIME_16_CYCLES,
	ADC_SAMPLE_TIME_32_CYCLES,
	ADC_SAMPLE_TIME_64_CYCLES,
	ADC_SAMPLE_TIME_387_CYCLES,
	ADC_SAMPLE_TIME_810_CYCLES,
}adc_sample_time_t;

typedef enum
{
	ADC_PRESCALER_DIV_1		= 0x00,
	ADC_PRESCALER_DIV_2,
	ADC_PRESCALER_DIV_4,
	ADC_PRESCALER_DIV_6,
	ADC_PRESCALER_DIV_8,
	ADC_PRESCALER_DIV_10,
	ADC_PRESCALER_DIV_12,
	ADC_PRESCALER_DIV_16,
	ADC_PRESCALER_DIV_32,
	ADC_PRESCALER_DIV_64,
	ADC_PRESCALER_DIV_128,
	ADC_PRESCALER_DIV_256,
}adc_prescaler_t;

typedef enum
{
	ADC_INT_FLAG_ADC_RDY				= 0x01,
	ADC_INT_FLAG_END_OF_SAMPLING		= 0x02,
	ADC_INT_FLAG_END_OF_CONVERSION		= 0x04,
	ADC_INT_FLAG_END_OF_REG_SEQ			= 0x08, //End of regular sequence.
	ADC_INT_FLAG_OVERRUN				= 0x10,
	ADC_INT_FLAG_INJ_END_OF_CONVERSION	= 0x20,
	ADC_INT_FLAG_INJ_END_OF_SEQ			= 0x40,
	ADC_INT_FLAG_WATCHDOG_1				= 0x80,
	ADC_INT_FLAG_WATCHDOG_2				= 0x100,
	ADC_INT_FLAG_WATCHDOG_3				= 0x200,
	ADC_INT_FLAG_INJ_QUEUE_OVERFLOW		= 0x400,
	ADC_INT_FLAG_LDORDY					= 0x1000,
	ADC_INT_FLAG_ALL					= 0x17FF,
}adc_int_flag_t;

typedef enum
{
	ADC_CK_SRC_PLL2_P,
	ADC_CK_SRC_PLL3_R,
	ADC_CK_SRC_PER_CK,
}adc_ck_src_t;

/**********     GLOBAL VARIABLE DECLRATIONS     **********/

/**********		GLOBAL FUNCTION DECLRATIONS		**********/
/**
 * adc_init_clk:
 * desc: select the kernel clock source and enable the AHB clock.
 */
void adc_init_clk(ADC_TypeDef* adc, adc_ck_src_t ck_src);
void adc_enable(ADC_TypeDef* adc);
void adc_disable(ADC_TypeDef* adc);
/**
 * func: adc_start_conversion
 * description:
 * starts an ADC conversion - adc_enable must be called first.
 */
void adc_start_conversion(ADC_TypeDef* adc);			
void adc_stop_conversion(ADC_TypeDef* adc);
void adc_set_continuous_mode(ADC_TypeDef* adc);
void adc_set_resolution(ADC_TypeDef* adc, adc_resolution_t res);
void adc_enable_watchdog_1(ADC_TypeDef* adc);
/**
 * func: adc_select_watchdog_1_channel
 * params:
 * adc: pointer to an ADC instance.
 * channel: number 0 thru 19 corresponding with adc channel number.
 */
void adc_select_watchdog_1_channel(ADC_TypeDef* adc, uint8_t channel);
void adc_set_watchdog_1_low_threshold(ADC_TypeDef* adc, uint32_t threshold);
void adc_set_watchdog_1_high_threshold(ADC_TypeDef* adc, uint32_t threshold);
/**
 * func: adc_set_sample_time
 * description: set the sample time in ADC clock cycles. 
 * params:
 * channel: channel number as int between 0 and 19
 */	
void adc_set_sample_time(ADC_TypeDef* adc, adc_sample_time_t sample_time, uint8_t channel);
void adc_set_channel(ADC_TypeDef* adc, uint8_t channel);
/**
 * func: adc_get_conversion
 * return:
 * returns the conversion result from the last converted regular channel.
 */
uint32_t adc_get_conversion(ADC_TypeDef* adc);
/** 
 * func: adc_get_interrupt
 * description:
 * used to check the status of an interrupt or status bit.
 * returns:
 * returns zero if the bit is not set. returns non-zero if the bit is set.
 */
uint32_t adc_get_interrupt(ADC_TypeDef* adc, adc_int_flag_t interrupt);
void adc_clear_interrupt(ADC_TypeDef* adc, adc_int_flag_t interrupt);
void adc_enable_interrupt(ADC_TypeDef* adc, adc_int_flag_t interrupt);
void adc_disable_interrupt(ADC_TypeDef* adc, adc_int_flag_t interrupt);

void adc12_set_clock_prescaler(adc_prescaler_t prescaler);
void adc12_enable_nvic_interrupts();
void adc12_disable_nvic_interrupts();
void adc12_set_int_handler(void (*func)());

void adc3_set_int_handler(void (*func)());
void adc3_enable_nvic_interrupts();
void adc3_disable_nvic_interrupts();



#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif  //_STM32_ADC_H_
