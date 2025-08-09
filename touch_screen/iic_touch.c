/*
 * iic_touch.c
 *
 *  Created on: Apr 16, 2024
 *      Author: awjpp
 */
#ifdef CORE_CM7

/**********		INCLUDES	 **********/
#include "iic_touch.h"

#if TOUCH_USE_LVGL == 1
#include "lvgl/lvgl.h"
#endif //TOUCH_USE_LVGL == 1



/**********		VARIABLE DEFINITIONS		**********/
typedef struct
{
	uint8_t td_status;
	uint8_t touch1_xh;
	uint8_t touch1_xl;
	uint8_t touch1_yh;
	uint8_t touch1_yl;
}touch_info_t;


/**********		STATIC FUNCTION DECLARATIONS	**********/
#if TOUCH_USE_LVGL == 1
static void touch_lvgl_read_cb(lv_indev_drv_t *indev, lv_indev_data_t *data);
static void touch_lvgl_add_indev();
#endif //TOUCH_USE_LVGL == 1


/**********		STATIC VARIABLES	**********/
touch_info_t touch_info;


/**********		STATIC FUNCTION DEFINITIONS		**********/
#if TOUCH_USE_LVGL == 1
static void touch_lvgl_read_cb(lv_indev_drv_t *indev, lv_indev_data_t *data)
{
	touch_update_data();
	if (touch_is_pressed())
	{
		data->point.x = touch_get_x();
		data->point.y = touch_get_y();
		data->state = LV_INDEV_STATE_PRESSED;
	}
	else
	{
		data->point.x = touch_get_x();
		data->point.y = touch_get_y();
		data->state = LV_INDEV_STATE_RELEASED;
	}
}

static void touch_lvgl_add_indev()
{
	static lv_indev_drv_t indev_drv; 			/*Descriptor of an input device driver*/
	lv_indev_drv_init(&indev_drv);				/*Basic initialization*/
	indev_drv.type = LV_INDEV_TYPE_POINTER; 	/*The touchpad is pointer type device*/
	indev_drv.read_cb = touch_lvgl_read_cb;

	lv_indev_drv_register(&indev_drv);
}
#endif //TOUCH_USE_LVGL == 1


/**********		GLOBAL FUNCTION DEFINITIONS		**********/
void touch_init()
{
	/*Init GPIO.*/
	io_init();

	/*Configure reset pin and perform reset.*/
	io_set_pin_dir_out(TOUCH_RESET_PORT, TOUCH_RESET_PIN);
	io_pin_out_clr(TOUCH_RESET_PORT, TOUCH_RESET_PIN);
	timer_delay_ms(10);
	io_pin_out_set(TOUCH_RESET_PORT, TOUCH_RESET_PIN);
	timer_delay_ms(400);

	/*Set the interrupt pin as input.*/
	io_set_pin_dir_in(TOUCH_INT_PORT, TOUCH_INT_PIN);

	/*Configure the IO pins.*/
	io_set_output_type(I2C_SCL_PORT, I2C_SCL_PIN, IO_OUTPUT_TYPE_OPEN_DRAIN);
	io_set_output_type(I2C_SDA_PORT, I2C_SDA_PIN, IO_OUTPUT_TYPE_OPEN_DRAIN);
	io_set_pin_mux(I2C_SCL_PORT, I2C_SCL_PIN, I2C_SCL_ALT_FUNC);
	io_set_pin_mux(I2C_SDA_PORT, I2C_SDA_PIN, I2C_SDA_ALT_FUNC);

	/*Initialize the I2C.*/
	i2c_init_clk(I2C_INST);
	i2c_set_clk_speed(I2C_INST, I2C_CLK_400K);
	i2c_disable_analog_filt(I2C_INST);
	i2c_enable(I2C_INST);

	/*Put the device in polling mode.*/
	uint8_t g_mode_data = FT6x_G_MODE_POLLING_MODE;
	i2c_write(I2C_INST, TOUCH_SLAVE_ADDR, FT6x_G_MODE, &g_mode_data, 2, true);

#if TOUCH_USE_LVGL == 1
	touch_lvgl_add_indev();
#endif //TOUCH_USE_LVGL == 1
}

void touch_update_data()
{
	if (!(io_read_pin(TOUCH_INT_PORT, TOUCH_INT_PIN)))
	{
		i2c_read(I2C_INST, TOUCH_SLAVE_ADDR, FT6x_TD_STATUS, (uint8_t*)&touch_info, 5);
		return;
	}
}

bool touch_is_pressed()
{
	if (touch_get_pts() > 0)
	{
		return true;
	}
	return false;
}

uint16_t touch_get_x()
{
#if TOUCH_SWAP_XY == 0

	uint16_t val = (touch_info.touch1_xh & 0xF) << 8;
	val = val | touch_info.touch1_xl;
	return val;

#endif //TOUCH_SWAP_XY == 1

#if TOUCH_SWAP_XY == 1

	uint16_t val = (touch_info.touch1_yh & 0xF) << 8;
	val = val | touch_info.touch1_yl;
	return val;

#endif //TOUCH_SWAP_XY == 1
}

uint16_t touch_get_y()
{
#if TOUCH_SWAP_XY == 0

	uint16_t val = (touch_info.touch1_yh & 0xF) << 8;
	val = val | touch_info.touch1_yl;
	return val;

#endif //TOUCH_SWAP_XY == 0

#if TOUCH_SWAP_XY == 1

	uint16_t val = (touch_info.touch1_xh & 0xF) << 8;
	val = val | touch_info.touch1_xl;
	return val;

#endif //TOUCH_SWAP_XY == 1
}

uint8_t touch_get_pts()
{
	if (io_read_pin(TOUCH_INT_PORT, TOUCH_INT_PIN))
	{
		return 0;
	}
	return touch_info.td_status & 0xF;
}

#endif //CORE_CM7
