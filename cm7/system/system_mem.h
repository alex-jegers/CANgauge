
#ifndef _SYSTEM_MEM_H_
#define _SYSTEM_MEM_H_

#ifdef __cplusplus
extern "C" {
#endif

/**********     INCLUDES        **********/
#include "stm32h745xx.h"

#include "file_system/fatfs/ff.h"
#include "file_system/fatfs/diskio.h"

/**********     DEFINES      **********/
#define SYS_MEM_REGION_EXTERN_RAM			__attribute__((__section__(".ext_mem_ram")))
#define SYS_MEM_REGION_RAM_D1				__attribute__((__section__(".ram_d1")))
#define SYS_MEM_REGION_RAM_EXE				__attribute__((__section__(".itcm"), long_call))

#define SYSTEM_MEM_CONFIG_LENGTH_BYTES			0x100

#define SECTOR_SIZE_RAM			512
#define BLOCK_SIZE_RAM			1
#define NUM_SECTORS_RAM			0x2000

#define SECTOR_SIZE_EEPROM		512
#define BLOCK_SIZE_EEPROM		1
#define NUM_SECTORS_EEPROM		0x1FFFF / SECTOR_SIZE_EEPROM

#define SYS_MEM_CONFIG_FILE_PATH		"0:/System Data.txt"

/**********     GLOBAL VARIABLE DECLRATIONS     **********/

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

/**
 * sys_mem_flash_write_sector
 * 		desc: write to an area in flash.
 * 		params:
 * 			p: the sector number.
 * 			src: pointer to the data to write into the sector, must be 0x20000 bytes.
 */
void sys_mem_flash_write_sector(uint8_t sector, void* src);

/**
 * sys_mem_get_config_data:
 * 		desc: gets a line of data from the config file.
 * 		params:
 * 			data_to_get: a pointer to a string of the header of the data wanted (i.e. "LAST GAUGES STATE").
 * 			data_buf: a buffer where the line will be written to.
 * 		returns: the number of bytes written to data_buf.
 */
uint32_t sys_mem_get_config_data(const char* data_to_get, char* data_buf);

/**
 * sys_mem_set_config_data:
 * 		desc: overwrites new data to the config file.
 * 		params:
 * 			data: the line of data to write (i.e. "LAST GAUGES STATE,0,0,0,0).
 */
void sys_mem_set_config_data(char* data);

void sys_mem_create_default_config_file();

/**
 * sys_mem_csv_split	TODO:find a better file to put this.
 * 		desc: returns an element (index) from a string of comma separated values.
 */
char* sys_mem_csv_split(char* str, uint32_t index);





#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif  //_SYSTEM_MEM_H_
