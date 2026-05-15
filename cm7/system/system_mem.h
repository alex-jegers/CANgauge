
#ifndef _SYSTEM_MEM_H_
#define _SYSTEM_MEM_H_

#ifdef __cplusplus
extern "C" {
#endif

/**********     INCLUDES        **********/
#include "stm32h745xx.h"

/**********     DEFINES      **********/
#define SYSTEM_MEM_REGION_EXTERN_RAM			__attribute__((__section__(".ext_mem_ram")))
#define SYSTEM_MEM_REGION_NVM                   __attribute__((__section__(".config")))
#define SYSTEM_MEM_REGION_RAM_D1				__attribute__((__section__(".ram_d1")))
#define SYSTEM_MEM_REGION_RAM_EXE				__attribute__((__section__(".itcm")))

#define SYSTEM_MEM_CONFIG_LENGTH_BYTES			0x100

/**********     GLOBAL VARIABLE DECLRATIONS     **********/
typedef uint32_t flash_uint32_t;
typedef uint8_t flash_uint8_t;
typedef void* flash_void_t;

/**********		GLOBAL FUNCTION DECLRATIONS		**********/
/**
 * sys_mem_init_file_systems
 * 		desc: initializes a filesystem in RAM and EEPROM. RAM
 * 			is used for firmware updates, EEPROM is used for data logging
 * 			and a general save system.
 */
void sys_mem_init_file_systems();

/**
 * sys_mem_get_ram_fs_ptr:
 * 		desc: returns a pointer of the physical address to the start of the file
 * 			system in RAM. This is used to calculate other physical addresses
 * 			when only the block address is supplied during USB communication
 * 			and what not.
 */
uint8_t* sys_mem_get_ram_fs_ptr();


void system_mem_save_nvm_u32(flash_uint32_t* p, uint32_t val);
void system_mem_save_nvm_u8(flash_uint8_t* p, uint8_t val);
void system_mem_save_nvm_void(flash_void_t* p, void* src, uint32_t size);








#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif  //_SYSTEM_MEM_H_
