/*
 * stm32_timer.h
 *
 *  Created on: Feb 28, 2024
 *      Author: awjpp
 */

#ifndef DRIVERS_STM32_TIMER_H_
#define DRIVERS_STM32_TIMER_H_

#include "stm32h745xx.h"

void timer_ms_inc(uint32_t time_in_ms);
uint32_t timer_get_time_ms();
void timer_delay_ms(uint32_t time_ms);

void timer_us_inc(uint32_t time_in_us);
uint32_t timer_get_time_us();
void timer_delay_us(uint32_t time_us);

#endif /* DRIVERS_STM32_TIMER_H_ */
