/**********     INCLUDES        **********/
#include "system_mem.h"
#include "system_cm7.h"
#include "drivers/drivers.h"
#include <string.h>

#include "file_system/fatfs/ff.h"
#include "file_system/fatfs/diskio.h"
/**********		DEFINES		**********/

/**********		EXTERNAL VARIABLE DEFINITIONS		**********/

/**********		STATIC VARIABLES		**********/
SYS_MEM_REGION_EXTERN_RAM static uint8_t system_mem_ram_file_system[SECTOR_SIZE_RAM * NUM_SECTORS_RAM];

/**********		STATIC FUNCTION DECLRATIONS		**********/
SYS_MEM_REGION_RAM_EXE static void prv_memcpy(void* dest, void* src, size_t n_bytes);
/**********		STATIC FUNCTION DEFINITIONS		**********/
SYS_MEM_REGION_RAM_EXE static void prv_memcpy(void* dest, void* src, size_t n_bytes)
{
	for (uint32_t i = 0; i < n_bytes; i++)
	{
		*((uint8_t*)dest + i) = *((uint8_t*)src + i);
	}
}

/**********		GLOBAL FUNCTION DEFINITIONS		**********/
void sys_mem_init_file_systems()
{
	/* Create the file system. */
	static FATFS fs_ram, fs_eeprom;           // Filesystem object
	FRESULT res;        // API result code
	const MKFS_PARM params =
	{
			.fmt = FM_FAT,
			.n_fat = 1,
			.align = 0,
			.n_root = 0,
			.au_size = 0
	};

	/* Set up the working memory and make the file system in RAM. */
	SYS_MEM_REGION_EXTERN_RAM static uint8_t work_ram[4096];
	memset(work_ram, 0, 4096);
	res = f_mkfs("1:", &params, &work_ram, 4096);
	assert(res == FR_OK);

	/* Do the same for the EEPROM file system. */
	SYS_MEM_REGION_EXTERN_RAM static uint8_t work_eeprom[4096];
	memset(work_eeprom, 0, 4096);
	res = f_mkfs("0:", &params, &work_eeprom, 4096);
	assert(res == FR_OK);

	// Give a work area to each drive
	res = f_mount(&fs_ram, "1:", 0);
	if (res != FR_OK)
	assert(res == FR_OK);

	// Give a work area to each drive
	res = f_mount(&fs_eeprom, "0:", 0);
	if (res != FR_OK)
	assert(res == FR_OK);

	/* Create a directory called firmware in the RAM FS. */
	res = f_mkdir("1:Firmware");
	assert(res == FR_OK);
	res = f_mkdir("1:Other");
	assert(res == FR_OK);

	FIL file;
	res = f_open(&file, "1:/other/test", FA_WRITE | FA_CREATE_NEW);
	uint32_t bw = 0;
	res = f_write(&file, "Hello, World!\r\n", 15, &bw);
	assert( res == FR_OK );
	assert( bw == 15 );
	f_close(&file);

}

SYS_MEM_REGION_RAM_EXE void sys_mem_flash_write_sector(uint8_t sector, void* src)
{
    /* Bank start address. */
    uint32_t sector_saddr = (sector * 0x20000) + 0x08000000;

    /* Save the whole bank back to FLASH. */
    flash_b1_unlock();
    flash_b1_erase(sector);
    while (flash_b1_qw_status()) {}
    flash_b1_enable_write();
    for (uint32_t i = 0; i < 0x20000; i+= 0x20)
    {
    	prv_memcpy((uint8_t*)sector_saddr + i, (uint8_t*)src + i, 0x20);
        while (flash_b1_qw_status()) {}
    }
    flash_b1_disable_write();
    flash_b1_lock();
}

uint8_t* sys_mem_get_ram_fs_ptr()
{
	return &system_mem_ram_file_system;
}
