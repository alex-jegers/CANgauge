/*
 * stm32_dma2d.h
 *
 *  Created on: Mar 8, 2024
 *      Author: awjpp
 */

#ifndef DRIVERS_STM32_DMA2D_H_
#define DRIVERS_STM32_DMA2D_H_

/***********	INCLUDES	************/
#include "stm32h745xx.h"


/***********	TYPEDEFS	************/
typedef enum
{
	CONFIGURATION_ERROR				= DMA2D_CR_CEIE,
	CLUT_TRANSFER_COMPLETE			= DMA2D_CR_CTCIE,
	CLUT_ACCESS_ERROR				= DMA2D_CR_CAEIE,
	TRANSFER_WATERMARK				= DMA2D_CR_TWIE,
	TRANSFER_COMPLETE				= DMA2D_CR_TCIE,
	TRANSFER_ERROR					= DMA2D_CR_TEIE,
}dma2d_cr_enable_interrupt_t;

/***********	GLOBAL FUNCTION DECLARATIONS	************/
void dma2d_init();
void dma2d_start_copy(void* src_addr, void* dst_addr, uint32_t x_size, uint32_t y_size);
void dma2d_enable_interrupt(dma2d_cr_enable_interrupt_t interrupt);

void dma2d_nvic_enable_interrupt();
void dma2d_set_transfer_complete_handler(void (*func)());
void dma2d_set_transfer_error_handler(void (*func)());


#endif /* DRIVERS_STM32_DMA2D_H_ */
