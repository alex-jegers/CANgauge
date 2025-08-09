/**********     INCLUDES        **********/
#include "cst830_touch.h"
#include "drivers/stm32_sys_timer.h"

#if TOUCH_USE_LVGL == 1
#include "lvgl/lvgl.h"
#endif //TOUCH_USE_LVGL == 1

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
static uint32_t time_at_last_press = 0;
const static uint32_t time_between_press_int = 11;		//The time in ms between screen pressed interrupts if the screen is being held down.

/**********		STATIC FUNCTION DECLRATIONS		**********/
#if TOUCH_USE_LVGL == 1
static void touch_lvgl_read_cb(lv_indev_drv_t *indev, lv_indev_data_t *data);
static void touch_lvgl_add_indev();
#endif //TOUCH_USE_LVGL == 1
static void screen_pressed_int_handler();

/**********		STATIC FUNCTION DEFINITIONS		**********/
#if TOUCH_USE_LVGL == 1
static void touch_lvgl_read_cb(lv_indev_drv_t *indev, lv_indev_data_t *data)
{
	cst830_update_data();
	if (cst830_is_pressed())
	{
		data->point.x = cst830_get_x();
		data->point.y = cst830_get_y();
		data->state = LV_INDEV_STATE_PRESSED;
	}
	else
	{
		data->point.x = cst830_get_x();
		data->point.y = cst830_get_y();
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

static void screen_pressed_int_handler()
{
	time_at_last_press = timer_get_time_ms();
}

/**********		GLOBAL FUNCTION DEFINITIONS		**********/
void cst830_init()
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

	/*Put the screen in normal mode.*/
	const uint8_t work_mode_val = CST830_WORK_MODE_RAW;
	i2c_write(I2C_INST, CST830_SLAVE_ADDR, CST830_WORK_MODE, &work_mode_val, 1, true);

#if TOUCH_USE_LVGL == 1
	touch_lvgl_add_indev();
#endif //TOUCH_USE_LVGL == 1
}

void cst830_update_data()
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



