/*
 * stm32_iic.h
 *
 *  Created on: Apr 14, 2024
 *      Author: awjpp
 */

#ifndef DRIVERS_STM32_IIC_H_
#define DRIVERS_STM32_IIC_H_


#include "stm32h745xx.h"
#include "stdbool.h"
#include "stddef.h"


typedef enum 
{
	I2C_CLK_100K,
	I2C_CLK_400K,
	I2C_CLK_1M,
}i2c_clk_speed_t;


void i2c_init_clk(I2C_TypeDef* i2c);
void i2c_set_clk_speed(I2C_TypeDef* i2c, i2c_clk_speed_t clk_frq);
void i2c_enable(I2C_TypeDef* i2c);
void i2c_disable_analog_filt(I2C_TypeDef* i2c);
void i2c_disable_clk_stretch(I2C_TypeDef* i2c);
int8_t i2c_read(I2C_TypeDef* i2c, uint8_t slave_addr, uint8_t internal_addr, uint8_t* data, uint8_t num_bytes);
int8_t i2c_write(I2C_TypeDef* i2c, uint8_t slave_addr, uint8_t internal_addr, uint8_t* data, uint8_t num_bytes, bool auto_stop);
int8_t i2c_probe(I2C_TypeDef* i2c);		//Returns the address of a devices on the bus that acks.

uint32_t i2c_status(I2C_TypeDef* i2c);
void i2c_clear_status(I2C_TypeDef* i2c);

#endif /* DRIVERS_STM32_IIC_H_ */
