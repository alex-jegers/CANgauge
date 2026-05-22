
#ifndef _EEPROM_H_
#define _EEPROM_H_

#ifdef __cplusplus
extern "C" {
#endif

/**********     INCLUDES        **********/
#include "stdbool.h"
#include "stdint.h"
#include "stddef.h"
/**********     TYPEDEFS         **********/
typedef enum
{
	EEPROM_STS_OK,
	EEPROM_STS_I2C_ERR,
	EEPROM_STS_ERR
}eeprom_sts_t;
/**********     DEFINES      **********/

/**********     GLOBAL VARIABLE DECLRATIONS     **********/

/**********		GLOBAL FUNCTION DECLRATIONS		**********/
/**
 * eeprom_write:
 * 		desc: write data, data, or size, size, to eeprom address, addr.
 * 		returns: zero if an error occurred. Non-zero for success.
 */
eeprom_sts_t eeprom_write(uint32_t addr, void* data, uint32_t num_bytes);

/**
 * eeprom_read:
 * 		desc: reads size bytes from eeprom address, addr, into data.
 * 		returns: zero if an error occurred. Non-zero if the read was a success.
 */
eeprom_sts_t eeprom_read(void* data, uint32_t addr, uint32_t num_bytes);

/**
 * eeprom_probe:
 * 		returns: zero if there is no IIC eeprom present. Non-zero if there is an IIC
 * 			eeprom present.
 */
bool eeprom_present();

int8_t eeprom_status();



#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif  //_EEPROM_H_
