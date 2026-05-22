/*
 * stm32_iic.c
 *
 *  Created on: Apr 14, 2024
 *      Author: awjpp
 */


#include "stm32_iic.h"

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"

#include "drivers/drivers.h"



#define RCC_D2CCIP2R_I2C123SEL_PCLK1					0x0 << RCC_D2CCIP2R_I2C123SEL_Pos
#define RCC_D2CCIP2R_I2C123SEL_PLLR3					0x1 << RCC_D2CCIP2R_I2C123SEL_Pos
#define RCC_D2CCIP2R_I2C123SEL_HSI						0x2 << RCC_D2CCIP2R_I2C123SEL_Pos
#define RCC_D2CCIP2R_I2C123SEL_CSI						0x3 << RCC_D2CCIP2R_I2C123SEL_Pos

#define RCC_D3CCIPR_I2C4SEL_PCLK1						0x0 << RCC_D3CCIPR_I2C4SEL_Pos
#define RCC_D3CCIPR_I2C4SEL_PLLR3						0x1 << RCC_D3CCIPR_I2C4SEL_Pos
#define RCC_D3CCIPR_I2C4SEL_HSI							0x2 << RCC_D3CCIPR_I2C4SEL_Pos
#define RCC_D3CCIPR_I2C4SEL_CSI							0x3 << RCC_D3CCIPR_I2C4SEL_Pos


static bool prv_timeout = false;
static TimerHandle_t prv_timer_timeout = NULL;
static SemaphoreHandle_t prv_i2c_mutex = NULL;

static uint8_t i2c_get_data(I2C_TypeDef* i2c);
static void i2c_write_data(I2C_TypeDef* i2c, uint8_t data);
static void prv_timer_cb_timeout(TimerHandle_t* timer);				//Callback for FreeRTOS timer.
static int8_t prv_start_timer();
static void prv_clear_timer();

static uint8_t i2c_get_data(I2C_TypeDef* i2c)
{
	return i2c->RXDR;
}

static void i2c_write_data(I2C_TypeDef* i2c, uint8_t data)
{
	i2c->TXDR = data;
}

static void prv_timer_cb_timeout(TimerHandle_t* timer)
{
	prv_timeout = true;
	i2c_bus_reset(I2C4);
	//prv_clear_timer();
}

static int8_t prv_start_timer()
{
	if (prv_timer_timeout == NULL)
	{
		return -1;
	}

	if (xTimerStart(prv_timer_timeout, 0) == pdFAIL)
	{
		return -1;
	}

	return 0;
}

static void prv_clear_timer()
{
	if (prv_timer_timeout == NULL)
	{
		return;
	}
	else
	{
		xTimerReset(prv_timer_timeout, 0);
		xTimerStop(prv_timer_timeout, 0);
	}
}

