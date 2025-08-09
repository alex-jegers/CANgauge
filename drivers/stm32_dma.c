/*
 * stm32_dma.c
 *
 *  Created on: Feb 25, 2024
 *      Author: awjpp
 */


#include "stm32_dma.h"
#include "stm32_io.h"

static MDMA_Channel_TypeDef* MDMA_Channel[16] = { MDMA_Channel0,   
										MDMA_Channel1,   
										MDMA_Channel2,   
										MDMA_Channel3,   
										MDMA_Channel4,   
										MDMA_Channel5,   
										MDMA_Channel6,   
										MDMA_Channel7,   
										MDMA_Channel8,   
										MDMA_Channel9,   
										MDMA_Channel10,  
										MDMA_Channel11,  
										MDMA_Channel12,  
										MDMA_Channel13,  
										MDMA_Channel14,  
										MDMA_Channel15 };  

static void (*mdma_block_complete_handler)();

void mdma_init()
{
	/*Enable the AHB clock for the MDMA.*/
	RCC->AHB3ENR |= RCC_AHB3ENR_MDMAEN;

	/*Enable MDMA interrupts.*/
	NVIC_EnableIRQ(MDMA_IRQn);
}


void mdma_start_transfer(void* src_addr, void* dst_addr, uint32_t bytes)
{
	/*Set the buffer size.*/
	uint32_t bytes_per_transfer = bytes - 1;
	if (bytes_per_transfer > 127)
	{
		bytes_per_transfer = 127;
	}
	MDMA_Channel[0]->CTCR |= bytes_per_transfer << MDMA_CTCR_TLEN_Pos
							| MDMA_CTCR_SWRM					//Software request mode.
							| 1 << MDMA_CTCR_TRGM_Pos			//Block transfer mode.
							| 0x2 << MDMA_CTCR_DINCOS_Pos		//Inc dest addr by 32 bits.
							| 0x2 << MDMA_CTCR_SINCOS_Pos		//Inc src addr by 32 bits.
							| 0x2 << MDMA_CTCR_DSIZE_Pos		//Dest data size is 32 bits.
							| 0x2 << MDMA_CTCR_SSIZE_Pos		//Src data size is 32 bits.
							| 0x2 << MDMA_CTCR_DINC_Pos			//Dest addr is incremented each transfer.
							| 0x2 << MDMA_CTCR_SINC_Pos;		//Src addr is incremented each transfer.;

	/*Set the size of the entire block to be transferred.*/
	uint32_t block_size = bytes;
	MDMA_Channel[0]->CBNDTR = block_size;

	/*Set the src and dest addresses.*/
	MDMA_Channel[0]->CSAR = (uint32_t)src_addr;
	MDMA_Channel[0]->CDAR = (uint32_t)dst_addr;

	/*Enable the DMA and transfer complete interrupt and request the transfer to start.*/
	MDMA_Channel[0]->CCR |= MDMA_CCR_EN;
	MDMA_Channel[0]->CCR |= MDMA_CCR_SWRQ;
}


void mdma_set_block_complete_handler(uint8_t channel_num, void (*func)())
{
	mdma_block_complete_handler = func;
}



void mdma_enable_interrupt(uint8_t channel_num, uint32_t mdma_ccr_xie)
{
	MDMA_Channel[channel_num]->CCR |= mdma_ccr_xie;
}

void mdma_nvic_enable_interrupt()
{
	NVIC_EnableIRQ(MDMA_IRQn);
}

void MDMA_IRQHandler()
{
	uint32_t ir = MDMA_Channel[0]->CISR;
	if (ir & MDMA_CISR_BTIF)
	{
		if (mdma_block_complete_handler) {
			mdma_block_complete_handler();
		}
	}

	MDMA_Channel[0]->CIFCR = 0x1F;
}

#if MDMA_TEST_MODE == 1
uint32_t src_data[5] = { 0x0BADC0DE, 0xDEADBEEF, 0xBADCAFE0, 0x12345678, 0x55555555 };
uint32_t dst_data[5];
void mdma_test(uint32_t tlen, uint32_t cbndtr)
{
	NVIC_EnableIRQ(MDMA_IRQn);

	MDMA_Channel[0]->CTCR = MDMA_CTCR_SWRM					//Software request mode.
						| 1 << MDMA_CTCR_TRGM_Pos			//Block transfer mode.
						| 0x2 << MDMA_CTCR_DINCOS_Pos		//Inc dest addr by 32 bits.
						| 0x2 << MDMA_CTCR_SINCOS_Pos		//Inc src addr by 32 bits.
						| 0x2 << MDMA_CTCR_DSIZE_Pos		//Dest data size is 32 bits.
						| 0x2 << MDMA_CTCR_SSIZE_Pos		//Src data size is 32 bits.
						| 19 << MDMA_CTCR_TLEN_Pos			//4 bytes transfered per transfer.
						| 0x2 << MDMA_CTCR_DINC_Pos			//Dest addr is incremented each transfer.
						| 0x2 << MDMA_CTCR_SINC_Pos;		//Src addr is incremented each transfer.

	MDMA_Channel[0]->CBNDTR = 20;

	MDMA_Channel[0]->CSAR = (uint32_t)&src_data;
	MDMA_Channel[0]->CDAR = (uint32_t)&dst_data;

	io_pin_out_set(GPIOA, GPIO_PIN8_Msk);
	io_pin_out_clr(GPIOA, GPIO_PIN8_Msk);

	MDMA_Channel[0]->CCR = MDMA_CCR_EN
						| MDMA_CCR_SWRQ
						| MDMA_CCR_CTCIE;

}
#endif	//MDMA_TEST_MODE



