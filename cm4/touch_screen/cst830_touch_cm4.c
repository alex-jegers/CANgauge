

/**********     INCLUDES        **********/
#include <shared_mem.h>
#include "cst830_touch_cm4.h"

#include "drivers/stm32_io.h"

#include "FreeRTOS.h"
#include "queue.h"

#include <assert.h>

/**********		DEFINES		**********/

/**********		VARIABLE DEFINITIONS		**********/
typedef struct
{
	uint8_t touch_num;
	uint8_t touch1_xh;
	uint8_t touch1_xl;
	uint8_t touch1_yh;
	uint8_t touch1_yl;
}touch_info_raw_t;

/**********		STATIC VARIABLES		**********/
touch_info_t touch_info;
static uint32_t time_since_last_update_ms = 0;
static uint8_t iic_addr = 0;

/**********		STATIC FUNCTION DECLRATIONS		**********/
static void prv_init();

/**********		STATIC FUNCTION DEFINITIONS		**********/
static void prv_init()
{
    /*Init GPIO.*/
	io_init();

	/*Configure reset pin and perform reset.*/
	//TODO: Make sure the delays are happening.
	io_set_pin_dir_out(TOUCH_RESET_PORT, TOUCH_RESET_PIN);
	io_pin_out_clr(TOUCH_RESET_PORT, TOUCH_RESET_PIN);
	vTaskDelay(1000);
	io_pin_out_set(TOUCH_RESET_PORT, TOUCH_RESET_PIN);
	vTaskDelay(1000);

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
	i2c_enable_timeout_detection(I2C_INST);
	i2c_enable(I2C_INST);

	/*Put the screen in normal mode.*/
	const uint8_t work_mode_val = CST830_WORK_MODE_NORMAL;
	//i2c_write(I2C_INST, CST830_SLAVE_ADDR, CST830_WORK_MODE, &work_mode_val, 2, true);

	const uint8_t auto_sleep_val = CST820_DISAUTOSLEEP_ON;
	i2c_write(I2C_INST, CST830_SLAVE_ADDR, CST820_DISAUTOSLEEP, &auto_sleep_val, 2, true);
}

/**********		GLOBAL FUNCTION DEFINITIONS		**********/
void cst830_read_data()
{
	touch_info_raw_t data;
	i2c_read(I2C_INST, CST830_SLAVE_ADDR, CST830_TOUCH_NUM, (uint8_t*)&data, 5);


	touch_info.touch_num = data.touch_num & 0x0F;

	/*Calculate the x position.*/
	touch_info.touch1_x = (data.touch1_xh & 0x0F) << 8;
	touch_info.touch1_x = touch_info.touch1_x | data.touch1_xl;

	/*Calculate the y position. */
	touch_info.touch1_y = (data.touch1_yh & 0x0F) << 8;
	touch_info.touch1_y = touch_info.touch1_y | data.touch1_yl;
}

void cst830_task_update(touch_info_t* p_touch_data)
{
	/* Initialize the LCD screen. */
	prv_init();
	while (p_touch_data == NULL) {}
	while (1)
	{
		cst830_read_data();
		*p_touch_data = touch_info;
		vTaskDelay(CST830_REFRESH_RATE);
	}
}
