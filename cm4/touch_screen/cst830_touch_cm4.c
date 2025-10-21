

/**********     INCLUDES        **********/
#include "cst830_touch_cm4.h"

#include "drivers/stm32_io.h"		//TEMP - for debugging, using test led.

#include "app_shared_mem.h"

#include "FreeRTOS.h"

/**********		DEFINES		**********/

/**********		VARIABLE DEFINITIONS		**********/
typedef struct
{
	uint8_t touch_num;
	uint8_t touch1_xh;
	uint8_t touch1_xl;
	uint8_t touch1_yh;
	uint8_t touch1_yl;
}touch_info_t;

/**********		STATIC VARIABLES		**********/
static touch_info_t touch_info;
static uint32_t time_since_last_update_ms = 0;

/**********		STATIC FUNCTION DECLRATIONS		**********/


/**********		STATIC FUNCTION DEFINITIONS		**********/

/**********		GLOBAL FUNCTION DEFINITIONS		**********/
void cst830_read_data()
{
	i2c_read(I2C_INST, CST830_SLAVE_ADDR, CST830_TOUCH_NUM, (uint8_t*)&touch_info, 5);
}

bool cst830_is_pressed()
{
	if (cst830_get_pts() > 0)
	{
		return true;
	}
	return false;
}

uint16_t cst830_get_x()
{
#if TOUCH_SWAP_XY == 0

	uint16_t val = (touch_info.touch1_xh & 0x0F) << 8;
	val = val | touch_info.touch1_xl;
	return val;

#endif //TOUCH_SWAP_XY == 1

#if TOUCH_SWAP_XY == 1

	uint16_t val = (touch_info.touch1_yh & 0x0F) << 8;
	val = val | touch_info.touch1_yl;
	return val;

#endif //TOUCH_SWAP_XY == 1
}

uint16_t cst830_get_y()
{
#if TOUCH_SWAP_XY == 0

	uint16_t val = (touch_info.touch1_yh & 0x0F) << 8;
	val = val | touch_info.touch1_yl;
	return val;

#endif //TOUCH_SWAP_XY == 0

#if TOUCH_SWAP_XY == 1

	uint16_t val = (touch_info.touch1_xh & 0x0F) << 8;
	val = val | touch_info.touch1_xl;
	return val;

#endif //TOUCH_SWAP_XY == 1
}

uint8_t cst830_get_pts()
{
	return touch_info.touch_num & 0x0F;
}

void cst830_update()
{
	while (1)
	{
		cst830_read_data();
		shared_mem_set_cst830_touch_data(cst830_is_pressed(), cst830_get_x(), cst830_get_y());
		vTaskDelay(CST830_REFRESH_RATE);
	}
}
