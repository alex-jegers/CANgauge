/*
 * stm32_rcc.h
 *
 *  Created on: Feb 17, 2024
 *      Author: awjpp
 */

#ifndef INC_STM32_RCC_H_
#define INC_STM32_RCC_H_

#include "stm32h745xx.h"

#define F_CPU_M7			480000000
#define F_CPU_M4			240000000
#define RCC_F_TIMERS_CLK	240000000

/*Kernel clock select bit masks.*/
#define RCC_D1CCIPR_FMCSEL_PLL1Q		0x1
#define RCC_D1CCIPR_FMCSEL_PLL2R		0x2
#define RCC_D2CCIP1R_SPI45SEL_PLL3Q		0x2


void rcc_main_clock_config();
void rcc_init_systick();
void rcc_c2_clock_config();

void rcc_enable_pll1p();
void rcc_enable_pll1q();
void rcc_enable_pll1r();
void rcc_enable_pll2p();
void rcc_enable_pll2q();
void rcc_enable_pll2r();
void rcc_enable_pll3p();
void rcc_enable_pll3q();
void rcc_enable_pll3r();


#endif /* INC_STM32_RCC_H_ */
