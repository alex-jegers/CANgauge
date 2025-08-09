/*
 * stm32_rcc.c
 *
 *  Created on: Feb 17, 2024
 *      Author: awjpp
 */

/***********CLOCK SPEEDS****************/
/*
 * HSI	=	8MHz
 *
 * PLL1
 * 	R	=	240MHz
 * 	Q	=	480MHz
 * 	P 	=	480MHz
 */


#include "stm32_rcc.h"





static void rcc_init_pll();
static void rcc_init_pll1();		//480MHz
static void rcc_init_pll2();		//
static void rcc_init_pll3();		//


void rcc_main_clock_config()
{
	/*Divide the HSI (64MHz) by 8 so sys_clk = 8MHz*/
	RCC->CR |=  RCC_CR_HSIDIV_8;

	/*Initialize the PLL to accept 8MHz HSI.*/
	rcc_init_pll();

	/*Initialize all 3 PLLs.*/
	rcc_init_pll1();
	rcc_init_pll2();
	rcc_init_pll3();

	/*Enable PLL1P for the sys_clk.*/
	rcc_enable_pll1p();

	/*Set domain 1 prescalers.*/
	RCC->D1CFGR = RCC_D1CFGR_HPRE_DIV2
				| RCC_D1CFGR_D1CPRE_DIV1
				| RCC_D1CFGR_D1PPRE_DIV2;

	/*Set domain 2 prescalers.*/
	RCC->D2CFGR = RCC_D2CFGR_D2PPRE1_DIV2
				| RCC_D2CFGR_D2PPRE2_DIV2;

	/*Set domain 3 prescalers.*/
	RCC->D3CFGR = RCC_D3CFGR_D3PPRE_DIV2;

	/*Switch to VOS0.*/
	PWR->CR3 = PWR_CR3_LDOEN;						//Enable just the LDO.
	PWR->D3CR = PWR_D3CR_VOS;						//Switch to VOS2.
	RCC->APB4ENR |= RCC_APB4ENR_SYSCFGEN;			//Enable SYSCFG clock.
	SYSCFG->PWRCR |= SYSCFG_PWRCR_ODEN;				//Enable VOS0.
	while ((PWR->D3CR & PWR_D3CR_VOSRDY) == 0) {}	//Wait for VOSRDY to be set.

	/*Set the sys_clk source to PLL1.*/
	RCC->CFGR |= RCC_CFGR_SW_PLL1;

	/*Wait until the clock has successfully switched.*/
	while ((RCC->CFGR & RCC_CFGR_SWS_PLL1) == 0) {}


}

void rcc_init_systick()
{
	/*Configure SysTick w/ x8 divider and enable interrupt, 1ms interrupts.*/
	SysTick->LOAD = F_CPU_M7 / 8000;
	SysTick->CTRL = SysTick_CTRL_TICKINT_Msk
					| SysTick_CTRL_ENABLE_Msk;
}

void rcc_c2_clock_config()
{
	/*Configure SysTick w/ x8 divider and enable interrupt, 1ms interrupts.*/
	SysTick->LOAD = F_CPU_M4 / 8000;
	SysTick->CTRL = SysTick_CTRL_TICKINT_Msk
					| SysTick_CTRL_ENABLE_Msk;
}

static void rcc_init_pll()
{
	/*Set the PLL1 CLK source to the HSI and divide by 1 so PLL in = 8MHz.*/
	RCC->PLLCKSELR = RCC_PLLCKSELR_PLLSRC_HSI;

	/*Set the all PLL refclk to 8MHz-16MHz range.*/
	RCC->PLLCFGR |= RCC_PLLCFGR_PLL1RGE_3
				| RCC_PLLCFGR_PLL2RGE_3
				| RCC_PLLCFGR_PLL3RGE_3;
}

