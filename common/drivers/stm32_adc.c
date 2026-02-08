/**********     INCLUDES        **********/
#include "stm32_adc.h"

/**********		DEFINES		**********/

/**********		EXTERNAL VARIABLE DEFINITIONS		**********/

/**********		STATIC VARIABLES		**********/
void (*adc12_interrupt_cb)();
void (*adc3_interrupt_cb)();

/**********		STATIC FUNCTION DECLRATIONS		**********/

/**********		STATIC FUNCTION DEFINITIONS		**********/

/**********		GLOBAL FUNCTION DEFINITIONS		**********/
void adc_init_clk(ADC_TypeDef* adc, adc_ck_src_t ck_src)
{
	/*Set the kernel clock selection.*/
	RCC->D3CCIPR &= ~(RCC_D3CCIPR_ADCSEL_Msk);	//Clear both bits.
	RCC->D3CCIPR |= ck_src << RCC_D3CCIPR_ADCSEL_Pos;
	RCC->AHB1ENR |= RCC_AHB1ENR_ADC12EN;		//Enable the bus clock.
}

void adc_enable(ADC_TypeDef* adc)
{
	adc->CR &= ~(ADC_CR_DEEPPWD);
	adc->CR |= ADC_CR_ADVREGEN;
	while ((adc->ISR & ADC_ISR_LDORDY) == 0) {}
	adc->CR |= ADC_CR_ADEN;
}

void adc_disable(ADC_TypeDef* adc)
{
	adc->CR &= ~(ADC_CR_ADDIS);
}

void adc_start_conversion(ADC_TypeDef* adc)
{
	adc->CR |= ADC_CR_ADSTART;
}			

void adc_stop_conversion(ADC_TypeDef* adc)
{
	adc->CR &= ~(ADC_CR_ADSTART);
}

void adc_set_continuous_mode(ADC_TypeDef* adc)
{
	adc->CFGR |= ADC_CFGR_CONT;
}

void adc_set_resolution(ADC_TypeDef* adc, adc_resolution_t res)
{
	adc->CFGR &= ~(0x7 << ADC_CFGR_RES_Pos);	//clear bits.
	adc->CFGR |= res << ADC_CFGR_RES_Pos;		//set bits.
}

void adc_enable_watchdog_1(ADC_TypeDef* adc)
{
	adc->CFGR |= ADC_CFGR_AWD1EN;
}

void adc_select_watchdog_1_channel(ADC_TypeDef* adc, uint8_t channel)
{
	adc->CFGR &= ~(0x1F << ADC_CFGR_AWD1CH_Pos); 	//clear the bits.
	adc->CFGR |= (channel << ADC_CFGR_AWD1CH_Pos);	//set the bits.
}

void adc_set_watchdog_1_low_threshold(ADC_TypeDef* adc, uint32_t threshold)
{
	adc->LTR1 = 0;

	adc_resolution_t current_res = adc->CFGR & (0x7 << ADC_CFGR_RES_Pos);
	switch (current_res)
	{
	case ADC_RESOLUTION_16_BIT:
		adc->LTR1 = threshold;
		break;
	case ADC_RESOLUTION_14_BIT:
		adc->LTR1 = threshold << 2;
		break;
	case ADC_RESOLUTION_12_BIT:
		adc->LTR1 = threshold << 4;
		break;
	case ADC_RESOLUTION_10_BIT:
		adc->LTR1 = threshold << 6;
		break;
	case ADC_RESOLUTION_8_BIT:
		adc->LTR1 = threshold << 8;
		break;
	default:
		break;
	}
}

void adc_set_watchdog_1_high_threshold(ADC_TypeDef* adc, uint32_t threshold)
{
	adc->HTR1 = 0;

	adc_resolution_t current_res = adc->CFGR & (0x7 << ADC_CFGR_RES_Pos);
	switch (current_res)
	{
	case ADC_RESOLUTION_16_BIT:
		adc->HTR1 = threshold;
		break;
	case ADC_RESOLUTION_14_BIT:
		adc->HTR1 = threshold << 2;
		break;
	case ADC_RESOLUTION_12_BIT:
		adc->HTR1 = threshold << 4;
		break;
	case ADC_RESOLUTION_10_BIT:
		adc->HTR1 = threshold << 6;
		break;
	case ADC_RESOLUTION_8_BIT:
		adc->HTR1 = threshold << 8;
		break;
	default:
		break;
	}
}

void adc_set_sample_time(ADC_TypeDef* adc, adc_sample_time_t sample_time, uint8_t channel)
{
	if (channel > 9)
	{
		channel = channel - 10;
		adc->SMPR2 &= ~(0x7 << (channel * 3));			//clear the bits.
		adc->SMPR2 |= sample_time << (channel * 3);		//set the bits.
	}
	else
	{
		adc->SMPR1 &= ~(0x7 << (channel * 3));			//clear the bits.
		adc->SMPR1 |= sample_time << (channel * 3);		//set the bits.
	}
}

void adc_set_channel(ADC_TypeDef* adc, uint8_t channel)
{
	adc->SQR1 = channel << 6;
	adc->PCSEL = (1 << channel);
}

uint32_t adc_get_conversion(ADC_TypeDef* adc)
{
	return adc->DR;
}

uint32_t adc_get_interrupt(ADC_TypeDef* adc, adc_int_flag_t interrupt)
{
	return adc->ISR & (uint32_t)interrupt;
}
void adc_clear_interrupt(ADC_TypeDef* adc, adc_int_flag_t interrupt)
{
	adc->ISR = (uint32_t)interrupt;
}
void adc_enable_interrupt(ADC_TypeDef* adc, adc_int_flag_t interrupt)
{
	adc->IER |= interrupt;
}
void adc_disable_interrupt(ADC_TypeDef* adc, adc_int_flag_t interrupt)
{
	adc->IER &= ~(interrupt);
}

void adc12_set_clock_prescaler(adc_prescaler_t prescaler)
{
	ADC12_COMMON->CCR &= ~(ADC_CCR_PRESC_Msk);				//bit clear.
	ADC12_COMMON->CCR |= prescaler << ADC_CCR_PRESC_Pos;	//bit set.
}

void adc12_enable_nvic_interrupts()
{
	NVIC_EnableIRQ(ADC_IRQn);
}

void adc12_disable_nvic_interrupts()
{
	NVIC_DisableIRQ(ADC_IRQn);
}

void adc12_set_int_handler(void (*func)())
{
	adc12_interrupt_cb = func;
}

void adc3_set_int_handler(void (*func)())
{
	adc3_interrupt_cb = func;
}

void adc3_enable_nvic_interrupts()
{
	NVIC_EnableIRQ(ADC3_IRQn);
}

void adc3_disable_nvic_interrupts()
{
	NVIC_DisableIRQ(ADC3_IRQn);
}

void ADC1_2_IRQHandler()
{
	if (adc12_interrupt_cb != NULL)
	{
		adc12_interrupt_cb();
	}
}

void ADC3_IRQHandler()
{
	if (adc3_interrupt_cb != NULL)
	{
		adc3_interrupt_cb();
	}
}
