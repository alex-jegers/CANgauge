

/**********     INCLUDES        **********/
#include "cst830_touch_cm7.h"

#include "drivers/stm32_io.h"		//TEMP - for debugging, using test led.

#include "app_shared_mem.h"

#include "FreeRTOS.h"
#include "task.h"

#include "lvgl/lvgl.h"


/**********		DEFINES		**********/

/**********		VARIABLE DEFINITIONS		**********/

/**********		STATIC VARIABLES		**********/

/**********		STATIC FUNCTION DECLRATIONS		**********/
static void touch_lvgl_read_cb(lv_indev_t *indev, lv_indev_data_t *data);
static void touch_lvgl_add_indev();



/**********		STATIC FUNCTION DEFINITIONS		**********/
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



/**********		GLOBAL FUNCTION DEFINITIONS		**********/
void cst830_init()
{
    /*Init GPIO.*/
	io_init();

	/*Configure reset pin and perform reset.*/
	//TODO: Make sure the delays are happening.
	io_set_pin_dir_out(TOUCH_RESET_PORT, TOUCH_RESET_PIN);
	io_pin_out_clr(TOUCH_RESET_PORT, TOUCH_RESET_PIN);
	vTaskDelay(10);
	io_pin_out_set(TOUCH_RESET_PORT, TOUCH_RESET_PIN);
	vTaskDelay(400);

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

	touch_lvgl_add_indev();
}

