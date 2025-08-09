
/***********	INCLUDES	************/
#include "stm32h745xx.h"
#include "stm32_io.h"
#include "stdbool.h"

/***********	TYPEDEFS	************/
typedef enum 
{
    SPI_SCK_DIV_2   = 0,
    SPI_SCK_DIV_4   = 1,
    SPI_SCK_DIV_8   = 2,
    SPI_SCK_DIV_16  = 3,
    SPI_SCK_DIV_32  = 4,
    SPI_SCK_DIV_64  = 5,
    SPI_SCK_DIV_128 = 6,
    SPI_SCK_DIV_256 = 7
}SPI_SCK_DIV_t;

/***********	DEFINES		************/
#define SPI4_SCK_io			GPIOE, GPIO_PIN2_Msk
#define SPI4_MISO_io		GPIOE, GPIO_PIN5_Msk
#define SPI4_MOSI_io		GPIOE, GPIO_PIN6_Msk
#define SPI4_SS_io			GPIOE, GPIO_PIN4_Msk
#define SPI_CFG2_COMM_HALF_DUPLEX		0x3

/***********	VARIABLE DEFINITIONS	************/

/***********	GLOBAL FUNCTION DECLARATIONS	************/
void spi_init(SPI_TypeDef* spi);
void spi_enable(SPI_TypeDef* spi);		//Call last, enables SPI and write locks a bunch of registers.
void spi_disable(SPI_TypeDef* spi);		//Disables SPI and write unlocks the registers that are locked when enabled.
void spi4_enable_clocks();				//Enables the peripheral clock and sets the kernel clock to PLL3Q.
void spi4_enable_io();					//Enables the IO associated with SPI4 (see defines in this header file).
void spi_set_sck_div(SPI_TypeDef* spi, SPI_SCK_DIV_t div);  //Sets the division factor for the SPI clock. The kernel clock being used is PLL3R.
void spi4_set_ss_low();
void spi4_set_ss_high();

int8_t spi_tx(SPI_TypeDef* spi, uint32_t data, uint8_t bits, bool hold_ss_low);		//Returns 0 if the transmission failed.
int8_t spi_rx(SPI_TypeDef* spi, uint32_t* data, uint8_t bits, bool hold_ss_low);
