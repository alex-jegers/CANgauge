
#ifndef _CST830_TOUCH_H_
#define _CST830_TOUCH_H_

#ifdef __cplusplus
extern "C" {
#endif

/**********     INCLUDES        **********/
#include "drivers/stm32_iic.h"
#include "drivers/stm32_io.h"
#include "stm32h745xx.h"
#include "stdbool.h"

/**********     DEFINES      **********/
#define TOUCH_USE_LVGL				1
#define TOUCH_SWAP_XY				0

#define I2C_INST					I2C4
#define I2C_SCL_PORT				GPIOD
#define I2C_SCL_PIN					GPIO_PIN12_Msk
#define I2C_SCL_ALT_FUNC			GPIO_AFR_AF4

#define I2C_SDA_PORT				GPIOD
#define I2C_SDA_PIN					GPIO_PIN13_Msk
#define I2C_SDA_ALT_FUNC			GPIO_AFR_AF4

#define TOUCH_RESET_PORT			GPIOI
#define TOUCH_RESET_PIN				GPIO_PIN7_Msk

#define TOUCH_INT_PORT				GPIOI
#define TOUCH_INT_PIN				GPIO_PIN6_Msk

#define CST830_SLAVE_ADDR           0x2A

#define CST830_FW_VER_H             0xA7
#define CST830_FW_VER_L             0xA8
#define CST830_WORK_MODE			0x00
#define CST830_WORK_MODE_NORMAL		0x00
#define CST830_WORK_MODE_IDAC		0x04
#define CST830_WORK_MODE_POS		0xE0
#define CST830_WORK_MODE_RAW		0x06
#define CST830_WORK_MODE_SIG		0x07

#define CST830_TOUCH_NUM			0x02

#define CST830_TOUCH1_XH			0x03
#define CST830_TOUCH1_XL			0x04
#define CST830_TOUCH1_XH_PTS_Msk	0x0F
#define CST830_TOUCH1_XL_PTS_Msk	0xFF

/**********     GLOBAL VARIABLE DECLRATIONS     **********/

/**********		GLOBAL FUNCTION DECLRATIONS		**********/
void cst830_init();
void cst830_update_data();		//performs transaction with screen to get most recent touch data.
bool cst830_is_pressed();		//returns true if the screen was being touched at last update.
uint16_t cst830_get_x();		//returns the last touched x coordinate.
uint16_t cst830_get_y();		//returns the last touched y coordinate.
uint8_t cst830_get_pts();		//returns the number of touch points.



#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif  //_CST830_TOUCH_H_
