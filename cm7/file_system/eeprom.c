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
/**********		EXTERNAL VARIABLE DEFINITIONS		**********/

/**********		STATIC VARIABLES		**********/

/**********		STATIC FUNCTION DECLRATIONS		**********/

/**********		STATIC FUNCTION DEFINITIONS		**********/

/**********		GLOBAL FUNCTION DEFINITIONS		**********/
bool eeprom_present()
{
	uint32_t mfg_data = 0;
	i2c_read(I2C4, EEPROM_HOST_CODE, EEPROM_IIC_ADDR, I2C_INTERNAL_ADDR_8_BIT, &mfg_data, 4, false);
	if (mfg_data != 0x00c8d000)
	{
		return false;
	}
	return true;
}

int8_t eeprom_status()
{
	return i2c_write(I2C4, EEPROM_IIC_ADDR, 0x0000, I2C_INTERNAL_ADDR_16_BIT, NULL, 0, true);
}

int8_t eeprom_write(uint16_t addr, void* data, uint32_t size)
{
	while (eeprom_status() != EEPROM_RDY) {}
	return i2c_write(I2C4, EEPROM_IIC_ADDR, (uint16_t)addr, I2C_INTERNAL_ADDR_16_BIT, (uint8_t*)data, size, true);
}

int8_t eeprom_read(void* data, uint16_t addr, uint32_t size)
{
	return i2c_read(I2C4, EEPROM_IIC_ADDR, addr, I2C_INTERNAL_ADDR_16_BIT, (uint8_t*)data, size, false);
}

