/*
 * stm32_hsem.c
 *
 *  Created on: May 22, 2024
 *      Author: awjpp
 */

#include "stm32_hsem.h"


#if USE_STM32_HSEM


/**********     STATIC VARIABLES     **********/
static void (*hsem2_interrupt_cb)();
static void (*hsem1_interrupt_cb)();

/**********     GLOBAL FUNCTION DECLRATIONS     **********/
void hsem_init_clk()
{
	/*Enable the clock.*/
	RCC->AHB4ENR |= RCC_AHB4ENR_HSEMEN;
}

bool hsem_signal(uint32_t hsem, uint32_t procid)
{
	bool return_val = hsem_unlock(hsem, procid);
	hsem_clear_int(hsem);
	return return_val;
}

void hsem_wait_void(uint32_t hsem, uint32_t procid)
{
	while (((1 << hsem) & hsem_get_status()) == 0){}
}

bool hsem_wait_bool(uint32_t hsem, uint32_t procid)
{
	uint32_t ir = hsem_get_status();
	if ((ir & (1 << hsem)) == 0)
	{
		return false;
	}
	else
	{
		return true;
	}
}

bool hsem_fast_lock(uint32_t id)
{
	if(HSEM->RLR[id] == 0)
	{
		return true;
	}
	return false;
}

bool hsem_lock(uint32_t hsem, uint32_t procid)
{
	HSEM->R[hsem] = (ACTIVE_CORE_ID << HSEM_R_COREID_Pos)
					| (procid << HSEM_R_PROCID_Pos)
					| (HSEM_R_LOCK);

	uint32_t readback = HSEM->R[hsem];
	return readback & HSEM_R_LOCK;
}

bool hsem_unlock(uint32_t hsem, uint32_t procid)
{
	HSEM->R[hsem] = (ACTIVE_CORE_ID << HSEM_R_COREID_Pos)
					| (procid << HSEM_R_PROCID_Pos);

	uint32_t readback = HSEM->R[hsem];
	return readback & HSEM_R_LOCK;
}

uint32_t hsem_get_status()
{
#ifdef CORE_CM7
	return HSEM->C1ISR;
#endif //CORE_CM7

#ifdef CORE_CM4
	return HSEM->C2ISR;
#endif //CORE_CM4

}

void hsem_clear_all(uint32_t key)
{
	HSEM->CR = (ACTIVE_CORE_ID << HSEM_CR_COREID_Pos)
				| (key << HSEM_CR_KEY_Pos);
}

void hsem_clear_int(uint32_t hsem)
{
#ifdef CORE_CM7
	HSEM->C1ICR = 1 << hsem;
#endif //CORE_CM7

#ifdef CORE_CM4
	HSEM->C2ICR = 1 << hsem;
#endif	//CORE_CM4
}

void hsem_clear_all_int()
{
#ifdef CORE_CM7
	HSEM->C1ICR = 0xFFFFFFFF;
#endif //CORE_CM7

#ifdef CORE_CM4
	HSEM->C2ICR = 0xFFFFFFFF;
#endif	//CORE_CM4
}

void hsem_enable_interrupt(uint32_t hsem_msk)
{
#ifdef CORE_CM7
	HSEM->C1IER |= hsem_msk;
#endif //CORE_CM7

#ifdef CORE_CM4
	HSEM->C2IER |= hsem_msk;
#endif	//CORE_CM4
}

void hsem_disable_interrupt(uint32_t hsem_msk)
{
#ifdef CORE_CM7
	HSEM->C1IER &= ~(hsem_msk);
#endif //CORE_CM7

#ifdef CORE_CM4
	HSEM->C2IER &= ~(hsem_msk);
#endif	//CORE_CM4
}

void hsem1_assign_int_handler(void (*func)())
{
	hsem1_interrupt_cb = func;
}

void hsem2_assign_int_handler(void (*func)())
{
	hsem2_interrupt_cb = func;
}

#ifdef CORE_CM7
void HSEM1_IRQHandler()
{
	if (hsem1_interrupt_cb != NULL)
	{
		hsem1_interrupt_cb();
	}
	hsem_clear_all_int();
}
#endif //CORE_CM7

#ifdef CORE_CM4
void HSEM2_IRQHandler()
{
	if (hsem2_interrupt_cb != NULL)
	{
		hsem2_interrupt_cb();
	}
	hsem_clear_all_int();
}
#endif //CORE_CM4

#endif	//USE_STM32_HSEM