void i2c_init(I2C_TypeDef* i2c)
{
	/*I2C4 bus and kernel clock enable and selection.*/
	if (i2c == I2C4)
	{
		if (prv_i2c_mutex == NULL)
		{
			prv_i2c_mutex = xSemaphoreCreateRecursiveMutex();
		}
		assert( prv_i2c_mutex != NULL );

		RCC->APB4ENR |= RCC_APB4ENR_I2C4EN;					//enable APB clock.
		RCC->D3CCIPR &= ~(0x3 << RCC_D3CCIPR_I2C4SEL_Pos);	//clear the bits.
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
		i2c->TIMINGR = (0 << I2C_TIMINGR_PRESC_Pos)
						| (37 << I2C_TIMINGR_SCLH_Pos)
						| (37 << I2C_TIMINGR_SCLL_Pos)
						| (0 << I2C_TIMINGR_SDADEL_Pos)
						| (0 << I2C_TIMINGR_SCLDEL_Pos);
		break;
	
	case I2C_CLK_400K:
		i2c->TIMINGR = (0 << I2C_TIMINGR_PRESC_Pos)
						| (6 << I2C_TIMINGR_SCLH_Pos)
						| (6 << I2C_TIMINGR_SCLL_Pos)
						| (0 << I2C_TIMINGR_SDADEL_Pos)
						| (0 << I2C_TIMINGR_SCLDEL_Pos);
		break;

	case I2C_CLK_1M:
		i2c->TIMINGR = (0 << I2C_TIMINGR_PRESC_Pos) |
						(1 << I2C_TIMINGR_SCLH_Pos) |
						(1 << I2C_TIMINGR_SCLL_Pos);
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

void i2c_enable_timeout_detection(I2C_TypeDef* i2c)
{
	prv_timer_timeout = xTimerCreate("I2C_TIMER", pdMS_TO_TICKS(100), pdFALSE, NULL, prv_timer_cb_timeout);
}

i2c_exit_code_t i2c_read(I2C_TypeDef* i2c, uint8_t slave_addr, uint16_t internal_addr, i2c_internal_addr_t internal_addr_type,
				uint8_t* data, uint8_t num_bytes, bool auto_stop)
{
	if (xSemaphoreTakeRecursive(prv_i2c_mutex, portMAX_DELAY) == pdFALSE)
	{
		return I2C_EXIT_CODE_ERR;
	}

	prv_start_timer();

	while ((i2c_status(I2C4) & I2C_ISR_BUSY) != 0)
	{
		if (prv_timeout == true)
		{
			i2c_bus_reset(i2c);
			xSemaphoreGiveRecursive(prv_i2c_mutex);
			return I2C_EXIT_CODE_TIMEOUT;
		}
		if (i2c_status(i2c) & I2C_ISR_ARLO)
		{
			i2c_bus_reset(i2c);
			xSemaphoreGiveRecursive(prv_i2c_mutex);
			return I2C_EXIT_CODE_ARB_LOST;
		}
	}

	/* If setting the internal register pointer failed, return an error. */
	i2c_exit_code_t wr_sts = i2c_write(i2c, slave_addr, internal_addr, internal_addr_type, NULL, 0, auto_stop);

	if (wr_sts != I2C_EXIT_CODE_TC)
	{
		xSemaphoreGiveRecursive(prv_i2c_mutex);
		return wr_sts;
	}
	prv_start_timer();

	i2c->CR2 = slave_addr;						//set slave address and clear the rest of the register.
	i2c->CR2 |= I2C_CR2_RD_WRN;						//set bit for requesting read.
	i2c->CR2 |= num_bytes << I2C_CR2_NBYTES_Pos;	//set the number of bytes.
	if (auto_stop == true)
	{
		//i2c->CR2 |= I2C_CR2_AUTOEND;					//enable auto stop.
	}
	i2c_clear_status(i2c);

	i2c->CR2 |= I2C_CR2_START;						//start the transmission.

	uint8_t x = 0;
	int8_t rtn = 0;
	while (1)
	{
		if (i2c_status(i2c) & I2C_ISR_RXNE)
		{
			*data = i2c_get_data(i2c);
			data++;
			x++;
		}
		if (i2c_status(i2c) & I2C_ISR_NACKF)
		{
			rtn = I2C_EXIT_CODE_NACK;
			break;
		}
		if (i2c_status(i2c) & I2C_ISR_STOPF)
		{
			if (x == num_bytes)
			{
				rtn = I2C_EXIT_CODE_STOP;
				break;
			}
			rtn = I2C_EXIT_CODE_ERR;
			break;
		}
		if (i2c_status(i2c) & I2C_ISR_TIMEOUT)
		{
			rtn = I2C_EXIT_CODE_TIMEOUT;
			break;
		}
		if (i2c_status(i2c) & I2C_ISR_ARLO)
		{
			rtn = I2C_EXIT_CODE_ARB_LOST;
			break;
		}
		if (i2c_status(i2c) & I2C_ISR_TC)
		{
			i2c->CR2 |= I2C_CR2_STOP;
			rtn = I2C_EXIT_CODE_TC;
			break;
		}

		if (prv_timeout)
		{
			rtn = -1;
			break;
		}

	}

	prv_clear_timer();

	xSemaphoreGiveRecursive(prv_i2c_mutex);

	return rtn;
}
/*Returns zero for success, non-zero for a failure.*/
i2c_exit_code_t i2c_write(I2C_TypeDef* i2c, uint8_t slave_addr, uint16_t internal_addr, i2c_internal_addr_t internal_addr_type,
				uint8_t* data, uint8_t num_bytes, bool auto_stop)
{
	if (xSemaphoreTakeRecursive(prv_i2c_mutex, portMAX_DELAY) == pdFALSE)
	{
		return I2C_EXIT_CODE_ERR;
	}

	prv_start_timer();

	while ((i2c_status(I2C4) & I2C_ISR_BUSY) != 0)
	{
		if (prv_timeout == true)
		{
			xSemaphoreGiveRecursive(prv_i2c_mutex);
			return I2C_EXIT_CODE_TIMEOUT;
		}
	}

	i2c_clear_status(i2c);
	i2c->ISR |= I2C_ISR_TXE;						//Flush the TXDR register.

	if (internal_addr_type == I2C_INTERNAL_ADDR_8_BIT)
	{
		i2c->TXDR = internal_addr;						//send the internal address first.

		/* If there are no data bytes and were only setting the internal address this will be the only thing we send
		 * so check if we need to stop after it as well.
		 */
		if ((num_bytes == 0) && (auto_stop == true))
		{
			//i2c->CR2 |= I2C_CR2_STOP;
		}
		num_bytes++;
	}
	else if (internal_addr_type == I2C_INTERNAL_ADDR_16_BIT)
	{
		i2c->TXDR = ((internal_addr & 0xFF00) >> 8);
		num_bytes += 2;								//Add one byte to send for the second half of the internal address.
	}

	i2c->CR2 = slave_addr;							//set slave address and clear the rest of the register.
	i2c->CR2 |= num_bytes << I2C_CR2_NBYTES_Pos;	//set the number of bytes.

	if (auto_stop == true)
	{
		//i2c->CR2 |= auto_stop << I2C_CR2_AUTOEND_Pos;	//set the auto end bit if needed.
	}

	i2c->CR2 |= I2C_CR2_START;						//start the transmission.

	uint8_t bytes_transferred = 0;
	i2c_exit_code_t rtn = 0;
	while(1)
	{

		if (i2c_status(i2c) & I2C_ISR_TXE)
		{
			if (bytes_transferred + 1 == num_bytes)		//If this next byte is the last one set the stop bit to send a stop signal after this bit.
			{
				if (auto_stop == true)
				{
					i2c->CR2 |= I2C_CR2_STOP;
				}
			}
			/* Check if we need to transfer another internal address byte. */
			if ((internal_addr_type == I2C_INTERNAL_ADDR_16_BIT) && (bytes_transferred == 0))
			{
				i2c_write_data(i2c, (internal_addr & 0x00FF));			//Write the other half of the internal address.
			}
			/* Otherwise start sending data. */
			else
			{
				i2c_write_data(i2c, *data);
				data++;
			}
			bytes_transferred++;
		}


		if (i2c_status(i2c) & I2C_ISR_NACKF)
		{
			rtn = I2C_EXIT_CODE_NACK;
			break;
		}

		if (i2c_status(i2c) & I2C_ISR_TC)
		{
			rtn = I2C_EXIT_CODE_TC;
			break;
		}

		if (i2c_status(i2c) & I2C_ISR_STOPF)
		{
			if (bytes_transferred == num_bytes)
			{
				rtn = I2C_EXIT_CODE_TC;
			}
			else
			{
				rtn = I2C_EXIT_CODE_ERR;
			}
			break;
		}

		if (i2c_status(i2c) & I2C_ISR_ARLO)
		{
			rtn = I2C_EXIT_CODE_ARB_LOST;
			break;
		}

		if (i2c_status(i2c) & I2C_ISR_TIMEOUT)
		{
			rtn = I2C_EXIT_CODE_TIMEOUT;
			break;
		}

		if (prv_timeout)
		{
			rtn = I2C_EXIT_CODE_TIMEOUT;
			break;
		}
	}

	prv_clear_timer();

	xSemaphoreGiveRecursive(prv_i2c_mutex);

	return rtn;
}

void i2c_bus_reset(I2C_TypeDef* i2c)
{
	io_set_output_type(GPIOD, GPIO_PIN12_Msk, IO_OUTPUT_TYPE_OPEN_DRAIN);
	io_set_output_type(GPIOD, GPIO_PIN13_Msk, IO_OUTPUT_TYPE_OPEN_DRAIN);
	io_set_pin_dir_out(GPIOD, GPIO_PIN12_Msk);

	for (uint8_t i = 0; i < 10; i++)
	{
		io_pin_out_set(GPIOD, GPIO_PIN12_Msk);
		timer_delay_ms(1);
		io_pin_out_clr(GPIOD, GPIO_PIN12_Msk);
		timer_delay_ms(1);
	}

	io_set_pin_mux(GPIOD, GPIO_PIN12_Msk, GPIO_AFR_AF4);
	io_set_pin_mux(GPIOD, GPIO_PIN13_Msk, GPIO_AFR_AF4);
}

int8_t i2c_probe(I2C_TypeDef* i2c)
{
	for (int8_t addr = 0; addr < 128; addr++)
	{
		int8_t rtn = i2c_write(i2c, addr, 0x00, I2C_INTERNAL_ADDR_8_BIT, NULL, 0, false);
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
	prv_timeout = false;
	i2c->ICR = 0xFFFFFFFF;
}
