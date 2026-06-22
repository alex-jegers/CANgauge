

/**********     INCLUDES        **********/
#include "drivers/drivers.h"

#include <stdbool.h>
#include "touch_scr.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "event_groups.h"

/**********		DEFINES		**********/

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

#define CST820_DISAUTOSLEEP			0xFE
#define CST820_DISAUTOSLEEP_ON		0x01	//Disables auto sleep.
#define CST820_DISAUTOSLEEP_OFF		0x00	//Enables auto sleep.

#define CST830_REFRESH_PERIOD_MS			30

#define EVENT_BITS_TASK_STOPPED				(EventBits_t)0x01

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
static touch_info_t touch_info;
static bool prv_run = false;
static EventGroupHandle_t prv_event_group;
static void (*prv_scr_touched_cb)();

/**********		STATIC FUNCTION DECLRATIONS		**********/
static void prv_init();
static int8_t prv_read_data();								//performs transaction with screen to get most recent touch data.

/*
 * prv_task_update:
 *
 * desc: takes a pointer to a touch_info_t struct where it writes touch data to
 * every so often as specified by CST830_REFRESH_RATE.
 * */
static void prv_task_update(touch_info_t* p_touch_data);

/**********		STATIC FUNCTION DEFINITIONS		**********/
static void prv_init()
{
    /*Init GPIO.*/
	io_init();

	/*Configure reset pin and perform reset.*/
	io_set_pin_dir_out(TOUCH_RESET_PORT, TOUCH_RESET_PIN);
	io_pin_out_clr(TOUCH_RESET_PORT, TOUCH_RESET_PIN);
	vTaskDelay(500);
	io_pin_out_set(TOUCH_RESET_PORT, TOUCH_RESET_PIN);
	vTaskDelay(500);

	/*Set the interrupt pin as input.*/
	io_set_pin_dir_in(TOUCH_INT_PORT, TOUCH_INT_PIN);

	/*Configure the IO pins.*/
	io_set_output_type(I2C_SCL_PORT, I2C_SCL_PIN, IO_OUTPUT_TYPE_OPEN_DRAIN);
	io_set_output_type(I2C_SDA_PORT, I2C_SDA_PIN, IO_OUTPUT_TYPE_OPEN_DRAIN);
	io_set_pin_mux(I2C_SCL_PORT, I2C_SCL_PIN, I2C_SCL_ALT_FUNC);
	io_set_pin_mux(I2C_SDA_PORT, I2C_SDA_PIN, I2C_SDA_ALT_FUNC);

	/*Initialize the I2C.*/
	i2c_init(I2C_INST);
	i2c_set_clk_speed(I2C_INST, I2C_CLK_400K);
	i2c_disable_analog_filt(I2C_INST);
	i2c_enable_timeout_detection(I2C_INST);
	i2c_enable(I2C_INST);

	/*Put the screen in normal mode.*/
	const uint8_t work_mode_val = CST830_WORK_MODE_NORMAL;
	//i2c_write(I2C_INST, CST830_SLAVE_ADDR, CST830_WORK_MODE, &work_mode_val, 2, true);

	const uint8_t auto_sleep_val = CST820_DISAUTOSLEEP_ON;
	i2c_write(I2C_INST, CST830_SLAVE_ADDR, CST820_DISAUTOSLEEP, I2C_INTERNAL_ADDR_8_BIT, &auto_sleep_val, 1, true);
}

static int8_t prv_read_data()
{
	touch_info_raw_t data;
	int8_t status = i2c_read(I2C_INST, CST830_SLAVE_ADDR, CST830_TOUCH_NUM, I2C_INTERNAL_ADDR_8_BIT,
					(uint8_t*)&data, 5, false);

	if (status != I2C_EXIT_CODE_TC)
	{
		return status;
	}

	touch_info.touch_num = data.touch_num & 0x0F;

	/*Calculate the x position.*/
	touch_info.touch1_x = (data.touch1_xh & 0x0F) << 8;
	touch_info.touch1_x = touch_info.touch1_x | data.touch1_xl;

	/*Calculate the y position. */
	touch_info.touch1_y = (data.touch1_yh & 0x0F) << 8;
	touch_info.touch1_y = touch_info.touch1_y | data.touch1_yl;

	return status;
}

static void prv_task_update(touch_info_t* p_touch_data)
{
	/* Initialize the LCD screen. */
	prv_init();
	prv_run = true;
	while (p_touch_data == NULL) {}
	while (prv_run)
	{
		if (prv_read_data() != I2C_EXIT_CODE_TC)
		{
			i2c_bus_reset(I2C4);
		}
		*p_touch_data = touch_info;
		if (touch_info.touch_num > 0)
		{
			if (prv_scr_touched_cb != NULL)
			{
				prv_scr_touched_cb();
			}
		}
		vTaskDelay(CST830_REFRESH_PERIOD_MS);

	}
	/* Enable autosleep. */
	const uint8_t auto_sleep_val = CST820_DISAUTOSLEEP_OFF;
	i2c_write(I2C_INST, CST830_SLAVE_ADDR, CST820_DISAUTOSLEEP, I2C_INTERNAL_ADDR_8_BIT, &auto_sleep_val, 2, true);
	xEventGroupSetBits(prv_event_group, EVENT_BITS_TASK_STOPPED);
	vTaskDelete(NULL);
}

/**********		GLOBAL FUNCTION DEFINITIONS		**********/
void touch_scr_run(touch_info_t* p_touch_data)
{
	prv_event_group = xEventGroupCreate();
	xEventGroupClearBits(prv_event_group, EVENT_BITS_TASK_STOPPED);
	xTaskCreate((TaskFunction_t)prv_task_update, "TOUCH_SCR", 800 / 4, p_touch_data, 3, NULL);
}

bool touch_scr_stop(uint32_t block_time_ms)
{
	prv_run = false;
    
	/* If the event group is NULL, the task was never even created in the first place. */
    if (prv_event_group == NULL)
    {
    	return pdTRUE;
    }

	uint32_t rtn = xEventGroupWaitBits(prv_event_group, EVENT_BITS_TASK_STOPPED,
										pdFALSE, pdTRUE, block_time_ms);
	return rtn & EVENT_BITS_TASK_STOPPED;
}


void touch_scr_set_touched_cb(void (*func)())
{
	prv_scr_touched_cb = func;
}

void touch_scr_clear_touched_cb(void (*func)())
{
	prv_scr_touched_cb = NULL;
}
