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

/* Example: Mapping of physical drive number for each drive */
#define DEV_EEPROM	0	/* Map FTL to physical drive 0 */



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

		// translate the reslut code here

		return stat;

	}
	return STA_NOINIT;
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
			return RES_OK;
		}

	}
	return STA_NOINIT;
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
	int result;

	switch (pdrv) {
	case DEV_EEPROM :
		uint32_t phy_addr = sector * 512;
		uint32_t num_bytes = 512 * count;	//512 bytes per sector.

		for (uint32_t i = 0; i < num_bytes; i++)
		{
			eeprom_read(buff + (i * 128), phy_addr + (i * 128), 128);
		}
		res = RES_OK;
		return res;
	}

	return RES_PARERR;
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
		uint32_t phy_addr = sector * 512;
		uint32_t num_eeprom_blocks = count * 4;		//EEPROM block size is 128.

		for (uint32_t i = 0; i < num_eeprom_blocks; i++)
		{
			eeprom_write(phy_addr + (i * 128), buff + (i * 128), 128);
			while (eeprom_status() != I2C_EXIT_CODE_TC) {}
		}
		res = RES_OK;
		return res;
	}

	return RES_PARERR;
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

	switch (pdrv) {
	case DEV_EEPROM :

		if (cmd == GET_SECTOR_COUNT)
		{
			const uint32_t sector_count = 1000;
			*(uint32_t*)buff = sector_count;
		}
		else if (cmd == GET_SECTOR_SIZE)
		{
			const uint32_t sector_size = 512;
			*(uint32_t*)buff = sector_size;
		}
		else if (cmd == GET_BLOCK_SIZE)
		{
			const uint32_t block_size = 512;
			*(uint32_t*)buff = block_size;
		}

		res = RES_OK;
		return res;
	}

	return RES_PARERR;
}

