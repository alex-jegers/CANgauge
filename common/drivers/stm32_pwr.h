/*
 * stm32_pwr.h
 *
 *  Created on: Feb 17, 2024
 *      Author: awjpp
 */

#ifndef INC_STM32_PWR_H_
#define INC_STM32_PWR_H_

#ifdef __cplusplus
extern "C" {
#endif




/**********     INCLUDES        **********/
#include "stm32h745xx.h"

/**********     TYPEDEFS         **********/
typedef enum
{
    PWR_VOS_MODE_SCALE_3 = 0x01,
    PWR_VOS_MODE_SCALE_2 = 0x02,
    PWR_VOS_MODE_SCALE_1 = 0x03,
    PWR_VOS_MODE_SCALE_0 = 0x04,
}pwr_vos_mode_t;

/**********     DEFINES      **********/

/**********     GLOBAL VARIABLE DECLRATIONS     **********/

/**********		GLOBAL FUNCTION DECLRATIONS		**********/
/**
 * pwr_set_vos_mode:
 * desc: performs the proper sequencing to put the CPU in the vos power
 *      mode specified in vos_mode. Does not change clock speeds, must 
 *      ensure clocks are within operating limits before changing VOS
 *      mode. See RM0399 table 62 for clock speed limits vs power mode.
 * params: 
 * vos_mode: SCALE_3 is lowest power.
 *      SCALE_0 is highest power.
 */
void pwr_set_vos_mode(pwr_vos_mode_t vos_mode);

/**
 * pwr_get_current_vos_mode:
 * returns: the vos power mode that the device is currently operating in. 
 */
pwr_vos_mode_t pwr_get_current_vos_mode();



#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* INC_STM32_PWR_H_ */
