/*
 * stm32_pwr.c
 *
 *  Created on: Feb 17, 2024
 *      Author: awjpp
 */



/**********     INCLUDES        **********/
#include "stm32_pwr.h"
#include "drivers/drivers.h"

/**********		DEFINES		**********/
#define prv_wait_for_vos_rdy()      while ((PWR->D3CR & PWR_D3CR_VOSRDY) == 0) {}   

/**********		EXTERNAL VARIABLE DEFINITIONS		**********/

/**********		STATIC VARIABLES		**********/

/**********		STATIC FUNCTION DECLRATIONS		**********/


/**********		STATIC FUNCTION DEFINITIONS		**********/


/**********		GLOBAL FUNCTION DEFINITIONS		**********/
void pwr_set_vos_mode(pwr_vos_mode_t vos_mode)
{
    /* If we're going to mode 0 (highest power) just execute the sequence. */
    if (vos_mode == PWR_VOS_MODE_SCALE_0)
    {
        PWR->D3CR = PWR_VOS_MODE_SCALE_1 << PWR_D3CR_VOS_Pos;	//Switch to VOS scale 1.
	    RCC->APB4ENR |= RCC_APB4ENR_SYSCFGEN;			        //Enable SYSCFG clock.
	    SYSCFG->PWRCR |= SYSCFG_PWRCR_ODEN;				        //Enable VOS0.
	    while ((PWR->D3CR & PWR_D3CR_VOSRDY) == 0) {}	        //Wait for VOSRDY to be set. 
        return;       
    }

    /* Check if we are currently in VOS0, if we are we have to go to
    VOS1 before doing anything else. */
    if (pwr_get_current_vos_mode() == PWR_VOS_MODE_SCALE_0)
    {
        /* Clear the overdrive bit. Now we're in VOS1. */
        SYSCFG->PWRCR &= ~(SYSCFG_PWRCR_ODEN);
    }

    if (vos_mode == PWR_VOS_MODE_SCALE_1)
    {
        PWR->D3CR = PWR_VOS_MODE_SCALE_1 << PWR_D3CR_VOS_Pos;
    }
    else if (vos_mode == PWR_VOS_MODE_SCALE_2)
    {
        /* Clear just the first bit in case we're changing from VOS1. */
        PWR->D3CR &= ~(0x1 << PWR_D3CR_VOS_Pos);
        PWR->D3CR |= PWR_VOS_MODE_SCALE_2 << PWR_D3CR_VOS_Pos;  //Set the VOS2 bit.
    }
    else if (vos_mode == PWR_VOS_MODE_SCALE_3)
    {
        PWR->D3CR &= ~(0x3 << PWR_D3CR_VOS_Pos);                //Clear both bits.
        PWR->D3CR |= PWR_VOS_MODE_SCALE_3 << PWR_D3CR_VOS_Pos;  //Set the VOS3 bit.
    }
    
    prv_wait_for_vos_rdy();
    return; 
}

pwr_vos_mode_t pwr_get_current_vos_mode()
{
    if ((SYSCFG->PWRCR & SYSCFG_PWRCR_ODEN) == SYSCFG_PWRCR_ODEN)
    {
        return PWR_VOS_MODE_SCALE_0;
    }
    else
    {
        //Returns the ACTVOS bits casted as pwr_vos_mode_t.
        return (pwr_vos_mode_t)((PWR->CSR1 >> 14) & 0x3);   
    }
}
