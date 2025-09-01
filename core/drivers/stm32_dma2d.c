/*
 * stm32_dma2d.c
 *
 *  Created on: Mar 8, 2024
 *      Author: awjpp
 */

#include "stm32_dma2d.h"

/***********	TYPEDEFS	************/
typedef enum
{
	MEM_TO_MEM						= 0x0,
	MEM_TO_MEM_PFC					= 0x1 << DMA2D_CR_MODE_Pos,
	MEM_TO_MEM_BLEND				= 0x2 << DMA2D_CR_MODE_Pos,
	REG_TO_MEM						= 0x3 << DMA2D_CR_MODE_Pos,
	MEM_TO_MEM_BLEND_FIXED_FG		= 0x4 << DMA2D_CR_MODE_Pos,
	MEM_TO_MEM_BLEND_FIXED_BG		= 0x5 << DMA2D_CR_MODE_Pos,
	CR_MODE_MASK					= 0x7 << DMA2D_CR_MODE_Pos,
}dma2d_cr_mode_t;

typedef enum
{
	ARGB8888,
	RGB888,
	RGB565,
	ARGB1555,
	L8,
	AL44,
	AL88,
	L4,
	A8,
	A4,
	YCbCr,
	FGPFCCR_MASK			= 0xF,
}dma2d_fgpfccr_color_mode_t;

/***********	STATIC VARIABLES	************/
static uint32_t dma2d_disp_x_size = 0;
static uint32_t dma2d_disp_y_size = 0;
void (*dma2d_transfer_complete_handler)();
void (*dma2d_transfer_error_handler)();

/***********	STATIC FUNCTION DECLARATIONS	************/
static void dma2d_set_transfer_mode(dma2d_cr_mode_t mode);
static void dma2d_set_color_mode(dma2d_fgpfccr_color_mode_t fgpgccr_mode);
static void dma2d_start(dma2d_cr_enable_interrupt_t interrupt, dma2d_cr_mode_t mode);

/***********	STATIC FUNCTION DEFINTIONS	************/
static void dma2d_set_transfer_mode(dma2d_cr_mode_t cr_mode)
{
	DMA2D->CR &= ~(CR_MODE_MASK);	//Clear bits
	DMA2D->CR |= cr_mode;			//Set bits.
}

static void dma2d_set_color_mode(dma2d_fgpfccr_color_mode_t fgpgccr_mode)
{
	DMA2D->FGPFCCR &= ~(FGPFCCR_MASK);	//Clear bits.
	DMA2D->FGPFCCR |= fgpgccr_mode;		//Set bits.
}

static void dma2d_start(dma2d_cr_enable_interrupt_t interrupt, dma2d_cr_mode_t mode)
{
	DMA2D->CR = 0;
	DMA2D->CR = interrupt | mode | DMA2D_CR_START;
}

/***********	GLOBAL FUNCTION DEFINTIONS	************/
void dma2d_init(uint32_t disp_x_size, uint32_t disp_y_size)
{
	RCC->AHB3ENR |= RCC_AHB3ENR_DMA2DEN;		//TODO: RCC function for this.
	dma2d_disp_x_size = disp_x_size;
	dma2d_disp_y_size = disp_y_size;
}

void dma2d_start_copy(void* src_addr, void* dst_addr, uint32_t x_size, uint32_t y_size)
{
	DMA2D->FGMAR = (uint32_t)src_addr;			//Set the source addr.
	DMA2D->OMAR = (uint32_t)dst_addr;			//Set the destination addr.

	/*Set the line offset. In pixels when LOM = 0 in CR.*/
	DMA2D->OOR = dma2d_disp_x_size - x_size;

	/*Set pixels per line in pixels.*/
	uint16_t pixels_per_line = x_size;
	uint16_t number_of_lines = y_size;
	DMA2D->NLR = number_of_lines | (pixels_per_line << DMA2D_NLR_PL_Pos);

	/*Set color mode.*/
	dma2d_set_color_mode(ARGB8888);

	/*Set the data transfer type.*/
	dma2d_set_transfer_mode(MEM_TO_MEM);

	/*Start the transfer.*/
	dma2d_start(TRANSFER_COMPLETE, MEM_TO_MEM);
}

void dma2d_enable_interrupt(dma2d_cr_enable_interrupt_t interrupt)
{
	DMA2D->CR |= interrupt;
}

void dma2d_nvic_enable_interrupt()
{
	NVIC_EnableIRQ(DMA2D_IRQn);
}

void dma2d_set_transfer_complete_handler(void (*func)())
{
	dma2d_transfer_complete_handler = func;
}

void dma2d_set_transfer_error_handler(void (*func)())
{
	dma2d_transfer_error_handler = func;
}


/***********	INTERRUPT HANDLERS	************/
void DMA2D_IRQHandler()
{
	uint32_t isr = DMA2D->ISR;

	if (isr & DMA2D_ISR_TCIF)
	{
		if (dma2d_transfer_complete_handler)
		{
			dma2d_transfer_complete_handler();
		}
	}

	if (isr & DMA2D_ISR_TEIF)
	{
		if (dma2d_transfer_error_handler)
		{
			dma2d_transfer_error_handler();
		}
	}

	/*Clears all interrupt flags.*/
	DMA2D->IFCR = 0x1F;
}
