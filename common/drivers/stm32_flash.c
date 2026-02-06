/**********     INCLUDES        **********/
#include "stm32_flash.h"

/**********		DEFINES		**********/
#define FLASH_UNLOCK_KEY1           0x45670123
#define FLASH_UNLOCK_KEY2           0xCDEF89AB
#define FLASH_CR_PSIZE_8_BIT        0x00 << FLASH_CR_PSIZE_Pos
#define FLASH_CR_PSIZE_16_BIT       0x01 << FLASH_CR_PSIZE_Pos
#define FLASH_CR_PSIZE_32_BIT       0x02 << FLASH_CR_PSIZE_Pos
#define FLASH_CR_PSIZE_64_BIT       0x03 << FLASH_CR_PSIZE_Pos

/**********		EXTERNAL VARIABLE DEFINITIONS		**********/

/**********		STATIC VARIABLES		**********/

/**********		STATIC FUNCTION DECLRATIONS		**********/

/**********		STATIC FUNCTION DEFINITIONS		**********/

/**********		GLOBAL FUNCTION DEFINITIONS		**********/
bool flash_b2_unlock()
{
    /* Check to see if it's already locked, if it's unlocked, return.*/
    uint32_t lock = FLASH->CR2 & FLASH_CR_LOCK;
    if (lock == 0)
    {
        return true;
    }

    /* If it wasn't unlocked, unlock it.*/
    FLASH->KEYR2 = FLASH_UNLOCK_KEY1;
    FLASH->KEYR2 = FLASH_UNLOCK_KEY2;
    return true;
}

bool flash_b2_enable_write()
{
    FLASH->CR2 = FLASH_CR_PSIZE_64_BIT;
    FLASH->CR2 |= FLASH_CR_PG;
}

void flash_b2_disable_write()
{
    FLASH->CR2 &= ~(FLASH_CR_PG);
}

void flash_b2_lock()
{
    FLASH->CR2 |= FLASH_CR_LOCK;
}

bool flash_b2_erase(uint8_t sector)
{
    if (sector > 7)
    {
        return false;
    }

    FLASH->CR2 = FLASH_CR_PSIZE_64_BIT;

    FLASH->CR2 |= FLASH_CR_SER
                | (sector << FLASH_CR_SNB_Pos);
    FLASH->CR2 |= FLASH_CR_START;
    return true;
}

bool flash_b2_qw_status()
{
    return FLASH->SR2 & FLASH_SR_QW;
}

void flash_b2_force_write()
{
    FLASH->CR2 |= FLASH_CR_FW;
}