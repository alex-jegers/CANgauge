
#ifndef _TEMPLATE_H_
#define _TEMPLATE_H_

#ifdef __cplusplus
extern "C" {
#endif

/**********     INCLUDES        **********/
#include "stdbool.h"
#include "stdint.h"
#include "stddef.h"
/**********     TYPEDEFS         **********/

/**********     DEFINES      **********/

/**********     GLOBAL VARIABLE DECLRATIONS     **********/

/**********		GLOBAL FUNCTION DECLRATIONS		**********/
/**
 * eeprom_write:
 * 		desc: write data, data, or size, size, to eeprom address, addr.
 * 		returns: zero if an error occurred. Non-zero for success.
 */
int8_t eeprom_write(uint16_t addr, void* data, uint32_t size);

/**
 * eeprom_read:
 * 		desc: reads size bytes from eeprom address, addr, into data.
 * 		returns: zero if an error occurred. Non-zero if the read was a success.
 */
int8_t eeprom_read(void* data, uint16_t addr, uint32_t size);

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

#endif  //_TEMPLATE_H_
