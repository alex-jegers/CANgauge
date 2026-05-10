/**********     INCLUDES        **********/
#include "system_mem.h"
#include "drivers/drivers.h"
#include <string.h>
/**********		DEFINES		**********/

/**********		EXTERNAL VARIABLE DEFINITIONS		**********/

/**********		STATIC VARIABLES		**********/
SYSTEM_MEM_REGION_EXTERN_RAM uint8_t prv_flash_buffer[SYSTEM_MEM_CONFIG_LENGTH_BYTES];
SYSTEM_MEM_REGION_EXTERN_RAM uint8_t system_mem_ram_fs[512 * 0x4000];

/**********		STATIC FUNCTION DECLRATIONS		**********/
void prv_save_nvm(void* dest, void* src, uint32_t size);
/**********		STATIC FUNCTION DEFINITIONS		**********/
void prv_save_nvm(void* dest, void* src, uint32_t size)
{
    /* Get the sector number. */
    uint32_t sector_number = ((uint32_t)dest & 0xFFFFF) / (uint32_t)0x20000;

    /* Bank start address. */
    uint32_t sector_saddr = (sector_number * 0x20000) + 0x08100000;

    /* Copy the bank into the RAM buffer. */
    memcpy((void*)prv_flash_buffer, (void*)sector_saddr, SYSTEM_MEM_CONFIG_LENGTH_BYTES);

    /* Get the data's offset from the start of the bank. */
    uint32_t offset = (uint32_t)dest - sector_saddr;

    /* Add that to the start address of the RAM buffer. */
    uint32_t write_addr_ram = (uint32_t)&prv_flash_buffer + offset;

    /*Copy the new data to that address in RAM. */
    memcpy((void*)write_addr_ram, (void*)src, size);

    /* Save the whole bank back to FLASH. */
    flash_b2_unlock();
    flash_b2_erase(sector_number);
    while (flash_b2_qw_status()) {}
    flash_b2_enable_write();
    for (uint32_t i = 0; i < SYSTEM_MEM_CONFIG_LENGTH_BYTES; i+= 0x20)
    {
        memcpy((void*)sector_saddr + i, (void*)prv_flash_buffer + i, 0x20);
        while (flash_b2_qw_status()) {}
    }
    flash_b2_disable_write();
    flash_b2_lock();
}

/**********		GLOBAL FUNCTION DEFINITIONS		**********/
void system_mem_save_nvm_u32(flash_uint32_t* p, uint32_t val)
{
	prv_save_nvm(p, &val, sizeof(uint32_t));
}

void system_mem_save_nvm_u8(flash_uint8_t* p, uint8_t val)
{
	prv_save_nvm(p, &val, sizeof(uint8_t));
}

void system_mem_save_nvm_void(flash_void_t* p, void* src, uint32_t size)
{
	prv_save_nvm(p, src, size);
}

uint8_t* sys_mem_get_ram_fs_ptr()
{
	return &system_mem_ram_fs;
}
