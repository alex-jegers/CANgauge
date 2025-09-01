

/**********     INCLUDES        **********/
#include "cst830_touch.h"
#include "drivers/stm32_sys_timer.h"
#include "drivers/stm32_exti.h"

#include "application/app_shared_mem.h"

#if TOUCH_USE_LVGL == 1
#include "lvgl/lvgl.h"
#endif //TOUCH_USE_LVGL == 1

/**********		DEFINES		**********/
#define CST830_REFRESH_RATE				30

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
#if TOUCH_USE_LVGL == 1
static void touch_lvgl_read_cb(lv_indev_t *indev, lv_indev_data_t *data);
static void touch_lvgl_add_indev();
#endif //TOUCH_USE_LVGL == 1


/**********		STATIC FUNCTION DEFINITIONS		**********/
#if TOUCH_USE_LVGL == 1
static void touch_lvgl_read_cb(lv_indev_t *indev, lv_indev_data_t *data)
{
	data->point.x = shared_mem_get_cst830_pos_x();
	data->point.y = shared_mem_get_cst830_pos_y();
	data->state = shared_mem_get_cst830_is_pressed();
}

static void touch_lvgl_add_indev()
{
	lv_indev_t * indev = lv_indev_create();
	lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
	lv_indev_set_read_cb(indev, touch_lvgl_read_cb);

}
#endif //TOUCH_USE_LVGL == 1


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

void cst830_timer_tasks()
{
	if (time_since_last_update_ms >= CST830_REFRESH_RATE)
	{
		cst830_update_data();
		shared_mem_set_cst830_touch_data(cst830_is_pressed(), cst830_get_x(), cst830_get_y());
	}
}
void cst830_tick_inc(uint32_t ms)
{
	time_since_last_update_ms++;
}