/*Initialize PLL1 to 480MHz.*/
static void rcc_init_pll1()
{
	/*Set the initial divider to 1.*/
	RCC->PLLCKSELR |= 1 << RCC_PLLCKSELR_DIVM1_Pos;

	/*Multiply by 59 + 1, so this gives 8 * 60  = 480.*/
	RCC->PLL1DIVR = (14 << RCC_PLL1DIVR_N1_Pos) 	//Mult 60..
				| (0 << RCC_PLL1DIVR_R1_Pos)		//Div 2,	R = 240MHz
				| (0 << RCC_PLL1DIVR_Q1_Pos)		//No Div,	Q = 480MHz
				| (0 << RCC_PLL1DIVR_P1_Pos);		//No div,	P = 480MHz

	/*Enable PLL1.*/
	RCC->CR |= RCC_CR_PLL1ON;
	while ((RCC->CR & RCC_CR_PLL1RDY) == 0){}
}

/*Initialize PLL2.*/
static void rcc_init_pll2()
{
	/*Set the initial divider to 1.*/
	RCC->PLLCKSELR |= 1 << RCC_PLLCKSELR_DIVM2_Pos;

	/*Multiply by 28 + 1, so this gives 8 * 29  = 232.*/
	RCC->PLL2DIVR = (28 << RCC_PLL2DIVR_N2_Pos) 	//Mult 29..
				| (0 << RCC_PLL2DIVR_R2_Pos)		//No div,	R = 232MHz
				| (0 << RCC_PLL2DIVR_Q2_Pos)		//No Div,	Q = 232MHz
				| (0 << RCC_PLL2DIVR_P2_Pos);		//No div,	P = 232MHz

	/*Enable PLL1.*/
	RCC->CR |= RCC_CR_PLL2ON;
	while ((RCC->CR & RCC_CR_PLL2RDY) == 0){}
}


static void rcc_init_pll3()
{
	/*Set the initial divider to 1.*/
	RCC->PLLCKSELR |= 1 << RCC_PLLCKSELR_DIVM3_Pos;

	/*Set up the dividers.*/
	RCC->PLL3DIVR = (26 << RCC_PLL3DIVR_N3_Pos)		//Mult 27...
				| (25 << RCC_PLL3DIVR_R3_Pos)		//Div 18,	R = 12MHz	//TODO: CHANGE BACK TO 17
				| (17 << RCC_PLL3DIVR_Q3_Pos)		//Div 18,	Q = 12MHz
				| (0 << RCC_PLL3DIVR_P3_Pos);		//No div, 	P = 216MHz

	/*Enable PLL3.*/
	RCC->CR |= RCC_CR_PLL3ON;
	while ((RCC->CR & RCC_CR_PLL3RDY) == 0) {}

}


void rcc_enable_pll1p()
{
	RCC->PLLCFGR |= RCC_PLLCFGR_DIVP1EN;
}
void rcc_enable_pll1q()
{
	RCC->PLLCFGR |= RCC_PLLCFGR_DIVQ1EN;
}
void rcc_enable_pll1r()
{
	RCC->PLLCFGR |= RCC_PLLCFGR_DIVR1EN;
}
void rcc_enable_pll2p()
{
	RCC->PLLCFGR |= RCC_PLLCFGR_DIVP2EN;
}
void rcc_enable_pll2q()
{
	RCC->PLLCFGR |= RCC_PLLCFGR_DIVQ2EN;
}
void rcc_enable_pll2r()
{
	RCC->PLLCFGR |= RCC_PLLCFGR_DIVR2EN;
}
void rcc_enable_pll3p()
{
	RCC->PLLCFGR |= RCC_PLLCFGR_DIVP3EN;
}
void rcc_enable_pll3q()
{
	RCC->PLLCFGR |= RCC_PLLCFGR_DIVQ3EN;
}
void rcc_enable_pll3r()
{
	RCC->PLLCFGR |= RCC_PLLCFGR_DIVR3EN;
}
