/*
 * stm32_iic.c
 *
 *  Created on: Apr 14, 2024
 *      Author: awjpp
 */


#include "stm32_iic.h"




#define RCC_D2CCIP2R_I2C123SEL_PCLK1					0x0
#define RCC_D2CCIP2R_I2C123SEL_PLLR3					0x1
#define RCC_D2CCIP2R_I2C123SEL_HSI						0x2
#define RCC_D2CCIP2R_I2C123SEL_CSI						0x3

#define RCC_D3CCIPR_I2C4SEL_PCLK1						0x0
#define RCC_D3CCIPR_I2C4SEL_PLLR3						0x1
#define RCC_D3CCIPR_I2C4SEL_HSI							0x2
#define RCC_D3CCIPR_I2C4SEL_CSI							0x3


static uint8_t i2c_get_data(I2C_TypeDef* i2c);
static void i2c_write_data(I2C_TypeDef* i2c, uint8_t data);

static uint8_t i2c_get_data(I2C_TypeDef* i2c)
{
	return i2c->RXDR;
}

static void i2c_write_data(I2C_TypeDef* i2c, uint8_t data)
{
	i2c->TXDR = data;
}



void i2c_init_clk(I2C_TypeDef* i2c)
{
	/*I2C4 bus and kernel clock enable and selection.*/
	if (i2c == I2C4)
	{
		RCC->APB4ENR |= RCC_APB4ENR_I2C4EN;					//enable APB clock.
		RCC->D3CCIPR &= (0x3 << RCC_D3CCIPR_I2C4SEL_Pos);	//clear the bits.
		RCC->D3CCIPR |= RCC_D3CCIPR_I2C4SEL_HSI;			//select the HSI as the kernel clock
		return;
	}

	/*I2C1,2,3 bus clock enable.*/
	if (i2c == I2C1)
	{
		RCC->APB1LENR |= RCC_APB1LENR_I2C1EN;					//enable APB clock.
	}	
	if (i2c == I2C2)
	{
		RCC->APB1LENR |= RCC_APB1LENR_I2C2EN;					//enable APB clock.
	}	
	if (i2c == I2C3)
	{
		RCC->APB1LENR |= RCC_APB1LENR_I2C3EN;					//enable APB clock.
	}		

	/*I2C1,2,3 kernel clock selection.*/
	RCC->D2CCIP2R &= (0x3 << RCC_D2CCIP2R_I2C123SEL_Pos);	//clear the bits.
	RCC->D2CCIP2R |= RCC_D2CCIP2R_I2C123SEL_HSI;			//select the HSI as the kernel clock
}


void i2c_set_clk_speed(I2C_TypeDef* i2c, i2c_clk_speed_t i2c_clk)
{
	switch (i2c_clk)
	{
	case I2C_CLK_100K:
		i2c->TIMINGR = (15 << I2C_TIMINGR_PRESC_Pos)
						| (37 << I2C_TIMINGR_SCLH_Pos)
						| (37 << I2C_TIMINGR_SCLL_Pos)
						| (4 << I2C_TIMINGR_SDADEL_Pos)
						| (4 << I2C_TIMINGR_SCLDEL_Pos);
		break;
	
	case I2C_CLK_400K:
		i2c->TIMINGR = (7 << I2C_TIMINGR_PRESC_Pos)
						| (16 << I2C_TIMINGR_SCLH_Pos)
						| (16 << I2C_TIMINGR_SCLL_Pos)
						| (2 << I2C_TIMINGR_SDADEL_Pos)
						| (2 << I2C_TIMINGR_SCLDEL_Pos);
		break;

	case I2C_CLK_1M:
		i2c->TIMINGR = (7 << I2C_TIMINGR_PRESC_Pos) |
						(7 << I2C_TIMINGR_SCLH_Pos) |
						(7 << I2C_TIMINGR_SCLL_Pos);
		break;
	default:
		break;
	}
}

void i2c_enable(I2C_TypeDef* i2c)
{
	i2c->CR1 |= I2C_CR1_PE;
}

void i2c_disable_analog_filt(I2C_TypeDef* i2c)
{
	i2c->CR1 |= I2C_CR1_ANFOFF;
}

void i2c_disable_clk_stretch(I2C_TypeDef* i2c)
{
	i2c->CR1 |= I2C_CR1_NOSTRETCH;
}

int8_t i2c_read(I2C_TypeDef* i2c, uint8_t slave_addr, uint8_t internal_addr, uint8_t* data, uint8_t num_bytes)
{
	if (i2c_write(i2c, slave_addr, internal_addr, NULL, 1, false) != 0)
	{
		i2c_clear_status(i2c);
		return -1;
	}

	/*Wait for write transfer to complete.*/
	while (!(i2c_status(i2c) & I2C_ISR_TC)){}

	i2c->CR2 = slave_addr;							//set slave address and clear the rest of the register.
	i2c->CR2 |= I2C_CR2_RD_WRN;						//set bit for requesting read.
	i2c->CR2 |= num_bytes << I2C_CR2_NBYTES_Pos;	//set the number of bytes.
	i2c->CR2 |= I2C_CR2_AUTOEND;					//enable auto stop.
	i2c->CR2 |= I2C_CR2_START;						//start the transmission.

	uint8_t x = 0;
	while (x < num_bytes)
	{
		if (i2c_status(i2c) & I2C_ISR_RXNE)
		{
			*data = i2c_get_data(i2c);
			data++;
			x++;
		}
		if (i2c_status(i2c) & I2C_ISR_NACKF)
		{
			i2c_clear_status(i2c);
			return -1;
		}

	}
	i2c_clear_status(i2c);
	return 0;
}

/*Returns zero for success, non-zero for a failure.*/
int8_t i2c_write(I2C_TypeDef* i2c, uint8_t slave_addr, uint8_t internal_addr, uint8_t* data, uint8_t num_bytes, bool auto_stop)
{
	i2c->CR2 = slave_addr;							//set slave address and clear the rest of the register.
	i2c->CR2 |= num_bytes << I2C_CR2_NBYTES_Pos;	//set the number of bytes.	
	i2c->TXDR = internal_addr;						//send the internal address first.
	i2c->CR2 |= auto_stop << I2C_CR2_AUTOEND_Pos;	//set the auto end bit if needed.
	i2c->CR2 |= I2C_CR2_START;						//start the transmission.

	uint8_t x = 1;
	while (x < num_bytes)
	{
		if (i2c_status(i2c) & I2C_ISR_TXE)
		{
			i2c_write_data(i2c, *data);
			data++;
			x++;
		}
		if (i2c_status(i2c) & I2C_ISR_NACKF)
		{
			i2c_clear_status(i2c);
			return -1;
		}
		if (i2c_status(i2c) & I2C_ISR_STOPF)
		{
			i2c_clear_status(i2c);
			return 0;
		}
	}
	i2c_clear_status(i2c);
	return 0;
}

int8_t i2c_probe(I2C_TypeDef* i2c)
{
	for (int8_t addr = 40; addr < 128; addr++)
	{
		int8_t rtn = i2c_write(i2c, addr, 0x00, NULL, 2, false);
		if (rtn == 0)
		{
			return addr;
		}
	}
	return -1;
}

uint32_t i2c_status(I2C_TypeDef* i2c)
{
	return i2c->ISR;
}

void i2c_clear_status(I2C_TypeDef* i2c)
{
	i2c->ICR = 0xFFFFFFFF;
}
