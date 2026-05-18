/*
 * stm32_rcc.h
 *
 *  Created on: Feb 17, 2024
 *      Author: awjpp
 */

#ifndef INC_STM32_RCC_H_
#define INC_STM32_RCC_H_

#ifdef __cplusplus
extern "C" {
#endif

/**********     INCLUDES        **********/
#include "stm32h745xx.h"
#include "system/system_mem.h"

/**********     DEFINES      **********/
#define F_CPU_M7			480000000
#define F_CPU_M4			240000000
#define RCC_F_TIMERS_CLK	240000000

/* Kernel clock select bit masks. */
#define RCC_D1CCIPR_FMCSEL_PLL1Q		0x1
#define RCC_D1CCIPR_FMCSEL_PLL2R		0x2
#define RCC_D2CCIP1R_SPI45SEL_PLL3Q		0x2

/* Wont continue until PLL1RDY flag is set. */
#define rcc_enable_pll_1()       RCC->CR |= RCC_CR_PLL1ON; while ((RCC->CR & RCC_CR_PLL1RDY) == 0){} 
/* Wont continue until PLL2RDY flag is set. */
#define rcc_enable_pll_2()       RCC->CR |= RCC_CR_PLL2ON; while ((RCC->CR & RCC_CR_PLL2RDY) == 0){} 
/* Wont continue until PLL3RDY flag is set. */
#define rcc_enable_pll_3()       RCC->CR |= RCC_CR_PLL3ON; while ((RCC->CR & RCC_CR_PLL3RDY) == 0){} 

#define rcc_disable_pll_1()       RCC->CR &= ~(RCC_CR_PLL1ON)
#define rcc_disable_pll_2()       RCC->CR &= ~(RCC_CR_PLL2ON)
#define rcc_disable_pll_3()       RCC->CR &= ~(RCC_CR_PLL3ON)

#define rcc_enable_pll_1p()	    RCC->PLLCFGR |= RCC_PLLCFGR_DIVP1EN
#define rcc_enable_pll_1q()     RCC->PLLCFGR |= RCC_PLLCFGR_DIVQ1EN
#define rcc_enable_pll_1r()     RCC->PLLCFGR |= RCC_PLLCFGR_DIVR1EN
#define rcc_enable_pll_2p()     RCC->PLLCFGR |= RCC_PLLCFGR_DIVP2EN
#define rcc_enable_pll_2q()     RCC->PLLCFGR |= RCC_PLLCFGR_DIVQ2EN
#define rcc_enable_pll_2r()     RCC->PLLCFGR |= RCC_PLLCFGR_DIVR2EN
#define rcc_enable_pll_3p()     RCC->PLLCFGR |= RCC_PLLCFGR_DIVP3EN
#define rcc_enable_pll_3q()     RCC->PLLCFGR |= RCC_PLLCFGR_DIVQ3EN
#define rcc_enable_pll_3r()     RCC->PLLCFGR |= RCC_PLLCFGR_DIVR3EN

#define rcc_disable_pll_1p()	RCC->PLLCFGR &= ~(RCC_PLLCFGR_DIVP1EN | RCC_PLL1DIVR_R1)
#define rcc_disable_pll_1q()    RCC->PLLCFGR &= ~(RCC_PLLCFGR_DIVQ1EN | RCC_PLL1DIVR_R1)
#define rcc_disable_pll_1r()    RCC->PLLCFGR &= ~(RCC_PLLCFGR_DIVR1EN | RCC_PLL1DIVR_R1)
#define rcc_disable_pll_2p()    RCC->PLLCFGR &= ~(RCC_PLLCFGR_DIVP2EN | RCC_PLL1DIVR_R1)
#define rcc_disable_pll_2q()    RCC->PLLCFGR &= ~(RCC_PLLCFGR_DIVQ2EN | RCC_PLL1DIVR_R1)
#define rcc_disable_pll_2r()    RCC->PLLCFGR &= ~(RCC_PLLCFGR_DIVR2EN | RCC_PLL1DIVR_R1)
#define rcc_disable_pll_3p()    RCC->PLLCFGR &= ~(RCC_PLLCFGR_DIVP3EN | RCC_PLL1DIVR_R1)
#define rcc_disable_pll_3q()    RCC->PLLCFGR &= ~(RCC_PLLCFGR_DIVQ3EN | RCC_PLL1DIVR_R1)
#define rcc_disable_pll_3r()	RCC->PLLCFGR &= ~(RCC_PLLCFGR_DIVR3EN | RCC_PLL1DIVR_R1)

#define rcc_reset_usb2otg()			RCC->AHB1RSTR |= RCC_AHB1RSTR_USB2OTGFSRST
#define rcc_clr_reset_usb2otg()		RCC->AHB1RSTR &= ~(RCC_AHB1RSTR_USB2OTGFSRST)

/**********     TYPEDEFS        **********/
typedef enum 
{
    PER_CK_SRC_HSI,
    PER_CK_SRC_CSI,
    PER_CK_SRC_HSE,
}per_ck_src_t;

typedef enum
{
    RCC_SYS_CK_HSI,
    RCC_SYS_CK_CSI,
    RCC_SYS_CK_HSE,
    RCC_SYS_CK_PLL1_P
}rcc_sys_ck_t;

/**********     GLOBAL VARIABLE DECLRATIONS     **********/

/**********		GLOBAL FUNCTION DECLRATIONS		**********/
void rcc_main_clock_config();
void rcc_init_systick();
/**
 * rcc_enable_csi:
 * desc: turns on the csi oscillator and waits for it to be ready.
 *  Waits for the clock to become stable before returning.
 */
void rcc_enable_csi();
/**
 * rcc_select_per_ck:
 * desc: selects the source for per_ck.
 */
void rcc_select_per_ck(per_ck_src_t ck);
void rcc_c2_clock_config();

/**
 * rcc_config_pll_x:
 * desc: sets the initial, overall multiplier and the dividers for the sub-
 *      PLLs (p, q, r). 
 *      Ex: PLL1_R frequency(Hz) = input frequency * mult / div_r.
 *      The output frequency of the VCO (input freq. * mult) needs to be in
 *      compliance with PLLxVCOSEL setting (assumes PLLxVCOSEL is 0). 
 *      PLL needs to be disabled.
 *      Assumes an 8MHz input.
 *      The function will return 0 if anything not allowed is attemped with 
 *      any of the passed params.
 *      NOTE THAT - PLL1R, div 1 is not allowed, min value of 2.
 *          PLL1P, odd division values are not allowed except 1.
 *          PLL multipliers, minimum value of 4.
 * params: 
 *      mult: how much to multiply the input frequency by.
 *      div_x: how much to divide the VCO output by.
 * returns:
 *      0: if config settings are NOT valid and were not changed.
 *      non-zero: config settings are valid and were changed. 
 */
int8_t rcc_config_pll_1(uint16_t mult, uint8_t div_p, uint8_t div_q, uint8_t div_r);
/**
 * rcc_config_pll_x:
 * desc: sets the initial, overall multiplier and the dividers for the sub-
 *      PLLs (p, q, r). See rcc_config_pll_1 comment for more details.
 * params: 
 *      mult: how much to multiply the input frequency by.
 *      div_x: how much to divide the VCO output by.
 * returns:
 *      0: if config settings are NOT valid and were not changed.
 *      non-zero: config settings are valid and were changed. 
 */
int8_t rcc_config_pll_2(uint16_t mult, uint8_t div_p, uint8_t div_q, uint8_t div_r);
/**
 * rcc_config_pll_x:
 * desc: sets the initial, overall multiplier and the dividers for the sub-
 *      PLLs (p, q, r). See rcc_config_pll_1 comment for more details.
 * params: 
 *      mult: how much to multiply the input frequency by.
 *      div_x: how much to divide the VCO output by.
 * returns:
 *      0: if config settings are NOT valid and were not changed.
 *      non-zero: config settings are valid and were changed. 
 */
int8_t rcc_config_pll_3 (uint16_t mult, uint8_t div_p, uint8_t div_q, uint8_t div_r);

/**
 * rcc_disable_all_pll:
 * desc: fully shuts down all PLLs and their outputs.
 */
void rcc_disable_all_pll();

/**
 * rcc_set_sys_ck:
 * desc: sets sys_ck according to ck_src. Does not return until the clock is stable
 *      and running.
 */
void rcc_set_sys_ck(rcc_sys_ck_t ck_src);

SYS_MEM_REGION_RAM_EXE void rcc_sw_reset();
void rcc_set_systick_reload(uint32_t reload);



#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* INC_STM32_RCC_H_ */
