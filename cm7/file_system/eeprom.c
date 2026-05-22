/**********     INCLUDES        **********/
#include "eeprom.h"
#include "drivers/drivers.h"
#include "file_system/fatfs/ff.h"
#include "file_system/fatfs/diskio.h"

/**********		DEFINES		**********/
#define EEPROM_IIC_ADDR			0xA0
#define EEPROM_HOST_CODE		0xF8
#define EEPROM_SECURITY_ADDR	0xB0
#define EEPROM_RDY				I2C_EXIT_CODE_TC
#define EEPROM_MAX_ADDR			0x1FFFF				//For a 1Mbit EEPROM this is the largest address that can be read/written from/to.
/**********		EXTERNAL VARIABLE DEFINITIONS		**********/

/**********		STATIC VARIABLES		**********/

/**********		STATIC FUNCTION DECLRATIONS		**********/

/**********		STATIC FUNCTION DEFINITIONS		**********/

/**********		GLOBAL FUNCTION DEFINITIONS		**********/
bool eeprom_present()
{
	uint32_t mfg_data = 0;
	i2c_read(I2C4, EEPROM_HOST_CODE, EEPROM_IIC_ADDR, I2C_INTERNAL_ADDR_8_BIT, &mfg_data, 4, false);
	if (mfg_data != 0x00d0d000)
	{
		return false;
	}
	return true;
}

int8_t eeprom_status()
{
	return i2c_write(I2C4, EEPROM_IIC_ADDR, 0x00000000, I2C_INTERNAL_ADDR_16_BIT, NULL, 0, true);
}

eeprom_sts_t eeprom_write(uint32_t addr, void* data, uint32_t num_bytes)
{
	if (addr > EEPROM_MAX_ADDR)
	{
		return EEPROM_STS_ERR;
	}
	while (eeprom_status() != EEPROM_RDY) {}

	/*Figure out the 17 bit address mapping to the EEPROM addresses. */
	uint8_t dev_addr = (addr & 0x010000) >> 15;
	dev_addr |= EEPROM_IIC_ADDR;
	uint16_t internal_addr = addr & 0xFFFF;

	i2c_exit_code_t sts = i2c_write(I2C4, dev_addr, internal_addr, I2C_INTERNAL_ADDR_16_BIT, (uint8_t*)data, num_bytes, true);
	eeprom_sts_t rtn;
	if (sts == I2C_EXIT_CODE_TC)
	{
		rtn = EEPROM_STS_OK;
	}
	else
	{
		rtn = EEPROM_STS_I2C_ERR;
	}
	return rtn;
}

eeprom_sts_t eeprom_read(void* data, uint32_t addr, uint32_t num_bytes)
{
	/*Figure out the 17 bit address mapping to the EEPROM addresses. */
	uint8_t dev_addr = (addr & 0x010000) >> 15;
	dev_addr |= EEPROM_IIC_ADDR;
	uint16_t internal_addr = addr & 0xFFFF;
	uint32_t num_transactions = num_bytes / 0xFF;		//I2C->CR2 register only allows for 255 bytes per I2C transaction.
	uint8_t leftover_bytes = num_bytes % 0xFF;
	if (leftover_bytes > 0) { num_transactions++; }

	eeprom_sts_t rtn;
	i2c_exit_code_t sts;
	while (num_transactions > 0)
	{
		uint32_t n = (num_transactions == 1) ? leftover_bytes : 255;
		sts = i2c_read(I2C4, dev_addr, internal_addr, I2C_INTERNAL_ADDR_16_BIT, (uint8_t*)data, n, false);
		if (sts != I2C_EXIT_CODE_TC)
		{
			break;
		}
		num_transactions--;
		internal_addr += n;
		data = (uint8_t*)data + n;
	}

	if (sts == I2C_EXIT_CODE_TC)
	{
		rtn = EEPROM_STS_OK;
	}
	else
	{
		rtn = EEPROM_STS_I2C_ERR;
	}
	return rtn;
}



