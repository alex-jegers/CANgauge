/*
 * stm32_hsem.h
 *
 *  Created on: May 22, 2024
 *      Author: awjpp
 */
#define USE_STM32_HSEM 1
#if USE_STM32_HSEM

#ifndef DRIVERS_STM32_HSEM_H_
#define DRIVERS_STM32_HSEM_H_

/**********     INCLUDES     **********/
#include "stm32h7xx.h"
#include "stdbool.h"
#include "stddef.h"

/**********     DEFINES     **********/
#define CM4_CORE_ID			1
#define CM7_CORE_ID			3

#define HSEM_CLEAR_ALL_KEY	0

#ifdef CORE_CM7
#define ACTIVE_CORE_ID		CM7_CORE_ID
#endif

#ifdef CORE_CM4
#define ACTIVE_CORE_ID		CM4_CORE_ID
#endif


/**********     GLOBAL FUNCTION DELCRATIONS     **********/
void hsem_init_clk();									//initializes the bus clock.
bool hsem_signal(uint32_t hsem, uint32_t procid);		//signals a semaphore.
void hsem_wait_void(uint32_t hsem, uint32_t procid);	//waits for a signal, sits in a loop does not return anything.
bool hsem_wait_bool(uint32_t hsem, uint32_t procid);	//returns false while waiting.
bool hsem_fast_lock(uint32_t id);						//performs the 1 step lock procedure.
bool hsem_lock(uint32_t hsem, uint32_t procid);			//performs the 2 step lock procedure.
bool hsem_unlock(uint32_t hsem, uint32_t procid);		//unlocks the semaphore.
uint32_t hsem_get_status();								//returns the interrupt status register.
void hsem_clear_all(uint32_t key);
void hsem_clear_int(uint32_t hsem);						//clear an interrupt.
void hsem_clear_all_int();								//clear all the interrupts.
void hsem_enable_interrupt(uint32_t hsem_msk);			//enables interrupt(s)
void hsem_disable_interrupt(uint32_t hsem_msk);			//disables interrupt(s)
void hsem1_assign_int_handler(void (*func)());			//assigns the interrupt handler function for HSEM1.
void hsem2_assign_int_handler(void (*func)());			//assigns the interrupt handler function for HSEM2.

void HSEM2_IRQHandler();


#endif /* DRIVERS_STM32_HSEM_H_ */

#endif	//USE_STM32_HSEM
