/*
 * iic_touch.h
 *
 *  Created on: Apr 16, 2024
 *      Author: awjpp
 */

#ifndef TOUCH_SCREEN_IIC_TOUCH_H_
#define TOUCH_SCREEN_IIC_TOUCH_H_


/**********		INCLUDES	**********/
#include "drivers/stm32_iic.h"
#include "drivers/stm32_io.h"
#include "stm32h745xx.h"
#include "stdbool.h"

/**********		DEFINES		**********/
#define TOUCH_USE_LVGL				1
#define TOUCH_SWAP_XY				1

#define I2C_INST					I2C4
#define I2C_SCL_PORT				GPIOD
#define I2C_SCL_PIN					GPIO_PIN12_Msk
#define I2C_SCL_ALT_FUNC			GPIO_AFR_AF4

#define I2C_SDA_PORT				GPIOD
#define I2C_SDA_PIN					GPIO_PIN13_Msk
#define I2C_SDA_ALT_FUNC			GPIO_AFR_AF4

#define TOUCH_RESET_PORT			GPIOB
#define TOUCH_RESET_PIN				GPIO_PIN12_Msk

#define TOUCH_INT_PORT				GPIOG
#define TOUCH_INT_PIN				GPIO_PIN2_Msk

#define TOUCH_SLAVE_ADDR			0x70

#define FT6x_TD_STATUS				0x02
#define FT6x_P1_XH					0x03
#define FT6x_P1_XL					0x04
#define FT6x_P1_YH					0x05
#define FT6x_P1_YL					0x06
#define FT6x_P1_WEIGHT				0x07
#define FT6x_P1_MISC				0x08
#define FT6x_G_MODE					0xA4

#define FT6x_G_MODE_POLLING_MODE	0x00

/**********		GLOBAL FUNCTION DECLARATIONS		**********/
void touch_init();
void touch_update_data();		//performs transaction with screen to get most recent touch data.
bool touch_is_pressed();		//returns true if the screen was being touched at last update.
uint16_t touch_get_x();			//returns the last touched x coordinate.
uint16_t touch_get_y();			//returns the last touched y coordinate.
uint8_t touch_get_pts();		//returns the number of touch points.


#endif /* TOUCH_SCREEN_IIC_TOUCH_H_ */
