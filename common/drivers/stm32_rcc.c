/*
 * stm32_rcc.c
 *
 *  Created on: Feb 17, 2024
 *      Author: awjpp
 */


/**********     INCLUDES        **********/
#include "stm32_rcc.h"
#include <assert.h>

/**********		EXTERNAL VARIABLE DEFINITIONS		**********/

/**********		STATIC VARIABLES		**********/

/**********		STATIC FUNCTION DECLRATIONS		**********/
static void rcc_init_pll();
static void rcc_init_pll1();		//480MHz
static void rcc_init_pll2();		//
static void rcc_init_pll3();		//
static int8_t prv_config_pll(uint32_t* rcc_pll, uint16_t mult, uint8_t div_p, uint8_t div_q, uint8_t div_r);

/**********		STATIC FUNCTION DEFINITIONS		**********/
static void rcc_init_pll()
{
	/*Set the PLL1 CLK source to the HSI and divide by 1 so PLL in = 8MHz.*/
	RCC->PLLCKSELR = RCC_PLLCKSELR_PLLSRC_HSI;

	/*Set the all PLL refclk to 8MHz-16MHz range.*/
	RCC->PLLCFGR |= RCC_PLLCFGR_PLL1RGE_3
				| RCC_PLLCFGR_PLL2RGE_3
				| RCC_PLLCFGR_PLL3RGE_3;
}

static void rcc_init_pll1()
{
	/*Set the initial divider to 1.*/
	RCC->PLLCKSELR |= 1 << RCC_PLLCKSELR_DIVM1_Pos;

	/*Multiply by 59 + 1, so this gives 8 * 60  = 480.*/
	RCC->PLL1DIVR = (59 << RCC_PLL1DIVR_N1_Pos) 	//Mult 60..
				| (1 << RCC_PLL1DIVR_R1_Pos)		//Div 2,	R = 240MHz
				| (0 << RCC_PLL1DIVR_Q1_Pos)		//No Div,	Q = 480MHz
				| (0 << RCC_PLL1DIVR_P1_Pos);		//No div,	P = 480MHz

	/*Enable PLL1.*/
	RCC->CR |= RCC_CR_PLL1ON; while ((RCC->CR & RCC_CR_PLL1RDY) == 0){}
}

static void rcc_init_pll2()
{
	/*Set the initial divider to 1.*/
	RCC->PLLCKSELR |= 1 << RCC_PLLCKSELR_DIVM2_Pos;

	/*Multiply by 32 + 1, so this gives 8 * 33  = 264.*/
	RCC->PLL2DIVR = (32 << RCC_PLL2DIVR_N2_Pos) 	//Mult 33..
				| (0 << RCC_PLL2DIVR_R2_Pos)		//No div,	R = 264MHz
				| (0 << RCC_PLL2DIVR_Q2_Pos)		//No Div,	Q = 264MHz
				| (6 << RCC_PLL2DIVR_P2_Pos);		//Div 7,	P = 37.714MHz

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
				| (8 << RCC_PLL3DIVR_R3_Pos)		//Div 9,	R = 12MHz
				| (17 << RCC_PLL3DIVR_Q3_Pos)		//Div 18,	Q = 12MHz
				| (0 << RCC_PLL3DIVR_P3_Pos);		//No div, 	P = 216MHz

	/*Enable PLL3.*/
	RCC->CR |= RCC_CR_PLL3ON;
	while ((RCC->CR & RCC_CR_PLL3RDY) == 0) {}

}

