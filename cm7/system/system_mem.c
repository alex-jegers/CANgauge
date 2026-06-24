/**********     INCLUDES        **********/
#include "system_mem.h"
#include "system_cm7.h"
#include "drivers/drivers.h"
#include <string.h>

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
	const MKFS_PARM params_eeprom =
	{
			.fmt = FM_FAT,
			.n_fat = 1,
			.align = 0,
			.n_root = 16,
			.au_size = 0
	};

	/* Set up the working memory and make the file system in RAM. */
	SYS_MEM_REGION_EXTERN_RAM static uint8_t work_ram[4096];
	memset(work_ram, 0, 4096);
	res = f_mkfs("1:", &params, &work_ram, 4096);
	res = f_mount(&fs_ram, "1:", 0);

	/* Create a directory called firmware in the RAM FS. */
	res = f_mkdir("1:/Firmware");


	/* Setup the EEPROM File System. */
	/* Check if there's already a file system in EEPROM. */
	res = f_mount(&fs_eeprom, "0:", 0);
	FATFS* fs_ptr;
	uint32_t free_clusters;
	res = f_getfree("0:", &free_clusters, &fs_ptr);
	if (res != FR_OK)
	{
		/* Create a file system if there isnt one. */
		SYS_MEM_REGION_EXTERN_RAM static uint8_t work_eeprom[4096];	//TODO change to calloc.
		memset(work_eeprom, 0, 4096);
		res = f_mkfs("0:", &params_eeprom, &work_eeprom, 4096);
		assert(res == FR_OK);
	}
	/* Check if the config file is already there. */
	FIL temp;
	res = f_open(&temp, SYS_MEM_CONFIG_FILE_PATH, FA_READ);
	if (res != FR_OK)
	{
		res = f_open(&temp, SYS_MEM_CONFIG_FILE_PATH, FA_WRITE | FA_CREATE_NEW);
		assert(res == FR_OK || res == FR_EXIST);
	}
	f_close(&temp);
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

uint32_t sys_mem_get_config_data(const char* data_to_get, char* data_buf)
{
	uint32_t rtn_val = 0;
	/* Try to open the config file. */
	FIL config_file;
	FRESULT res;
	res = f_open(&config_file, SYS_MEM_CONFIG_FILE_PATH, FA_READ | FA_WRITE);
	if (res == FR_OK) //The file exists.
	{
		char* line = calloc(250, 1);
		char* line_copy = calloc(250, 1);
		while (f_eof(&config_file) == 0)
		{
			/* Read a line. */
			f_gets(line, 250, &config_file);
			strcpy(line_copy, line);

			/* Split it with ",". */
			char* split;		//Hold the strings from the config file.
			char* sv_ptr;		//For strtok_r.
			split = strtok_r(line_copy, ",", &sv_ptr);

			/* Check if it's a match. */
			if (strcmp(split, data_to_get) == 0)
			{
				/* Copy the line to data_buf. */
				uint32_t line_str_len = strlen(line);
				memcpy(data_buf, line, line_str_len);
				rtn_val = line_str_len;
				break;
			}
		}
		free(line);
		free(line_copy);
		f_close(&config_file);
	}
	if (rtn_val == 0)
	{
		//Do something if the file fails to open.
		sys_mem_create_default_config_file();
	}
	return rtn_val;
}

void sys_mem_set_config_data(char* data)
{
	FIL current_file;
	FIL new_file;
	FRESULT res;
	char* data_copy = calloc(250, 1);		//Make a copy of the data were writing to figure out what the header is.
	strcpy(data_copy, data);

	/* Split it with ",". */
	char* data_header;	//Hold the strings from the config file.
	char* sv_ptr;		//For strtok_r.a
	data_header = strtok_r(data_copy, ",", &sv_ptr);

	/* Figure out what the index of this header is, if it even exists. */
	uint32_t line_counter = 0;
	char* line = calloc(250, 1);
	res = f_open(&current_file, SYS_MEM_CONFIG_FILE_PATH, FA_READ | FA_WRITE);
	while (f_eof(&current_file) == 0)
	{
		/* Read a line. */
		f_gets(line, 250, &current_file);

		/* Split it with ",". */
		char* split;		//Hold the strings from the config file.
		char* sv_ptr;		//For strtok_r.
		split = strtok_r(line, ",", &sv_ptr);

		/* Check if it's a match. */
		if (strcmp(split, data_header) == 0)
		{
			break;
		}
		line_counter++;
	}

	uint32_t line_to_skip = line_counter;
	line_counter = 0;

	/* Now copy everything from the old file to a new file except for the line we're overwriting. */
	res = f_open(&new_file, "0:/temp", FA_WRITE | FA_CREATE_ALWAYS);
	f_lseek(&current_file, 0);
	while (f_eof(&current_file) == 0)
	{
		f_gets(line, 250, &current_file);
		uint32_t line_len = strlen(line);
		if (line_counter != line_to_skip && (line_len > 1))
		{
			f_puts(line, &new_file);
		}
		line_counter++;
	}
	/* Add the new data line to the end of the new file. */
	f_putc('\n', &new_file);
	f_puts(data, &new_file);

	f_close(&current_file);
	f_close(&new_file);
	f_unlink(SYS_MEM_CONFIG_FILE_PATH);				//Unlink the current config file.
	f_rename("0:/temp", SYS_MEM_CONFIG_FILE_PATH);	//Remane the temp file as the new config file.

	free(data_copy);
	free(line);
}

void sys_mem_create_default_config_file()
{
	FIL config_file;
	FRESULT res;
	f_unlink(SYS_MEM_CONFIG_FILE_PATH);		//Unlink the old one incase it's still there.
	res = f_open(&config_file, SYS_MEM_CONFIG_FILE_PATH, FA_CREATE_ALWAYS | FA_WRITE);
	const char* const config_str = "LAST GAUGES STATE,0,0,0,0,\nBRIGHTNESS,65535,\nPRESSURE UNITS,kPa,\nTEMPERATURE UNITS,C,\nLAST ERROR,NONE,\n";
	uint32_t len = strlen(config_str);
	uint32_t bw = 0;
	f_write(&config_file, config_str, (UINT)len, (UINT*)&bw);
	f_close(&config_file);
}

char* sys_mem_csv_split(char* str, uint32_t index)
{
	char* sv_ptr = NULL;
	char* split = NULL;

	split = strtok_r(str, ",", &sv_ptr);
	for (uint32_t i = 0; i < index; i++)
	{
		split = strtok_r(NULL, ",", &sv_ptr);
	}
	return split;
}
