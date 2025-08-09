/*
 * stm32_io.c
 *
 *  Created on: Feb 17, 2024
 *      Author: awjpp
 */


#include "stm32_io.h"
#include "stm32_rcc.h"	//for io_route_clock_to_pin().
#include "stddef.h"		//for NULL.

/**********		STATIC VARIABLES		**********/
static GPIO_TypeDef* test_led_port = NULL;
static uint32_t test_led_mask = 0;

/*Maps PLLQ1 to GPIOA Pin 8.*/
void io_route_clock_to_pin()
{
	rcc_enable_pll1q();
	RCC->CFGR |= (0x3 << RCC_CFGR_MCO1_Pos) | (15 << RCC_CFGR_MCO1PRE_Pos);
	io_set_pin_mux(GPIOA, GPIO_PIN8_Msk, GPIO_AFR_AF0);
}

void io_init()
{
	RCC->AHB4ENR |= RCC_AHB4ENR_GPIOAEN
				| RCC_AHB4ENR_GPIOBEN
				| RCC_AHB4ENR_GPIOCEN
				| RCC_AHB4ENR_GPIODEN
				| RCC_AHB4ENR_GPIOEEN
				| RCC_AHB4ENR_GPIOFEN
				| RCC_AHB4ENR_GPIOGEN
				| RCC_AHB4ENR_GPIOHEN
				| RCC_AHB4ENR_GPIOIEN
				| RCC_AHB4ENR_GPIOJEN
				| RCC_AHB4ENR_GPIOKEN;
}

void io_deinit()
{
	RCC->AHB4ENR &= ~(RCC_AHB4ENR_GPIOAEN
				| RCC_AHB4ENR_GPIOBEN
				| RCC_AHB4ENR_GPIOCEN
				| RCC_AHB4ENR_GPIODEN
				| RCC_AHB4ENR_GPIOEEN
				| RCC_AHB4ENR_GPIOFEN
				| RCC_AHB4ENR_GPIOGEN
				| RCC_AHB4ENR_GPIOHEN
				| RCC_AHB4ENR_GPIOIEN
				| RCC_AHB4ENR_GPIOJEN
				| RCC_AHB4ENR_GPIOKEN);
}

/*Set pin direction.*/
void io_set_pin_dir_out(GPIO_TypeDef* port, uint32_t mask)
{
	for (uint8_t i = 0; i < 16; i++)
	{
		uint32_t x = 1 << i;
		if (x & mask)
		{
			uint32_t bit_pos = i * 2;
			port->MODER &= ~(GPIO_MODER_MASK << bit_pos);		//Clear the bits.
			port->MODER |= GPIO_MODER_OUTPUT << bit_pos;					//Write the bits.
		}
	}
}

void io_set_pin_dir_in(GPIO_TypeDef* port, uint32_t mask)
{
	for (uint8_t i = 0; i < 16; i++)
	{
		uint32_t x = 1 << i;
		if (x & mask)
		{
			uint32_t bit_pos = i * 2;
			port->MODER &= ~(GPIO_MODER_MASK << bit_pos);		//Clear the bits.
		}
	}
}

/*Set pin output value.*/
void io_pin_out_set(GPIO_TypeDef* port, uint32_t mask)
{
	port->ODR |= mask;
}
void io_pin_out_clr(GPIO_TypeDef* port, uint32_t mask)
{
	port->ODR &= ~(mask);
}
void io_pin_out_tgl(GPIO_TypeDef* port, uint32_t mask)
{
	port->ODR ^= mask;
}

void io_set_pin_mux(GPIO_TypeDef* port, uint32_t mask, uint8_t gpio_afr_af)
{
	for (uint8_t i = 0; i < 16; i++)
	{
		uint32_t test_mask = 1 << i;
		if (test_mask & mask)
		{
			uint32_t bit_pos_afr = i * 4;
			uint32_t bit_pos_moder = i * 2;

			port->MODER &= ~(GPIO_MODER_MASK << bit_pos_moder);
			port->MODER |= GPIO_MODER_ALT_FUNC << bit_pos_moder;

			if (i < 8)
			{
 				port->AFR[0] |= gpio_afr_af << bit_pos_afr;
			}
			else
			{
				port->AFR[1] |= gpio_afr_af << (bit_pos_afr - 32);
			}
		}
	}
}

void io_clr_pin_mux(GPIO_TypeDef* port, uint32_t mask)
{
	for (uint8_t i = 0; i < 16; i++)
	{
		uint32_t test_mask = 1 << i;
		if (test_mask & mask)
		{
			uint32_t bit_pos_moder = i * 2;

			port->MODER &= ~(GPIO_MODER_ALT_FUNC << bit_pos_moder);
		}
	}
}

void io_set_output_speed(GPIO_TypeDef* port, uint32_t mask, uint8_t gpio_ospeedr)
{
	for (uint8_t i = 0; i < 16; i++)
	{
		uint32_t x = 1 << i;
		if (x & mask)
		{
			uint32_t bit_pos = i * 2;
			port->OSPEEDR &= ~(GPIO_OSPEEDR_Msk << bit_pos);		//Clear the bits.
			port->OSPEEDR |= gpio_ospeedr << bit_pos;				//Write the bits.
		}
	}
}

void io_set_output_type(GPIO_TypeDef* port, uint32_t mask, io_output_type_t io_output_type)
{
	port->OTYPER &= ~(mask);			//clear the bit.

	/*If output type is open drain, set the bits, otherwise leave them cleared.*/
	if (io_output_type == IO_OUTPUT_TYPE_OPEN_DRAIN)
	{
		port->OTYPER |= mask;			//set the bits.
	}

}

uint32_t io_read_port(GPIO_TypeDef* port)
{
	return port->IDR;
}

bool io_read_pin(GPIO_TypeDef* port, uint32_t pin_mask)
{
	uint32_t pin_val = port->IDR & pin_mask;
	if (pin_val > 0)
	{
		return true;
	}
	return false;
}

void io_init_test_led(GPIO_TypeDef* port, uint32_t mask)
{
	test_led_port = port;
	test_led_mask = mask;
	io_set_pin_dir_out(test_led_port, test_led_mask);
}

void io_test_led_on()
{
	io_pin_out_set(test_led_port, test_led_mask);
}
void io_test_led_off()
{
	io_pin_out_clr(test_led_port, test_led_mask);
}

void io_test_led_blink(uint32_t ms)
{
	io_pin_out_tgl(test_led_port, test_led_mask);
	timer_delay_ms(ms);
	io_pin_out_tgl(test_led_port, test_led_mask);
}
