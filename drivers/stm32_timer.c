/*
 * stm32_timer12.c
 *
 *  Created on: Apr 28, 2024
 *      Author: awjpp
 */


#include "drivers/stm32_timer.h"
#include "drivers/stm32_rcc.h"



#define TIM_CCMR1_OC1M_PWM_MODE1		0x6 << 4
#define TIM_CCMR1_OC2M_PWM_MODE1		0x6 << 12


void timer_init(TIM_TypeDef* timer)
{
	if (timer == TIM12)
	{
		RCC->APB1LENR |= RCC_APB1LENR_TIM12EN;
	}
}

void timer_enable(TIM_TypeDef* timer)
{
	timer->CR1 |= TIM_CR1_CEN;
}

void timer_disable(TIM_TypeDef* timer)
{
	timer->CR1 &= ~(TIM_CR1_CEN);
}

void timer_enable_pwm_output(TIM_TypeDef* timer, uint32_t channel)
{
	if (timer == TIM12)
	{
		timer->CR1 |= TIM_CR1_ARPE;							//enable auto-reload register.
		if (channel == 1)
		{
			timer->CCMR1 &= ~(TIM_CCMR1_CC1S_Msk);			//put into output mode.
			timer->CCMR1 |= TIM_CCMR1_OC1M_PWM_MODE1;		//put into pwm 1 mode.
			timer->CCMR1 |= TIM_CCMR1_OC1PE;				//enable the corresponding pre-load register.
			timer->CCER |= TIM_CCER_CC1E;					//enable the output signal.
		}

		/*Same steps as channel 1 only for the respective #2 timer bits.*/
		if (channel == 2)
		{
			timer->CCMR1 &= ~(TIM_CCMR1_CC2S_Msk);
			timer->CCMR1 |= TIM_CCMR1_OC2M_PWM_MODE1;
			timer->CCMR1 |= TIM_CCMR1_OC2PE;
			timer->CCER |= TIM_CCER_CC2E;
		}
	}
}

uint32_t timer_set_pwm_freq(TIM_TypeDef* timer, uint32_t freq_in_hz)
{
	/*check to make sure it's in the bounds of what's possible.*/
	/*less than 1Hz is possible but mathematically easier to rule it out for now.*/
	if (freq_in_hz < 1)
	{
		return 0;
	}
	if (freq_in_hz > (RCC_F_TIMERS_CLK / 2))
	{
		return 0;
	}

	uint32_t auto_reload = 0;
	uint32_t prescaler = 0;
	uint32_t output_frequency = 0;

	if (freq_in_hz > 3662 /*TODO: Replace with non-hard coded value.*/)
	{
		prescaler = 0;
		auto_reload = RCC_F_TIMERS_CLK / ((prescaler + 1) * freq_in_hz);
	}
	else
	{
		prescaler = 3662;
		auto_reload = RCC_F_TIMERS_CLK / ((prescaler + 1) * freq_in_hz);
	}

	timer->PSC = (uint16_t)prescaler;
	timer->ARR = (uint16_t)auto_reload;

	output_frequency = RCC_F_TIMERS_CLK / ((prescaler + 1) * auto_reload);
	return output_frequency;
}

void timer_set_pwm_duty_cycle(TIM_TypeDef* timer, uint32_t duty_cycle, uint32_t channel)
{
	uint16_t auto_reload = timer->ARR;
	if (channel == 1)
	{
		timer->CCR1 = (duty_cycle * auto_reload) / 65535;
	}
	if (channel == 2)
	{
		timer->CCR2 = (duty_cycle * auto_reload) / 65535;
	}
}


