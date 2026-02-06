
#ifndef _STM32_FLASH_H_
#define _STM32_FLASH_H_

#ifdef __cplusplus
extern "C" {
#endif

/**********     INCLUDES        **********/
#include "stm32h745xx.h"
#include <stdbool.h>
/**********     DEFINES      **********/

/**********     GLOBAL VARIABLE DECLRATIONS     **********/

/**********		GLOBAL FUNCTION DECLRATIONS		**********/
bool flash_b2_unlock();
bool flash_b2_enable_write();
void flash_b2_disable_write();
void flash_b2_lock();

/**
 * flash_b2_erase:
 * desc: erase (set to 1) a sector (128kb section) of flash bank 2.
 * params:
 * sector: number 0 thru 7 corresponding with the sector to erase.
 * returns:
 * false if sector is greater than 7.
 */
bool flash_b2_erase(uint8_t sector);

/**
 * flash_b2_qw_status:
 * returns:
 * true: if the QW bit in flash_sr2 is set.
 * false: if the QW bit in flash_sr2 is not set.
 */
bool flash_b2_qw_status();
void flash_b2_force_write();



#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif  //_STM32_FLASH_H_