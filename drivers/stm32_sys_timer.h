/*
 * stm32_timer.h
 *
 *  Created on: Feb 28, 2024
 *      Author: awjpp
 */

#ifndef DRIVERS_STM32_TIMER_H_
#define DRIVERS_STM32_TIMER_H_

#include "stm32h745xx.h"

void timer_inc(uint32_t time_in_ms);
uint32_t timer_get_time_ms();
void timer_delay_ms(uint32_t time_ms);

#endif /* DRIVERS_STM32_TIMER_H_ */
