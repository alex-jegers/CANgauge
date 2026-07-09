
#ifndef _STM32_FLASH_H_
#define _STM32_FLASH_H_

#ifdef __cplusplus
extern "C" {
#endif

/**********     INCLUDES        **********/
#include "stm32h745xx.h"
#include <stdbool.h>
#include "system/system_mem.h"
/**********     DEFINES      **********/

/**********     GLOBAL VARIABLE DECLRATIONS     **********/

/**********		GLOBAL FUNCTION DECLRATIONS		**********/
SYS_MEM_REGION_RAM_EXE bool flash_b2_unlock();
SYS_MEM_REGION_RAM_EXE void flash_b2_enable_write();
SYS_MEM_REGION_RAM_EXE void flash_b2_disable_write();
SYS_MEM_REGION_RAM_EXE void flash_b2_lock();

/**
 * flash_b2_erase:
 * desc: erase (set to 1) a sector (128kb section) of flash bank 2.
 * params:
 * sector: number 0 thru 7 corresponding with the sector to erase.
 * returns:
 * false if sector is greater than 7.
 */
SYS_MEM_REGION_RAM_EXE  bool flash_b2_erase(uint8_t sector);

/**
 * flash_b2_qw_status:
 * returns:
 * true: if the QW bit in flash_sr2 is set.
 * false: if the QW bit in flash_sr2 is not set.
 */
SYS_MEM_REGION_RAM_EXE bool flash_b2_qw_status();
SYS_MEM_REGION_RAM_EXE void flash_b2_force_write();

SYS_MEM_REGION_RAM_EXE bool flash_b1_unlock();
SYS_MEM_REGION_RAM_EXE void flash_b1_enable_write();
SYS_MEM_REGION_RAM_EXE void flash_b1_disable_write();
SYS_MEM_REGION_RAM_EXE void flash_b1_lock();

/**
 * flash_b1_erase:
 * desc: erase (set to 1) a sector (128kb section) of flash bank 1.
 * params:
 * sector: number 0 thru 7 corresponding with the sector to erase.
 * returns:
 * false if sector is greater than 7.
 */
SYS_MEM_REGION_RAM_EXE bool flash_b1_erase(uint8_t sector);

/**
 * flash_b1_qw_status:
 * returns:
 * true: if the QW bit in flash_sr1 is set.
 * false: if the QW bit in flash_sr1 is not set.
 */
SYS_MEM_REGION_RAM_EXE bool flash_b1_qw_status();
SYS_MEM_REGION_RAM_EXE void flash_b1_force_write();



#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif  //_STM32_FLASH_H_
