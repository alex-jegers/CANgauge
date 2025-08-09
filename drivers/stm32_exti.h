/*
 * stm32_exti.h
 *
 *  Created on: Feb 25, 2024
 *      Author: awjpp
 */

#ifndef INC_STM32_exti_H_
#define INC_STM32_exti_H_

#include "stm32h745xx.h"

#define EXTI_EVENT_IO_PIN0                1 << 0
#define EXTI_EVENT_IO_PIN1                1 << 1
#define EXTI_EVENT_IO_PIN2                1 << 2
#define EXTI_EVENT_IO_PIN3                1 << 3
#define EXTI_EVENT_IO_PIN4                1 << 4
#define EXTI_EVENT_IO_PIN5                1 << 5
#define EXTI_EVENT_IO_PIN6                1 << 6
#define EXTI_EVENT_IO_PIN7                1 << 7
#define EXTI_EVENT_IO_PIN8                1 << 8
#define EXTI_EVENT_IO_PIN9                1 << 9
#define EXTI_EVENT_IO_PIN10               1 << 10
#define EXTI_EVENT_IO_PIN11               1 << 11
#define EXTI_EVENT_IO_PIN12               1 << 12
#define EXTI_EVENT_IO_PIN13               1 << 13
#define EXTI_EVENT_IO_PIN14               1 << 14
#define EXTI_EVENT_IO_PIN15               1 << 15

typedef enum
{
    RISING,
    FALLING,
    BOTH
}exti_edge_t;

void exti_enable_io_interrupt(uint32_t exti_event_x, exti_edge_t edge);
uint32_t exti_io_intflags();
void exti_clear_interrupt(uint32_t exti_event_x);

void exti0_assign_int_handler(void (*func)());
void exti1_assign_int_handler(void (*func)());
void exti2_assign_int_handler(void (*func)());
void exti3_assign_int_handler(void (*func)());
void exti4_assign_int_handler(void (*func)());
void exti5_9_assign_int_handler(void (*func)());
void exti10_15_assign_int_handler(void (*func)());

#endif /* INC_STM32_exti_H_ */
