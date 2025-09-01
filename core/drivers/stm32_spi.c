
/***********	INCLUDES	************/
#include "stm32_spi.h"
#include "stm32_rcc.h"

/***********	TYPEDEFS	************/

/***********	DEFINES	************/

/***********	STATIC VARIABLES DEFINITIONS	************/

/***********	STATIC VARIABLES DECLARATIONS	************/

/***********	STATIC FUNCTION DECLARATIONS	************/

static void spi_set_bits_per_frame(SPI_TypeDef* spi, uint8_t num_bits);
static void spi_enable_master_mode(SPI_TypeDef* spi);
static void spi_enable_slave_mode(SPI_TypeDef* spi);
static void spi_start(SPI_TypeDef* spi);				//Sets the CSTART bit in CR1.
/***********	STATIC FUNCTION DEFINTIONS	************/
static void spi_set_bits_per_frame(SPI_TypeDef* spi, uint8_t num_bits)
{
	spi->CFG1 &= ~(0x1F << SPI_CFG1_DSIZE_Pos);			//Bit clear.
	spi->CFG1 |= (num_bits - 1) << SPI_CFG1_DSIZE_Pos;	//Set bits.
}

static void spi_enable_master_mode(SPI_TypeDef* spi)
{
	spi->CFG2 |= SPI_CFG2_MASTER;
}

static void spi_enable_slave_mode(SPI_TypeDef* spi)
{
	spi->CFG2 &= ~(SPI_CFG2_MASTER);
}

static void spi_start(SPI_TypeDef* spi)
{
	spi->CR1 |= SPI_CR1_CSTART;
}

/***********	GLOBAL FUNCTION DEFINTIONS	************/
void spi_init(SPI_TypeDef* spi)
{
	spi4_set_ss_high();
	spi->IFCR = SPI_IFCR_MODFC;
	spi->CFG2 |= (SPI_CFG2_COMM_HALF_DUPLEX << SPI_CFG2_COMM_Pos)
			| SPI_CFG2_AFCNTR										//SPI always has control of GPIO AF even when SPE = 0.
			| SPI_CFG2_SSOE;										//Enable SS output.
	spi->CFG1 |= (0x0 << SPI_CFG1_FTHLV_Pos);						//Not sure what this is doing exactly .
	spi->IER |= SPI_IER_EOTIE;										//Enable end of transfer interrupt.
	NVIC_ClearPendingIRQ(SPI4_IRQn);
	NVIC_EnableIRQ(SPI4_IRQn);
	spi_enable_master_mode(spi);
	spi_set_bits_per_frame(spi, 9);

}

void spi_enable(SPI_TypeDef* spi)
{
	spi->CR1 |= SPI_CR1_SPE;				//Enable SPI.
}

void spi_disable(SPI_TypeDef* spi)
{
	spi->CR1 &= ~(SPI_CR1_SPE);
}

void spi4_enable_clocks()
{
	RCC->APB2ENR |= RCC_APB2ENR_SPI4EN;							//Enable peripheral clock.

	/*Set the kernel clock to PLL3Q.*/
	RCC->D2CCIP1R &= ~(0x7 << RCC_D2CCIP1R_SPI45SEL_Pos);
	RCC->D2CCIP1R |= RCC_D2CCIP1R_SPI45SEL_PLL3Q << RCC_D2CCIP1R_SPI45SEL_Pos;
}

void spi4_enable_io()
{
	io_init();									//Enables IO clocks.
	io_set_pin_mux(SPI4_SCK_io, GPIO_AFR_AF5);
	io_set_pin_mux(SPI4_MISO_io, GPIO_AFR_AF5);
	io_set_pin_mux(SPI4_MOSI_io, GPIO_AFR_AF5);
	//io_set_pin_mux(SPI4_SS_io, GPIO_AFR_AF5);
	io_set_pin_dir_out(SPI4_SS_io);
	io_set_output_speed(SPI4_SS_io, GPIO_OSPEEDR_VERY_HIGH);
	io_set_output_type(SPI4_SS_io, IO_OUTPUT_TYPE_PUSH_PULL);
}

