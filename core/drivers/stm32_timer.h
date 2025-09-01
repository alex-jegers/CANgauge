/*
 * stm32_timer.h
 *
 *  Created on: Apr 28, 2024
 *      Author: awjpp
 */

#ifndef DRIVERS_TIMERS_STM32_TIMER_H_
#define DRIVERS_TIMERS_STM32_TIMER_H_

#include "stm32h745xx.h"



void timer_init(TIM_TypeDef* timer);
void timer_enable(TIM_TypeDef* timer);
void timer_disable(TIM_TypeDef* timer);
void timer_enable_pwm_output(TIM_TypeDef* timer, uint32_t channel);
void timer_disable_pwm_output(TIM_TypeDef* timer, uint32_t channel);
uint32_t timer_set_pwm_freq(TIM_TypeDef* timer, uint32_t freq_in_hz);
void timer_set_pwm_duty_cycle(TIM_TypeDef* timer, uint32_t duty_cycle, uint32_t channel);


#endif /* DRIVERS_TIMERS_STM32_TIMER12_H_ */
