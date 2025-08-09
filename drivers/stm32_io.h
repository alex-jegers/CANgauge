/*
 * stm32_io.h
 *
 *  Created on: Feb 17, 2024
 *      Author: awjpp
 */

#ifndef INC_STM32_IO_H_
#define INC_STM32_IO_H_

#include "stm32h745xx.h"
#include <stdbool.h>


typedef enum
{
	IO_OUTPUT_TYPE_PUSH_PULL,
	IO_OUTPUT_TYPE_OPEN_DRAIN,
}io_output_type_t;

typedef enum
{
	GPIO_OSPEEDR_LOW        = 0x0,
	GPIO_OSPEEDR_MED        = 0x1,
	GPIO_OSPEEDR_HIGH       = 0x2,
	GPIO_OSPEEDR_VERY_HIGH  = 0x3,
	GPIO_OSPEEDR_Msk        = 0x3,	
}io_output_speed_t;


#define GPIO_MODER_INPUT		0x0
#define GPIO_MODER_OUTPUT		0x1
#define GPIO_MODER_ALT_FUNC		0x2
#define GPIO_MODER_ANALOG		0x3
#define GPIO_MODER_MASK			0x3

#define GPIO_AFR_AF0            0
#define GPIO_AFR_AF1            1
#define GPIO_AFR_AF2            2
#define GPIO_AFR_AF3            3
#define GPIO_AFR_AF4            4
#define GPIO_AFR_AF5            5
#define GPIO_AFR_AF6            6
#define GPIO_AFR_AF7            7
#define GPIO_AFR_AF8            8
#define GPIO_AFR_AF9            9
#define GPIO_AFR_AF10           10
#define GPIO_AFR_AF11           11
#define GPIO_AFR_AF12           12
#define GPIO_AFR_AF13           13
#define GPIO_AFR_AF14           14
#define GPIO_AFR_AF15           15

#define GPIO_PIN0_Msk          1 << 0
#define GPIO_PIN1_Msk          1 << 1
#define GPIO_PIN2_Msk          1 << 2
#define GPIO_PIN3_Msk          1 << 3
#define GPIO_PIN4_Msk          1 << 4
#define GPIO_PIN5_Msk          1 << 5
#define GPIO_PIN6_Msk          1 << 6
#define GPIO_PIN7_Msk          1 << 7
#define GPIO_PIN8_Msk          1 << 8
#define GPIO_PIN9_Msk          1 << 9
#define GPIO_PIN10_Msk         1 << 10
#define GPIO_PIN11_Msk         1 << 11
#define GPIO_PIN12_Msk         1 << 12
#define GPIO_PIN13_Msk         1 << 13
#define GPIO_PIN14_Msk         1 << 14
#define GPIO_PIN15_Msk         1 << 15




/*Routes the main clock to GPIOA Pin 8.*/
void io_route_clock_to_pin();

/*Enables/disables IO clock.*/
void io_init();
void io_deinit();

/*Set pin direction.*/
void io_set_pin_dir_out(GPIO_TypeDef* port, uint32_t mask);
void io_set_pin_dir_in(GPIO_TypeDef* port, uint32_t mask);

/*Set pin output value.*/
void io_pin_out_set(GPIO_TypeDef* port, uint32_t mask);
void io_pin_out_clr(GPIO_TypeDef* port, uint32_t mask);
void io_pin_out_tgl(GPIO_TypeDef* port, uint32_t mask);

/*Other IO functions.*/
void io_set_pin_mux(GPIO_TypeDef* port, uint32_t mask, uint8_t gpio_afr_af);
void io_clr_pin_mux(GPIO_TypeDef* port, uint32_t mask);
void io_set_output_speed(GPIO_TypeDef* port, uint32_t mask, io_output_speed_t gpio_ospeedr);
void io_set_output_type(GPIO_TypeDef* port, uint32_t mask, io_output_type_t io_output_type);
uint32_t io_read_port(GPIO_TypeDef* port);
bool io_read_pin(GPIO_TypeDef* port, uint32_t pin_mask);
void io_init_test_led(GPIO_TypeDef* port, uint32_t mask);
void io_test_led_on();
void io_test_led_off();
void io_test_led_blink(uint32_t ms);


#endif /* INC_STM32_IO_H_ */
