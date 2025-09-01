/*
 * stm32_dma.h
 *
 *  Created on: Feb 25, 2024
 *      Author: awjpp
 */

#ifndef INC_STM32_DMA_H_
#define INC_STM32_DMA_H_

#include "stm32h745xx.h"

#define MDMA_TEST_MODE 1

void mdma_init();
void mdma_start_transfer(void* src_addr, void* dst_addr, uint32_t bytes);

void mdma_set_block_complete_handler(uint8_t channel_num, void (*func)());
void mdma_enable_interrupt(uint8_t channel_num, uint32_t mdma_ccr_xie);
void mdma_nvic_enable_interrupt();											//Enables MDMA0 interrupt in the NVIC.


#if MDMA_TEST_MODE == 1
void mdma_test(uint32_t tlen, uint32_t cbndtr);
#endif	//MDMA_TEST_MODE


#endif /* INC_STM32_DMA_H_ */