static int8_t prv_config_pll(uint32_t* rcc_pll, uint16_t mult, uint8_t div_p, uint8_t div_q, uint8_t div_r)
{
	/* Check that none of the dividers are bigger than allowed.*/
	if ((div_p > 128) || (div_q > 128) || (div_r > 128))
	{
		return 0;
	}

	/* Check that the multiplier isnt too big. */
	if ((mult > 512) || (mult < 4))
	{
		return 0;
	}

	/* Check that the VCO output is valid assuming a 8MHz input. */
	if ( ( (8 * mult) < 192) || ( (8 * mult) > 960) )
	{
		return 0;
	}

	*rcc_pll = mult - 1 
			| ((div_p - 1) << RCC_PLL1DIVR_P1_Pos)
			| ((div_q - 1) << RCC_PLL1DIVR_Q1_Pos)
			| ((div_r - 1) << RCC_PLL1DIVR_R1_Pos);

	return 1;
}
/**********		GLOBAL FUNCTION DEFINITIONS		**********/
void rcc_main_clock_config()
{
	/* Allocate SRAM1,2,3 to CPU1. */
	RCC->AHB2ENR |= RCC_AHB2ENR_D2SRAM1EN
				| RCC_AHB2ENR_D2SRAM2EN
				| RCC_AHB2ENR_D2SRAM3EN;

	/*Divide the HSI (64MHz) by 8 so sys_clk = 8MHz*/
	RCC->CR |=  RCC_CR_HSIDIV_8;

	/*Initialize the PLL to accept 8MHz HSI.*/
	rcc_init_pll();

	/*Initialize all 3 PLLs.*/
	assert( rcc_config_pll_1(60, 1, 1, 2) );
	assert( rcc_config_pll_2(29, 7, 1, 1) );
	assert( rcc_config_pll_3(27, 1, 18, 9) );

	/*Enable PLL1P for the sys_clk.*/
	rcc_enable_pll_1p();

	rcc_enable_pll_1();
	rcc_enable_pll_2();
	rcc_enable_pll_3();



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
	PWR->D3CR = PWR_D3CR_VOS;						//Switch to VOS scale 1.
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
	SysTick->LOAD = F_CPU_M7 / 8 / 1000;
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

void rcc_enable_csi()
{
	/* Turn on the oscillator. */
	RCC->CR |= RCC_CR_CSION;
	/* Wait for it to stabilize. */
	while ((RCC->CR & RCC_CR_CSIRDY) == 0) {}
}

void rcc_select_per_ck(per_ck_src_t ck)
{
	RCC->D1CCIPR &= ~(0x3 << RCC_D1CCIPR_CKPERSEL_Pos);
	RCC->D1CCIPR |= (uint32_t)ck << RCC_D1CCIPR_CKPERSEL_Pos;	
}

int8_t rcc_config_pll_1(uint16_t mult, uint8_t div_p, uint8_t div_q, uint8_t div_r)
{
	/* Check for a zero value. */
	if (div_p * div_q * div_r == 0)
	{
		return 0;
	}

	/* Check for div r equals 1. */
	if (div_r == 1)
	{
		return 0;
	}

	/* Check for an odd div p. */
	if (div_p % 2 != 0)
	{
		if (div_p != 1)
		{
			return 0;
		}
	}
	RCC->PLLCKSELR &= ~(0x3F << RCC_PLLCKSELR_DIVM1_Pos);
	RCC->PLLCKSELR |= (0x1 << RCC_PLLCKSELR_DIVM1_Pos);
	return prv_config_pll(&RCC->PLL1DIVR, mult, div_p, div_q, div_r);
}

int8_t rcc_config_pll_2(uint16_t mult, uint8_t div_p, uint8_t div_q, uint8_t div_r)
{
	RCC->PLLCKSELR &= ~(0x3F << RCC_PLLCKSELR_DIVM2_Pos);
	RCC->PLLCKSELR |= (0x1 << RCC_PLLCKSELR_DIVM2_Pos);
	return prv_config_pll(&RCC->PLL2DIVR, mult, div_p, div_q, div_r);	
}

int8_t rcc_config_pll_3 (uint16_t mult, uint8_t div_p, uint8_t div_q, uint8_t div_r)
{
	RCC->PLLCKSELR &= ~(0x3F << RCC_PLLCKSELR_DIVM3_Pos);
	RCC->PLLCKSELR |= (0x1 << RCC_PLLCKSELR_DIVM3_Pos);
	return prv_config_pll(&RCC->PLL3DIVR, mult, div_p, div_q, div_r);		
}

void rcc_disable_all_pll()
{
	rcc_disable_pll_1();
	rcc_disable_pll_2();
	rcc_disable_pll_3();

	rcc_disable_pll_1p();
	rcc_disable_pll_1q();
	rcc_disable_pll_1r();
	rcc_disable_pll_2p();
	rcc_disable_pll_2q();
	rcc_disable_pll_2r();
	rcc_disable_pll_3p();
	rcc_disable_pll_3q();
	rcc_disable_pll_3r();
}

void rcc_set_sys_ck(rcc_sys_ck_t ck_src)
{
	/* Enable HSI and wait for it to be ready. */
	RCC->CR |= RCC_CR_HSION;
	while ((RCC->CR & RCC_CR_HSIRDY) == 0) {}

	RCC->CFGR &= ~(0x3);		//clear the bits, temporarily select HSI for sys ck.

	RCC->CFGR |= ck_src;		//set the bits.

}

SYS_MEM_REGION_RAM_EXE void rcc_sw_reset()
{
	//NVIC_SystemReset();
	  __DSB();                                                          /* Ensure all outstanding memory accesses included
	                                                                       buffered write are completed before reset */
	  SCB->AIRCR  = (uint32_t)((0x5FAUL << SCB_AIRCR_VECTKEY_Pos)    |
	                           (SCB->AIRCR & SCB_AIRCR_PRIGROUP_Msk) |
	                            SCB_AIRCR_SYSRESETREQ_Msk    );         /* Keep priority group unchanged */
	  __DSB();                                                          /* Ensure completion of memory access */

	  for(;;)                                                           /* wait until reset */
	  {
	    __NOP();
	  }
}

void rcc_set_systick_reload(uint32_t reload)
{
	SysTick->LOAD = reload;
}

