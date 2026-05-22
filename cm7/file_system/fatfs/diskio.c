/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2025        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"			/* Basic definitions of FatFs */
#include "diskio.h"		/* Declarations FatFs MAI */

/* Example: Declarations of the platform and disk functions in the project */
//#include "platform.h"
#include "file_system/eeprom.h"
#include "drivers/stm32_iic.h"
#include "system/system_mem.h"
#include <assert.h>

/* Example: Mapping of physical drive number for each drive */
#define DEV_EEPROM	0	/* Map FTL to physical drive 0 */
#define DEV_RAM		1

uint8_t* ram_fs_buf = NULL;

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat;
	int result;

	switch (pdrv)
	{
	case DEV_EEPROM :
		result = eeprom_status();
		if (result == I2C_EXIT_CODE_TC)
		{
			stat = RES_OK;
		}
		else
		{
			stat = RES_NOTRDY;
		}
		break;

	case DEV_RAM:
		if (ram_fs_buf != NULL)
		{
			stat = RES_OK;
		}
		else
		{
			stat = RES_NOTRDY;
		}
		break;
	}
	return stat;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat;
	int result;

	switch (pdrv) {
	case DEV_EEPROM :
		if (eeprom_present() == true)
		{
			stat = RES_OK;
		}
		break;
	case DEV_RAM:
		ram_fs_buf = sys_mem_get_ram_fs_ptr();

		stat = RES_OK;
		break;

	}
	return stat;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	DRESULT res;

	switch (pdrv)
	{
	case DEV_EEPROM :
		uint32_t phy_addr = sector * SECTOR_SIZE_EEPROM;
		uint32_t num_bytes = SECTOR_SIZE_EEPROM * count;

		eeprom_read(buff, phy_addr, num_bytes);

		res = RES_OK;
		break;
	case DEV_RAM:
		uint8_t* starting_addr = ram_fs_buf + (sector * SECTOR_SIZE_RAM);
		num_bytes = SECTOR_SIZE_RAM * count;
		for (uint32_t i = 0; i < num_bytes; i++)
		{
			*(buff + i) = *(starting_addr + i);
		}
		res = RES_OK;
	}

	return res;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	DRESULT res;
	int result;

	switch (pdrv)
	{
	case DEV_EEPROM :
		uint32_t phy_addr = sector * SECTOR_SIZE_EEPROM;
		uint32_t num_eeprom_blocks = count * 4;		//EEPROM block size is 128.

		for (uint32_t i = 0; i < num_eeprom_blocks; i++)
		{
			assert( eeprom_write(phy_addr + (i * 128), buff + (i * 128), 128) == EEPROM_STS_OK );
			while (eeprom_status() != I2C_EXIT_CODE_TC) {}
		}
		res = RES_OK;
		break;
	case DEV_RAM:
		uint8_t* starting_addr = ram_fs_buf + (sector * SECTOR_SIZE_RAM);
		uint32_t num_bytes = SECTOR_SIZE_RAM * count;
		for (uint32_t i = 0; i < num_bytes; i++)
		{
			*(starting_addr + i) = *(buff + i);
		}
		res = RES_OK;
		break;

	}

	return res;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res;
	int result;

	switch (pdrv)
	{
	case DEV_EEPROM :

		if (cmd == GET_SECTOR_COUNT)
		{
			const uint32_t sector_count = NUM_SECTORS_EEPROM;
			*(uint32_t*)buff = sector_count;
		}
		else if (cmd == GET_SECTOR_SIZE)
		{
			const uint32_t sector_size = SECTOR_SIZE_EEPROM;
			*(uint32_t*)buff = sector_size;
		}
		else if (cmd == GET_BLOCK_SIZE)
		{
			const uint32_t block_size = BLOCK_SIZE_EEPROM;
			*(uint32_t*)buff = block_size;
		}
		break;
	case DEV_RAM:
		if (cmd == GET_SECTOR_COUNT)
		{
			const uint32_t sector_count = NUM_SECTORS_RAM;
			*(uint32_t*)buff = sector_count;
		}
		else if (cmd == GET_SECTOR_SIZE)
		{
			const uint32_t sector_size = SECTOR_SIZE_RAM;
			*(uint32_t*)buff = sector_size;
		}
		else if (cmd == GET_BLOCK_SIZE)
		{
			const uint32_t block_size = BLOCK_SIZE_RAM;
			*(uint32_t*)buff = block_size;
		}
		break;
	}
	res = RES_OK;
	return res;
}

