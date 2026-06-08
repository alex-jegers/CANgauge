/*
 * stm32_timer.c
 *
 *  Created on: Feb 28, 2024
 *      Author: awjpp
 */

#include "stm32_sys_timer.h"

static volatile uint32_t timer_time_us = 0;
static volatile uint32_t timer_time_ms = 0;
static volatile uint32_t timer_time_s = 0;
static volatile uint32_t timer_time_m = 0;

void timer_ms_inc(uint32_t time_in_ms)
{
	timer_time_ms++;
}

uint32_t timer_get_time_ms()
{
	return timer_time_ms;
}

void timer_delay_ms(uint32_t time_ms)
{
	uint32_t current_time = timer_get_time_ms();

	while ((timer_time_ms - current_time) < time_ms) {}

}

void timer_us_inc(uint32_t time_in_us)
{
	timer_time_us++;
}

uint32_t timer_get_time_us()
{
	return timer_time_us;
}

void timer_delay_us(uint32_t time_us)
{
	uint32_t current_time = timer_get_time_us();

	while ((timer_time_us - current_time) < time_us) {}
}