void spi_set_sck_div(SPI_TypeDef* spi, SPI_SCK_DIV_t div)
{
	spi->CFG1 &= ~(0x7 << SPI_CFG1_MBR_Pos);		//Clear the bits.
	spi->CFG1 |= div << SPI_CFG1_MBR_Pos;			//Set the bits.
}

void spi4_set_ss_low()
{
	io_pin_out_clr(SPI4_SS_io);
}

void spi4_set_ss_high()
{
	io_pin_out_set(SPI4_SS_io);
}

int8_t spi_tx(SPI_TypeDef* spi, uint32_t data, uint8_t bits, bool hold_ss_low)
{
	if ((spi->CR1 & SPI_CR1_CSTART) != 0)
	{
		return 1;
	}

	spi_disable(spi);
	spi->CR2 = 1 << SPI_CR2_TSIZE_Pos;		//Write the number of bytes to be transmitted.
	spi->CR1 |= SPI_CR1_HDDIR;						//Specify half-duplex direction as transmitter.
	SPI4->IFCR = SPI_IFCR_TXTFC						//Need to clear this interrupt otherwise it wont start the next transfer.
			| SPI_IFCR_EOTC;
	spi_set_bits_per_frame(spi, bits);					//Change to 9 for transmit, need to include D/C bit for LCD.
	spi_enable(spi);

	if (bits <= 8)
	{
		spi->TXDR = (uint8_t)(data);
	}
	else if ((bits > 8) && (bits <= 16))
	{
		spi->TXDR = (uint16_t)(data);
	}
	else if (bits > 16)
	{
		spi->TXDR = (uint32_t)(data);
	}

	spi4_set_ss_low(spi);						//Pull CS low.
	spi_start(spi);								//Start the data transmission.
	while ((SPI4->SR & SPI_SR_EOT) == 0){}		//Wait for the transfer to complete.

	if (hold_ss_low == false)
	{
		spi4_set_ss_high();						//Pull SS high.
	}
	else
	{
		return 0;
	}

	spi->IFCR = SPI_IFCR_EOTC_Msk;
	return 0;
}


int8_t spi_rx(SPI_TypeDef* spi, uint32_t* data, uint8_t bits, bool hold_ss_low)
{
	spi_disable(spi);
	spi->CR2 = 1 << SPI_CR2_TSIZE_Pos;		//Write the number of bytes to be transmitted.
	spi->CR1 &= ~(SPI_CR1_HDDIR);						//Specify half-duplex direction as receiver.
	spi_set_bits_per_frame(spi, bits);						//Switch back to 8 bits for recieving.
	spi_enable(spi);

	/*** Dummy cycle ***/
	/*for (uint32_t i = 0; i < num_bytes_read; i++)
	{
		spi->TXDR = 0xFF;
	}*/
	spi_start(spi);

	while ((SPI4->SR & SPI_SR_EOT) == 0){}		//Wait for the transfer to complete.

	if (bits <= 8)
	{
		*data = (uint8_t)spi->RXDR;
	}
	else if ((bits > 8) && (bits <= 16))
	{
		*data = (uint16_t)spi->RXDR;
	}
	else if (bits > 16)
	{
		*data = (uint32_t)spi->RXDR;
	}

	if (hold_ss_low == false)
	{
		spi4_set_ss_high();						//Pull SS high.
		return 1;
	}
	else
	{
		return 0;
	}
}

void SPI4_IRQHandler()
{
	if ((SPI4->SR & SPI_SR_EOT) != 0)
	{
		spi4_set_ss_high();				//Pull SS high.
	}
	SPI4->IFCR = SPI_IFCR_TXTFC			//Need to clear this interrupt otherwise it wont start the next transfer.
			| SPI_IFCR_EOTC;
	SPI4->IER &= ~(SPI_IER_EOTIE);		//Disable the interrupt.
}
