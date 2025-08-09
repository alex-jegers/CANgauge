/*
 * stm32_lcd.c
 *
 *  Created on: Feb 17, 2024
 *      Author: awjpp
 */

#ifdef CORE_CM7

#include "stm32_lcd.h"
#include "stm32_rcc.h"		//For enabling PLL clock.
#include "stm32_io.h"		//For controlling IO related to the lcd.
#include "stm32_dma2d.h"	//For frame buffer control.
#include "stm32_timer.h"	//For lcd backlight PWM.
#include "stm32_spi.h"		//For initializing the ST7701 driver used on CANgauge config.

/*******STATIC VARIABLES************/
static lv_disp_drv_t* lcd_flush_rdy_disp_drv;

/*******	STATIC FUNCTION DECLARATIONS	************/
static void lcd_lvgl_disp_flush(lv_disp_drv_t * disp_drv,
								const lv_area_t * area, lv_color_t * color_p);
static void lcd_lvgl_flush_complete();
static void disp_clean_dcache(lv_disp_drv_t *drv);
static void lcd_st7701_init();
static void lcd_st7701_send_cmd(uint8_t cmd, bool hold_ss_low);
static void lcd_st7701_send_param(uint8_t param, bool hold_ss_low);
static void lcd_st7701_read_param(uint32_t* data, bool hold_ss_low);
static bool lcd_st7701_check_id();
static uint8_t lcd_st7701_self_diag();

/*TODO: Rename these and combine into 1 once i figure out whats broken*/
static void lcd_st7701_my_spi_config();
static void lcd_st7701_adafruit_spi_config();

/*******		STATIC FUNCTION DEFINITIONS		********/
static void lcd_lvgl_disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
    uint32_t start_addr = (area->y1 * LTDC_SCREEN_SIZE_X_px * LTDC_BYTES_PER_PIXEL) + (area->x1 * LTDC_BYTES_PER_PIXEL);
    start_addr += LTDC_DISP_BUFFER_ADDR;

    uint32_t x_size = lv_area_get_width(area);
    uint32_t y_size = lv_area_get_height(area);

	dma2d_start_copy((void*)color_p, (void*)start_addr, x_size, y_size);
}

static void lcd_lvgl_flush_complete()
{
	lv_disp_flush_ready(lcd_flush_rdy_disp_drv);
}

static void disp_clean_dcache(lv_disp_drv_t *drv)
{
	SCB_CleanInvalidateDCache();
}

static void lcd_st7701_init()
{
#ifdef TARGET_HARDWARE_CANGAUGE

	//Hardware reset.
	io_set_pin_dir_out(LTDC_RESET_io);
	io_pin_out_clr(LTDC_RESET_io);
	timer_delay_ms(10);
	io_pin_out_set(LTDC_RESET_io);
	timer_delay_ms(200);

	//Enable SPI.
	spi4_enable_clocks();
	spi4_enable_io();
	spi_set_sck_div(SPI4, SPI_SCK_DIV_256);
	spi_init(SPI4);
	spi_enable(SPI4);

	lcd_st7701_adafruit_spi_config();

#endif //TARGET_HARDWARE_CANGAUGE
}

static void lcd_st7701_send_cmd(uint8_t cmd, bool hold_ss_low)
{
#ifdef TARGET_HARDWARE_CANGAUGE
	uint16_t full_cmd = 0x0000;
	full_cmd |= cmd;
	spi_tx(SPI4, full_cmd, 9, hold_ss_low);
#endif
}

static void lcd_st7701_send_param(uint8_t param, bool hold_ss_low)
{
#ifdef TARGET_HARDWARE_CANGAUGE
	uint16_t full_param = 0x0100;
	full_param |= param;
	spi_tx(SPI4, full_param, 9, hold_ss_low);
#endif
}

static void lcd_st7701_read_param(uint32_t* data, bool hold_ss_low)
{
#ifdef TARGET_HARDWARE_CANGAUGE
	spi_rx(SPI4, data, 8, hold_ss_low);
#endif
}

static bool lcd_st7701_check_id()
{
#ifdef TARGET_HARDWARE_CANGAUGE
	static uint32_t full_id, id1, id2, id3 = 0;
	spi_tx(SPI4, ST7701_RDDID_CMD, 9, true);
	spi_rx(SPI4, &id1, 8, true);
	spi_rx(SPI4, &id2, 8, true);
	spi_rx(SPI4, &id3, 8, false);

	full_id = id1 | (id2 << 8) | (id3 << 16);

	if (full_id != 0x00ffff7f)
	{
		return false;
	}
	return true;
#endif	//TARGET_HARDWARE_CANGAUGE
}

static uint8_t lcd_st7701_self_diag()
{
#ifdef TARGET_HARDWARE_CANGAUGE
	static uint32_t self_diag;
	lcd_st7701_send_cmd(ST7701_RDDSDR_CMD, true);
	lcd_st7701_read_param(&self_diag, false);
	self_diag &= ST7701_RDDSDR_PARAM;
	return self_diag;
#endif
}

static void lcd_st7701_my_spi_config()
{
	uint8_t self_diag = lcd_st7701_self_diag();
	if (self_diag != 0)
	{
		while (1)
		{
			io_test_led_on();
			timer_delay_ms(500);
			io_test_led_off();
			timer_delay_ms(500);
		}
	}

	//Software reset.
	lcd_st7701_send_cmd(ST7701_SWRESET_CMD, false);		//SW reset.
	timer_delay_ms(200);

	//Read and validate the LCD ID.
	if (!lcd_st7701_check_id())
	{
		//TODO: Call error handler here.
		while (1)
		{
			io_test_led_on();
			timer_delay_ms(500);
			io_test_led_off();
			timer_delay_ms(500);
		}
	}

	/*Sleep out.*/
	lcd_st7701_send_cmd(ST7701_SLPOUT_CMD, false);

	/*Display on.*/
	lcd_st7701_send_cmd(ST7701_DISPON_CMD, false);

	//Enable BK0 function of command 2.
	lcd_st7701_send_cmd(ST7701_CND2BKxSEL_CMD, true);
	lcd_st7701_send_param(ST7701_CND2BKxSEL_PARAM1, true);
	lcd_st7701_send_param(ST7701_CND2BKxSEL_PARAM2, true);
	lcd_st7701_send_param(ST7701_CND2BKxSEL_PARAM3, true);
	lcd_st7701_send_param(ST7701_CND2BKxSEL_PARAM4, true);
	lcd_st7701_send_param(ST7701_CND2BKxSEL_ENABLE_BK0, false);

	//Display line setting.
	lcd_st7701_send_cmd(0xC0, true);
	lcd_st7701_send_param(0x3B, true);
	lcd_st7701_send_param(0x00, false);

	//Porch control.
	lcd_st7701_send_cmd(0xC1, true);
	lcd_st7701_send_param(0x0B, true);
	lcd_st7701_send_param(0x02, false);

	//Inversion selection and frame rate control.
	lcd_st7701_send_cmd(0xC2, true);
	lcd_st7701_send_param(0x00, true);
	lcd_st7701_send_param(0x02, false);

	//RGB control.
	lcd_st7701_send_cmd(0xC3, true);
	lcd_st7701_send_param(0x00, false);

	//Color control.
	lcd_st7701_send_cmd(0xCD, true);
	lcd_st7701_send_param(0x08, false);

	//Positive gamma control.
	lcd_st7701_send_cmd(0xB0, true);
	lcd_st7701_send_param(0x02, true);
	lcd_st7701_send_param(0x13, true);
	lcd_st7701_send_param(0x1B, true);
	lcd_st7701_send_param(0x0D, true);
	lcd_st7701_send_param(0x10, true);
	lcd_st7701_send_param(0x05, true);
	lcd_st7701_send_param(0x08, true);
	lcd_st7701_send_param(0x07, true);
	lcd_st7701_send_param(0x07, true);
	lcd_st7701_send_param(0x24, true);
	lcd_st7701_send_param(0x04, true);
	lcd_st7701_send_param(0x11, true);
	lcd_st7701_send_param(0x0E, true);
	lcd_st7701_send_param(0x2C, true);
	lcd_st7701_send_param(0x33, true);
	lcd_st7701_send_param(0x1D, false);

	//Negative gamma control.
	lcd_st7701_send_cmd(0xB1, true);
	lcd_st7701_send_param(0x05, true);
	lcd_st7701_send_param(0x13, true);
	lcd_st7701_send_param(0x1B, true);
	lcd_st7701_send_param(0x0D, true);
	lcd_st7701_send_param(0x10, true);
	lcd_st7701_send_param(0x05, true);
	lcd_st7701_send_param(0x08, true);
	lcd_st7701_send_param(0x07, true);
	lcd_st7701_send_param(0x07, true);
	lcd_st7701_send_param(0x24, true);
	lcd_st7701_send_param(0x04, true);
	lcd_st7701_send_param(0x11, true);
	lcd_st7701_send_param(0x0E, true);
	lcd_st7701_send_param(0x2C, true);
	lcd_st7701_send_param(0x33, true);
	lcd_st7701_send_param(0x1D, false);

	//Enable BK1 function of command 2.
	lcd_st7701_send_cmd(ST7701_CND2BKxSEL_CMD, true);
	lcd_st7701_send_param(ST7701_CND2BKxSEL_PARAM1, true);
	lcd_st7701_send_param(ST7701_CND2BKxSEL_PARAM2, true);
	lcd_st7701_send_param(ST7701_CND2BKxSEL_PARAM3, true);
	lcd_st7701_send_param(ST7701_CND2BKxSEL_PARAM4, true);
	lcd_st7701_send_param(ST7701_CND2BKxSEL_ENABLE_BK1, false);

	//VOP amplitude setting.
	lcd_st7701_send_cmd(0xB0, true);
	lcd_st7701_send_param(0x5D, false);

	//VCOM amplitude setting.
	lcd_st7701_send_cmd(0xB1, true);
	lcd_st7701_send_param(0x43, false);

	//VGH voltage setting.
	lcd_st7701_send_cmd(0xB2, true);
	lcd_st7701_send_param(0x81, false);

	//VGL voltage setting.
	lcd_st7701_send_cmd(0xB5, true);
	lcd_st7701_send_param(0x43, false);

	//Power control 1.
	lcd_st7701_send_cmd(0xB7, true);
	lcd_st7701_send_param(0x85, false);

	//Power control 2.
	lcd_st7701_send_cmd(0xB8, true);
	lcd_st7701_send_param(0x20, false);

	//Source pre drive timing set 1.
	lcd_st7701_send_cmd(0xC1, true);
	lcd_st7701_send_param(0x78, false);

	//Source EQ2 setting.
	lcd_st7701_send_cmd(0xC2, true);
	lcd_st7701_send_param(0x78, false);

	//MIPI setting 1.
	//lcd_st7701_send_cmd(0xD0, true);
	//lcd_st7701_send_param(0x88, false);

	//Sunlight readable enhancement.
	//lcd_st7701_send_cmd(0xE0, true);
	//lcd_st7701_send_param(0x00, true);
	//lcd_st7701_send_param(0x00, true);
	//lcd_st7701_send_param(0x02, false);

	//Noise reduce control.
	//lcd_st7701_send_cmd(0xE1, true);
	//lcd_st7701_send_param(0x03, true);
	//lcd_st7701_send_param(0xA0, true);
	//lcd_st7701_send_param(0x00, true);
	//lcd_st7701_send_param(0x04, true);
	//lcd_st7701_send_param(0xA0, true);
	//lcd_st7701_send_param(0x00, true);
	//lcd_st7701_send_param(0x00, true);
	//lcd_st7701_send_param(0x00, true);
	//lcd_st7701_send_param(0x20, true);
	//lcd_st7701_send_param(0x20, false);

	//Sharpness control.
	//lcd_st7701_send_cmd(0xE2, true);
	//lcd_st7701_send_param(0x00, true);
	//lcd_st7701_send_param(0x00, true);
	//lcd_st7701_send_param(0x00, true);
	//lcd_st7701_send_param(0x00, true);
	//lcd_st7701_send_param(0x00, true);
	//lcd_st7701_send_param(0x00, true);
	//lcd_st7701_send_param(0x00, true);
	//lcd_st7701_send_param(0x00, true);
	//lcd_st7701_send_param(0x00, true);
	//lcd_st7701_send_param(0x00, true);
	//lcd_st7701_send_param(0x00, true);
	//lcd_st7701_send_param(0x00, true);
	//lcd_st7701_send_param(0x00, false);

	//Color calibration control.
	//lcd_st7701_send_cmd(0xE3, true);
	//lcd_st7701_send_param(0x00, true);
	//lcd_st7701_send_param(0x00, true);
	//lcd_st7701_send_param(0x11, true);
	//lcd_st7701_send_param(0x00, false);

	//Skin tone preservation control.
	//lcd_st7701_send_cmd(0xE4, true);
	//lcd_st7701_send_param(0x22, true);
	//lcd_st7701_send_param(0x00, false);

	//Disable BKx function of command 2.
	lcd_st7701_send_cmd(ST7701_CND2BKxSEL_CMD, true);
	lcd_st7701_send_param(ST7701_CND2BKxSEL_PARAM1, true);
	lcd_st7701_send_param(ST7701_CND2BKxSEL_PARAM2, true);
	lcd_st7701_send_param(ST7701_CND2BKxSEL_PARAM3, true);
	lcd_st7701_send_param(ST7701_CND2BKxSEL_PARAM4, true);
	lcd_st7701_send_param(ST7701_CND2BKxSEL_DISABLE, false);

	/*MADCTL*/
	lcd_st7701_send_cmd(ST7701_MADCTL_CMD, true);
	lcd_st7701_send_cmd(ST7701_MADCTL_ML_NORMAL_SCAN | ST7701_MADCTL_BGR_RGB, false);

	/*COLMOD*/
	lcd_st7701_send_cmd(ST7701_COLMOD_CMD, true);
	lcd_st7701_send_cmd(ST7701_COLMOD_VIPF_18BPP, false);

	//All pixels on.
	lcd_st7701_send_cmd(ST7701_ALLPON_CMD, false);

	//Read back the power mode.
	static uint32_t power_mode;
	lcd_st7701_send_cmd(ST7701_RDDPM_CMD, true);
	lcd_st7701_read_param(&power_mode, false);
	if (power_mode != ST7701_RDDPM_PARAM)
	{
		while (1)
		{
			io_test_led_on();
			timer_delay_ms(500);
			io_test_led_off();
			timer_delay_ms(500);
		}
	}

	//Read and check the self diagnostic register.
	self_diag = 0x00;
	self_diag = lcd_st7701_self_diag();
	if (self_diag != ST7701_RDDSDR_PARAM)
	{
		while (1)
		{
			io_test_led_on();
			timer_delay_ms(500);
			io_test_led_off();
			timer_delay_ms(500);
		}
	}
}

static void lcd_st7701_adafruit_spi_config()
{
	/*NOTES:
	 * - Operating in "DE mode".
	 *
	 */
	lcd_st7701_send_cmd(0x01, false);		//Software reset.

	lcd_st7701_send_cmd(0xFF, true);		//Command2 BKx Selection
	lcd_st7701_send_param(0x77, true);		//Nothing.
	lcd_st7701_send_param(0x01, true);		//Nothing.
	lcd_st7701_send_param(0x00, true);		//Nothing.
	lcd_st7701_send_param(0x00, true);		//Nothing.
	lcd_st7701_send_param(0x10, false);		//Selects and enables BK0.


	lcd_st7701_send_cmd(0xC0, true);		//Display line setting (LNESET).
	lcd_st7701_send_param(0x3B, true);		//0b0011 1011, display line setting = 59, no extra line.
	lcd_st7701_send_param(0x00, false);		//No delta line.


	lcd_st7701_send_cmd(0xC1, true);		//Porch control (PORCTRL)
	lcd_st7701_send_param(0x0B, true);		//Vertical back porch = 12.
	lcd_st7701_send_param(0x02, false);		//Vertical front porch = 2.

	lcd_st7701_send_cmd(0xC2, true);		//Inversion and frame rate control.
	lcd_st7701_send_param(0x00, true);		//"1 dot inversion"
	lcd_st7701_send_param(0x02, false);		//Sets minimum number of PCLK per line to 2.

	lcd_st7701_send_cmd(0xCC, true);		//THIS IS NOT IN THE DATASHEET????
	lcd_st7701_send_param(0x10, false);

	lcd_st7701_send_cmd(0xCD, true);		//Color control (COLCTRL)
	lcd_st7701_send_param(0x08, false);		//Pixel collect to DB[17:0] (see table 17), copy self MSB.

	lcd_st7701_send_cmd(0xB0, true);		//Positive voltage gamma control.
	lcd_st7701_send_param(0x02, true);
	lcd_st7701_send_param(0x13, true);
	lcd_st7701_send_param(0x1B, true);
	lcd_st7701_send_param(0x0D, true);
	lcd_st7701_send_param(0x10, true);
	lcd_st7701_send_param(0x05, true);
	lcd_st7701_send_param(0x08, true);
	lcd_st7701_send_param(0x07, true);
	lcd_st7701_send_param(0x07, true);
	lcd_st7701_send_param(0x24, true);
	lcd_st7701_send_param(0x04, true);
	lcd_st7701_send_param(0x11, true);
	lcd_st7701_send_param(0x0E, true);
	lcd_st7701_send_param(0x2C, true);
	lcd_st7701_send_param(0x33, true);
	lcd_st7701_send_param(0x1D, false);

	lcd_st7701_send_cmd(0xB1, true);		//Negative voltage gamma control.
	lcd_st7701_send_param(0x05, true);
	lcd_st7701_send_param(0x13, true);
	lcd_st7701_send_param(0x1B, true);
	lcd_st7701_send_param(0x0D, true);
	lcd_st7701_send_param(0x11, true);
	lcd_st7701_send_param(0x05, true);
	lcd_st7701_send_param(0x08, true);
	lcd_st7701_send_param(0x07, true);
	lcd_st7701_send_param(0x07, true);
	lcd_st7701_send_param(0x24, true);
	lcd_st7701_send_param(0x04, true);
	lcd_st7701_send_param(0x11, true);
	lcd_st7701_send_param(0x0E, true);
	lcd_st7701_send_param(0x2C, true);
	lcd_st7701_send_param(0x33, true);
	lcd_st7701_send_param(0x1D, false);

	lcd_st7701_send_cmd(0xFF, true);		//Command2 BKx Selection.
	lcd_st7701_send_param(0x77, true);		//Nothing.
	lcd_st7701_send_param(0x01, true);		//Nothing.
	lcd_st7701_send_param(0x00, true);		//Nothing.
	lcd_st7701_send_param(0x00, true);		//Nothing.
	lcd_st7701_send_param(0x11, false);		//Selects BK1 mode.

	lcd_st7701_send_cmd(0xB0, true);		//Vop amplitude setting.
	lcd_st7701_send_param(0x5D, false);

	lcd_st7701_send_cmd(0xB1, true);		//VCOM amplitude setting.
	lcd_st7701_send_param(0x43, false);

	lcd_st7701_send_cmd(0xB2, true);		//VGH voltage setting.
	lcd_st7701_send_param(0x81, false);

	lcd_st7701_send_cmd(0xB3, true);		//TEST command setting..?
	lcd_st7701_send_param(0x80, false);

	lcd_st7701_send_cmd(0xB5, true);		//VGL voltage setting.
	lcd_st7701_send_param(0x43, false);

	lcd_st7701_send_cmd(0xB7, true);		//Power control 1.
	lcd_st7701_send_param(0x85, false);

	lcd_st7701_send_cmd(0xB8, true);		//Power control 2.
	lcd_st7701_send_param(0x20, false);

	lcd_st7701_send_cmd(0xC1, true);		//Source pre drive timing set 1.
	lcd_st7701_send_param(0x78, false);

	lcd_st7701_send_cmd(0xC2, true);		//Source EQ2 setting.
	lcd_st7701_send_param(0x78, false);

	lcd_st7701_send_cmd(0xD0, true);		//MIPI setting. (why are we doing this)
	lcd_st7701_send_param(0x88, false);		//Enable EOTP report error, disable ERR pin output.

	lcd_st7701_send_cmd(0xE0, true);		//Not in datasheet???
	lcd_st7701_send_param(0x00, true);
	lcd_st7701_send_param(0x00, true);
	lcd_st7701_send_param(0x02, false);

	lcd_st7701_send_cmd(0xE1, true);		//Noise reduction control.
	lcd_st7701_send_param(0x03, true);		//Disable?? and set to level 3.
	lcd_st7701_send_param(0xA0, true);
	lcd_st7701_send_param(0x00, true);
	lcd_st7701_send_param(0x00, true);
	lcd_st7701_send_param(0x04, true);
	lcd_st7701_send_param(0xA0, true);
	lcd_st7701_send_param(0x00, true);
	lcd_st7701_send_param(0x00, true);
	lcd_st7701_send_param(0x00, true);
	lcd_st7701_send_param(0x20, true);
	lcd_st7701_send_param(0x20, false);

	lcd_st7701_send_cmd(0xE2, true);		//Sharpness control.
	lcd_st7701_send_param(0x00, true);		//Disable and set level to 0.
	lcd_st7701_send_param(0x00, true);		//All below do nothing.
	lcd_st7701_send_param(0x00, true);
	lcd_st7701_send_param(0x00, true);
	lcd_st7701_send_param(0x00, true);
	lcd_st7701_send_param(0x00, true);
	lcd_st7701_send_param(0x00, true);
	lcd_st7701_send_param(0x00, true);
	lcd_st7701_send_param(0x00, true);
	lcd_st7701_send_param(0x00, true);
	lcd_st7701_send_param(0x00, true);
	lcd_st7701_send_param(0x00, true);
	lcd_st7701_send_param(0x00, false);

	lcd_st7701_send_cmd(0xE3, true);		//Color calibration control.
	lcd_st7701_send_param(0x00, true);		//Disable.
	lcd_st7701_send_param(0x00, true);		//Nothing.
	lcd_st7701_send_param(0x11, true);		//Nothing.
	lcd_st7701_send_param(0x00, false);		//Nothing.

	lcd_st7701_send_cmd(0xE4, true);		//Skin tone preservation control.
	lcd_st7701_send_param(0x22, true);		//Enable and set mode?
	lcd_st7701_send_param(0x00, false);		//Does nothing.

	lcd_st7701_send_cmd(0xE5, true);		//Not in datasheet???
	lcd_st7701_send_param(0x05, true);
	lcd_st7701_send_param(0xEC, true);
	lcd_st7701_send_param(0xA0, true);
	lcd_st7701_send_param(0xA0, true);
	lcd_st7701_send_param(0x07, true);
	lcd_st7701_send_param(0xEE, true);
	lcd_st7701_send_param(0xA0, true);
	lcd_st7701_send_param(0xA0, true);
	lcd_st7701_send_param(0x00, true);
	lcd_st7701_send_param(0x00, true);
	lcd_st7701_send_param(0x00, true);
	lcd_st7701_send_param(0x00, true);
	lcd_st7701_send_param(0x00, true);
	lcd_st7701_send_param(0x00, true);
	lcd_st7701_send_param(0x00, true);
	lcd_st7701_send_param(0x00, false);

	lcd_st7701_send_cmd(0xE6, true);		//Not in datasheet???
	lcd_st7701_send_param(0x00, true);
	lcd_st7701_send_param(0x00, true);
	lcd_st7701_send_param(0x11, true);
	lcd_st7701_send_param(0x00, false);

	lcd_st7701_send_cmd(0xE7, true);		//Not in datasheet???
	lcd_st7701_send_param(0x22, true);
	lcd_st7701_send_param(0x00, false);

	lcd_st7701_send_cmd(0xE8, true);		//Not in datasheet???
	lcd_st7701_send_param(0x06, true);
	lcd_st7701_send_param(0xED, true);
	lcd_st7701_send_param(0xA0, true);
	lcd_st7701_send_param(0xA0, true);
	lcd_st7701_send_param(0x08, true);
	lcd_st7701_send_param(0xEF, true);
	lcd_st7701_send_param(0xA0, true);
	lcd_st7701_send_param(0xA0, true);
	lcd_st7701_send_param(0x00, true);
	lcd_st7701_send_param(0x00, true);
	lcd_st7701_send_param(0x00, true);
	lcd_st7701_send_param(0x00, true);
	lcd_st7701_send_param(0x00, true);
	lcd_st7701_send_param(0x00, true);
	lcd_st7701_send_param(0x00, true);
	lcd_st7701_send_param(0x00, false);

	lcd_st7701_send_cmd(0xEB, true);		//Not in datasheet???
	lcd_st7701_send_param(0x00, true);
	lcd_st7701_send_param(0x00, true);
	lcd_st7701_send_param(0x40, true);
	lcd_st7701_send_param(0x40, true);
	lcd_st7701_send_param(0x00, true);
	lcd_st7701_send_param(0x00, true);
	lcd_st7701_send_param(0x00, false);

	lcd_st7701_send_cmd(0xED, true);		//Not in datasheet???
	lcd_st7701_send_param(0xFF, true);
	lcd_st7701_send_param(0xFF, true);
	lcd_st7701_send_param(0xFF, true);
	lcd_st7701_send_param(0xBA, true);
	lcd_st7701_send_param(0x0A, true);
	lcd_st7701_send_param(0xBF, true);
	lcd_st7701_send_param(0x45, true);
	lcd_st7701_send_param(0xFF, true);
	lcd_st7701_send_param(0xFF, true);
	lcd_st7701_send_param(0x54, true);
	lcd_st7701_send_param(0xFB, true);
	lcd_st7701_send_param(0xA0, true);
	lcd_st7701_send_param(0xAB, true);
	lcd_st7701_send_param(0xFF, true);
	lcd_st7701_send_param(0xFF, true);
	lcd_st7701_send_param(0xFF, false);

	lcd_st7701_send_cmd(0xEF, true);		//Not in datasheet???
	lcd_st7701_send_param(0x10, true);
	lcd_st7701_send_param(0x0D, true);
	lcd_st7701_send_param(0x04, true);
	lcd_st7701_send_param(0x08, true);
	lcd_st7701_send_param(0x3F, true);
	lcd_st7701_send_param(0x1F, false);

	lcd_st7701_send_cmd(0xFF, true);		//Command2 BKx Selection.
	lcd_st7701_send_param(0x77, true);		//Nothing.
	lcd_st7701_send_param(0x01, true);		//Nothing.
	lcd_st7701_send_param(0x00, true);		//Nothing.
	lcd_st7701_send_param(0x00, true);		//Nothing.
	lcd_st7701_send_param(0x13, false);		//Selects BK3 setting.

	lcd_st7701_send_cmd(0xEF, true);		//Not in datasheet...
	lcd_st7701_send_param(0x08, false);

	lcd_st7701_send_cmd(0xFF, true);		//Command2 BKx Selection.
	lcd_st7701_send_param(0x77, true);		//Nothing.
	lcd_st7701_send_param(0x01, true);		//Nothing.
	lcd_st7701_send_param(0x00, true);		//Nothing.
	lcd_st7701_send_param(0x00, true);		//Nothing.
	lcd_st7701_send_param(0x00, false);		//Disable BK function of command 2.

	lcd_st7701_send_cmd(0x36, true);		//Display data access control. MADCTL
	lcd_st7701_send_param(0x00, false);		//Normal scan, RGB not BGR.

	lcd_st7701_send_cmd(0x3A, true);		//Interface pixel format (COLMOD).
	lcd_st7701_send_param(0x60, false);		//18 bits per pixel.

	lcd_st7701_send_cmd(0x11, false);		//Sleep out (SLPOUT).

	lcd_st7701_send_cmd(0x29, false);		//Display on (DISPON).
}


/******		GLOBAL FUNCTION DEFINITIONS		*******/
void lcd_init()
{
	//Initialize the ST7701 SPI driver.
#ifdef TARGET_HARDWARE_CANGAUGE
	io_set_output_speed(LTDC_RESET_io, GPIO_OSPEEDR_VERY_HIGH);
	io_set_pin_dir_out(LTDC_RESET_io);
	io_pin_out_clr(LTDC_RESET_io);
	lcd_st7701_init();
#endif	//TARGET_HARDWARE_CANGAUGE

	/*Enable pixel clock.*/
	rcc_enable_pll3r();

	/*Enable clock in the RCC.*/
	RCC->APB3ENR |= RCC_APB3ENR_LTDCEN;

	/*Set vertical and horizontal sync width.*/
	const uint32_t reg_vsync = LTDC_SSCR_VSYNC_HEIGHT_px - 1;
	const uint32_t reg_hsync = LTDC_SSCR_HSYNC_WIDTH_px - 1;
	LTDC->SSCR = (reg_vsync << LTDC_SSCR_VSH_Pos)
				| (reg_hsync << LTDC_SSCR_HSW_Pos);

	/*Set the vert and horz back porch.*/
	const uint32_t reg_vert_bp = LTDC_BPCR_VERT_BK_PORCH_px - 1;
	const uint32_t reg_horz_bp = LTDC_BPCR_HORZ_BK_PORCH_px - 1;
	LTDC->BPCR = (reg_vert_bp << LTDC_BPCR_AVBP_Pos)
				|(reg_horz_bp << LTDC_BPCR_AHBP_Pos);

	/*Set the active width and height.*/
	const uint32_t reg_act_height = LTDC_AWCR_ACT_HEIGHT_px - 1;
	const uint32_t reg_act_width = LTDC_AWCR_ACT_WIDTH_px - 1;
	LTDC->AWCR = (reg_act_height << LTDC_AWCR_AAH_Pos)
				| (reg_act_width << LTDC_AWCR_AAW_Pos);

	/*Set the total width and height.*/
	const uint32_t reg_tot_height = LTDC_TWCR_TOT_HEIGHT_px - 1;
	const uint32_t reg_tot_width = LTDC_TWCR_TOT_WIDTH_px - 1;
	LTDC->TWCR = (reg_tot_height << LTDC_TWCR_TOTALH_Pos)
				| (reg_tot_width << LTDC_TWCR_TOTALW_Pos);

	/*Configure synchronous signals and clock polarity.*/
	LTDC->GCR = (LTDC_GCR_DBW_Val << LTDC_GCR_DBW_Pos)
			| (LTDC_GCR_DGW_Val << LTDC_GCR_DGW_Pos)
			| (LTDC_GCR_DRW_Val << LTDC_GCR_DGW_Pos);

	/*Set the default background color.*/
	LTDC->BCCR = LTDC_BCCR_Val;


	/*Configure horizontal position.*/
	LTDC_Layer1->WHPCR = (LTDC_LxWHPCR_HORZ_START_Val << LTDC_LxWHPCR_WHSTPOS_Pos)
						| ((LTDC_LxWHPCR_HORZ_STOP_Val - 1) << LTDC_LxWHPCR_WHSPPOS_Pos);
	LTDC_Layer1->WVPCR = (LTDC_LxWVPCR_VERT_START_Val << LTDC_LxWVPCR_WVSTPOS_Pos)
						| ((LTDC_LxWVPCR_VERT_STOP_Val - 1) << LTDC_LxWVPCR_WVSPPOS_Pos);

	/*Constant alpha config register.*/
	LTDC_Layer1->CACR = LTDC_LxCACR_Val;

	/*Configure blending times layer factors.*/
	LTDC_Layer1->BFCR = (LTDC_LxBFCR_BF1_PXxCONST << LTDC_LxBFCR_BF1_Pos)
						| (LTDC_LxBFCR_BF2_PXxCONST << LTDC_LxBFCR_BF2_Pos);

	/*Set the buffer address.*/
	LTDC_Layer1->CFBAR = LTDC_DISP_BUFFER_ADDR;

	/*Set the buffer size registers.*/
	LTDC_Layer1->CFBLR = (LTDC_LxCFBLR_BUFFER_PITCH_Val << LTDC_LxCFBLR_CFBP_Pos)
						| (LTDC_LxCFBLR_LINE_LENGTH_Val << LTDC_LxCFBLR_CFBLL_Pos);
	LTDC_Layer1->CFBLNR = LTDC_LxCFBLNR_NUM_LINES_Val;

	/*Enable layer 1.*/
	LTDC_Layer1->CR = LTDC_LxCR_LEN;

	/*Reload the layer registers.*/
	LTDC->SRCR = LTDC_SRCR_IMR;

#ifdef TARGET_HARDWARE_CANGAUGE
	LTDC->GCR |= LTDC_GCR_PCPOL;
#endif

	/*Enable pin alternate function for all the LTDC pins.*/
	io_set_pin_mux(LTDC_RED_DATA_2_io, GPIO_AFR_AF14);
	io_set_pin_mux(LTDC_RED_DATA_3_io, GPIO_AFR_AF14);
	io_set_pin_mux(LTDC_RED_DATA_4_io, GPIO_AFR_AF14);
	io_set_pin_mux(LTDC_RED_DATA_5_io, GPIO_AFR_AF14);
	io_set_pin_mux(LTDC_RED_DATA_6_io, GPIO_AFR_AF14);
	io_set_pin_mux(LTDC_RED_DATA_7_io, GPIO_AFR_AF14);

	io_set_pin_mux(LTDC_GREEN_DATA_2_io, GPIO_AFR_AF14);
	io_set_pin_mux(LTDC_GREEN_DATA_3_io, GPIO_AFR_AF14);
	io_set_pin_mux(LTDC_GREEN_DATA_4_io, GPIO_AFR_AF14);
	io_set_pin_mux(LTDC_GREEN_DATA_5_io, GPIO_AFR_AF14);
	io_set_pin_mux(LTDC_GREEN_DATA_6_io, GPIO_AFR_AF14);
	io_set_pin_mux(LTDC_GREEN_DATA_7_io, GPIO_AFR_AF14);

	io_set_pin_mux(LTDC_BLUE_DATA_2_io, GPIO_AFR_AF14);
	io_set_pin_mux(LTDC_BLUE_DATA_3_io, GPIO_AFR_AF14);
	io_set_pin_mux(LTDC_BLUE_DATA_4_io, GPIO_AFR_AF14);
	io_set_pin_mux(LTDC_BLUE_DATA_5_io, GPIO_AFR_AF14);
	io_set_pin_mux(LTDC_BLUE_DATA_6_io, GPIO_AFR_AF14);
	io_set_pin_mux(LTDC_BLUE_DATA_7_io, GPIO_AFR_AF14);
	io_set_pin_mux(LTDC_CLK_io, GPIO_AFR_AF14);
	io_set_pin_mux(LTDC_HSYNC_io, GPIO_AFR_AF14);
	io_set_pin_mux(LTDC_VSYNC_io, GPIO_AFR_AF14);
	io_set_pin_mux(LTDC_DE_io, GPIO_AFR_AF14);



	/*Set the output speed to very high for all the IO lines.*/
	io_set_output_speed(LTDC_RED_DATA_2_io, GPIO_OSPEEDR_VERY_HIGH);
	io_set_output_speed(LTDC_RED_DATA_3_io, GPIO_OSPEEDR_VERY_HIGH);
	io_set_output_speed(LTDC_RED_DATA_4_io, GPIO_OSPEEDR_VERY_HIGH);
	io_set_output_speed(LTDC_RED_DATA_5_io, GPIO_OSPEEDR_VERY_HIGH);
	io_set_output_speed(LTDC_RED_DATA_6_io, GPIO_OSPEEDR_VERY_HIGH);
	io_set_output_speed(LTDC_RED_DATA_7_io, GPIO_OSPEEDR_VERY_HIGH);
	io_set_output_speed(LTDC_GREEN_DATA_2_io, GPIO_OSPEEDR_VERY_HIGH);
	io_set_output_speed(LTDC_GREEN_DATA_3_io, GPIO_OSPEEDR_VERY_HIGH);
	io_set_output_speed(LTDC_GREEN_DATA_4_io, GPIO_OSPEEDR_VERY_HIGH);
	io_set_output_speed(LTDC_GREEN_DATA_5_io, GPIO_OSPEEDR_VERY_HIGH);
	io_set_output_speed(LTDC_GREEN_DATA_6_io, GPIO_OSPEEDR_VERY_HIGH);
	io_set_output_speed(LTDC_GREEN_DATA_7_io, GPIO_OSPEEDR_VERY_HIGH);
	io_set_output_speed(LTDC_BLUE_DATA_2_io, GPIO_OSPEEDR_VERY_HIGH);
	io_set_output_speed(LTDC_BLUE_DATA_3_io, GPIO_OSPEEDR_VERY_HIGH);
	io_set_output_speed(LTDC_BLUE_DATA_4_io, GPIO_OSPEEDR_VERY_HIGH);
	io_set_output_speed(LTDC_BLUE_DATA_5_io, GPIO_OSPEEDR_VERY_HIGH);
	io_set_output_speed(LTDC_BLUE_DATA_6_io, GPIO_OSPEEDR_VERY_HIGH);
	io_set_output_speed(LTDC_BLUE_DATA_7_io, GPIO_OSPEEDR_VERY_HIGH);
	io_set_output_speed(LTDC_CLK_io, GPIO_OSPEEDR_VERY_HIGH);
	io_set_output_speed(LTDC_HSYNC_io, GPIO_OSPEEDR_VERY_HIGH);
	io_set_output_speed(LTDC_VSYNC_io, GPIO_OSPEEDR_VERY_HIGH);
	io_set_output_speed(LTDC_DE_io, GPIO_OSPEEDR_VERY_HIGH);

#ifdef TARGET_HARDWARE_STM32H745DISCO
	io_set_pin_mux(LTDC_RED_DATA_0_io, GPIO_AFR_AF14);
	io_set_pin_mux(LTDC_RED_DATA_1_io, GPIO_AFR_AF14);
	io_set_pin_mux(LTDC_GREEN_DATA_0_io, GPIO_AFR_AF14);
	io_set_pin_mux(LTDC_GREEN_DATA_1_io, GPIO_AFR_AF14);
	io_set_pin_mux(LTDC_BLUE_DATA_0_io, GPIO_AFR_AF14);
	io_set_pin_mux(LTDC_BLUE_DATA_1_io, GPIO_AFR_AF14);
	io_set_output_speed(LTDC_RED_DATA_0_io, GPIO_OSPEEDR_VERY_HIGH);
	io_set_output_speed(LTDC_RED_DATA_1_io, GPIO_OSPEEDR_VERY_HIGH);
	io_set_output_speed(LTDC_GREEN_DATA_0_io, GPIO_OSPEEDR_VERY_HIGH);
	io_set_output_speed(LTDC_GREEN_DATA_1_io, GPIO_OSPEEDR_VERY_HIGH);
	io_set_output_speed(LTDC_BLUE_DATA_0_io, GPIO_OSPEEDR_VERY_HIGH);
	io_set_output_speed(LTDC_BLUE_DATA_1_io, GPIO_OSPEEDR_VERY_HIGH);
	io_set_output_speed(LTDC_DISP_io, GPIO_OSPEEDR_VERY_HIGH);
	io_set_pin_dir_out(LTDC_DISP_io);
#endif	//TARGET_HARDWARE_STM32H745DISCO

	/*LCD backlight pin config.*/
	io_init();
	io_set_pin_dir_out(GPIOB, GPIO_PIN14_Msk);
	io_pin_out_clr(GPIOB, GPIO_PIN14_Msk);	/*TODO: Double check this PWM code.
	io_set_pin_mux(GPIOB, GPIO_PIN14_Msk, GPIO_AFR_AF2);
	timer_init(TIM12);
	timer_enable_pwm_output(TIM12, 1);
	timer_set_pwm_freq(TIM12, 1000);
	timer_set_pwm_duty_cycle(TIM12, 1000, 1);
	timer_enable(TIM12);
	*/

	/*Enable the LCD.*/
	lcd_enable();

	/*Initialize LVGL and the configure it to work with the LCD. lv_init must be called first.*/
	lv_init();
	lcd_lvgl_init();
}

/*Enables the LTDC module and the LCD screen itself.*/
void lcd_enable()
{
#ifdef TARGET_HARDWARE_STM32H745DISCO
	io_pin_out_set(LTDC_DISP_io);
#endif //TARGET_HARDWARE_STM32H745DISCO

#ifdef TARGET_HARDWARE_CANGAUGE
	io_pin_out_set(LTDC_RESET_io);
#endif	//TARGET_HARDWARE_CANGAUGE

	LTDC->GCR |= LTDC_GCR_LTDCEN;
}

void lcd_lvgl_init()
{
	/*Set up the frame buffer for lvgl */
	static lv_disp_draw_buf_t draw_buf_dsc_1;
	lv_disp_draw_buf_init(&draw_buf_dsc_1, LTDC_LVGL_BUFFER1_ADDR,
							LTDC_LVGL_BUFFER2_ADDR, LTDC_SCREEN_SIZE_X_px * LTDC_SCREEN_SIZE_Y_px);   /*Initialize the display buffer*/


	/*Link lvgl and the display driver.*/
	static lv_disp_drv_t disp_drv;                  /*Descriptor of a display driver*/
	lv_disp_drv_init(&disp_drv);                    /*Basic initialization*/

	/*IMPORTANT!!!
     *Inform the graphics library that you are ready with the flushing*/
    lcd_flush_rdy_disp_drv = &disp_drv;

	/*Set the resolution of the display*/
	disp_drv.hor_res = LTDC_SCREEN_SIZE_X_px;
	disp_drv.ver_res = LTDC_SCREEN_SIZE_Y_px;

	/*Used to copy the buffer's content to the display*/
	disp_drv.flush_cb = lcd_lvgl_disp_flush;
	disp_drv.clean_dcache_cb = disp_clean_dcache;

	/*Set a display buffer*/
	disp_drv.draw_buf = &draw_buf_dsc_1;

	/*Finally register the driver*/
	lv_disp_drv_register(&disp_drv);

	/*Call the flush ready function on a DMA2D transfer complete interrupt to let LVGL know were done flushing.*/
	dma2d_init(LTDC_SCREEN_SIZE_X_px, LTDC_SCREEN_SIZE_Y_px);
	dma2d_set_transfer_complete_handler(lcd_lvgl_flush_complete);
	dma2d_enable_interrupt(TRANSFER_COMPLETE);
	dma2d_nvic_enable_interrupt();
}

void lcd_solid_color_test_red()
{
	/*Enable pin alternate function for all the LTDC pins.*/
	io_set_pin_dir_out(LTDC_RED_DATA_2_io);
	io_set_pin_dir_out(LTDC_RED_DATA_3_io);
	io_set_pin_dir_out(LTDC_RED_DATA_4_io);
	io_set_pin_dir_out(LTDC_RED_DATA_5_io);
	io_set_pin_dir_out(LTDC_RED_DATA_6_io);
	io_set_pin_dir_out(LTDC_RED_DATA_7_io);
	io_set_pin_dir_out(LTDC_GREEN_DATA_2_io);
	io_set_pin_dir_out(LTDC_GREEN_DATA_3_io);
	io_set_pin_dir_out(LTDC_GREEN_DATA_4_io);
	io_set_pin_dir_out(LTDC_GREEN_DATA_5_io);
	io_set_pin_dir_out(LTDC_GREEN_DATA_6_io);
	io_set_pin_dir_out(LTDC_GREEN_DATA_7_io);
	io_set_pin_dir_out(LTDC_BLUE_DATA_2_io);
	io_set_pin_dir_out(LTDC_BLUE_DATA_3_io);
	io_set_pin_dir_out(LTDC_BLUE_DATA_4_io);
	io_set_pin_dir_out(LTDC_BLUE_DATA_5_io);
	io_set_pin_dir_out(LTDC_BLUE_DATA_6_io);
	io_set_pin_dir_out(LTDC_BLUE_DATA_7_io);

	io_pin_out_set(LTDC_RED_DATA_2_io);
	io_pin_out_set(LTDC_RED_DATA_3_io);
	io_pin_out_set(LTDC_RED_DATA_4_io);
	io_pin_out_set(LTDC_RED_DATA_5_io);
	io_pin_out_set(LTDC_RED_DATA_6_io);
	io_pin_out_set(LTDC_RED_DATA_7_io);
	io_pin_out_clr(LTDC_GREEN_DATA_2_io);
	io_pin_out_clr(LTDC_GREEN_DATA_3_io);
	io_pin_out_clr(LTDC_GREEN_DATA_4_io);
	io_pin_out_clr(LTDC_GREEN_DATA_5_io);
	io_pin_out_clr(LTDC_GREEN_DATA_6_io);
	io_pin_out_clr(LTDC_GREEN_DATA_7_io);
	io_pin_out_clr(LTDC_BLUE_DATA_2_io);
	io_pin_out_clr(LTDC_BLUE_DATA_3_io);
	io_pin_out_clr(LTDC_BLUE_DATA_4_io);
	io_pin_out_clr(LTDC_BLUE_DATA_5_io);
	io_pin_out_clr(LTDC_BLUE_DATA_6_io);
	io_pin_out_clr(LTDC_BLUE_DATA_7_io);
}

void lcd_solid_color_test_green()
{
	/*Enable pin alternate function for all the LTDC pins.*/
	io_set_pin_dir_out(LTDC_RED_DATA_2_io);
	io_set_pin_dir_out(LTDC_RED_DATA_3_io);
	io_set_pin_dir_out(LTDC_RED_DATA_4_io);
	io_set_pin_dir_out(LTDC_RED_DATA_5_io);
	io_set_pin_dir_out(LTDC_RED_DATA_6_io);
	io_set_pin_dir_out(LTDC_RED_DATA_7_io);
	io_set_pin_dir_out(LTDC_GREEN_DATA_2_io);
	io_set_pin_dir_out(LTDC_GREEN_DATA_3_io);
	io_set_pin_dir_out(LTDC_GREEN_DATA_4_io);
	io_set_pin_dir_out(LTDC_GREEN_DATA_5_io);
	io_set_pin_dir_out(LTDC_GREEN_DATA_6_io);
	io_set_pin_dir_out(LTDC_GREEN_DATA_7_io);
	io_set_pin_dir_out(LTDC_BLUE_DATA_2_io);
	io_set_pin_dir_out(LTDC_BLUE_DATA_3_io);
	io_set_pin_dir_out(LTDC_BLUE_DATA_4_io);
	io_set_pin_dir_out(LTDC_BLUE_DATA_5_io);
	io_set_pin_dir_out(LTDC_BLUE_DATA_6_io);
	io_set_pin_dir_out(LTDC_BLUE_DATA_7_io);

	io_pin_out_clr(LTDC_RED_DATA_2_io);
	io_pin_out_clr(LTDC_RED_DATA_3_io);
	io_pin_out_clr(LTDC_RED_DATA_4_io);
	io_pin_out_clr(LTDC_RED_DATA_5_io);
	io_pin_out_clr(LTDC_RED_DATA_6_io);
	io_pin_out_clr(LTDC_RED_DATA_7_io);
	io_pin_out_set(LTDC_GREEN_DATA_2_io);
	io_pin_out_set(LTDC_GREEN_DATA_3_io);
	io_pin_out_set(LTDC_GREEN_DATA_4_io);
	io_pin_out_set(LTDC_GREEN_DATA_5_io);
	io_pin_out_set(LTDC_GREEN_DATA_6_io);
	io_pin_out_set(LTDC_GREEN_DATA_7_io);
	io_pin_out_clr(LTDC_BLUE_DATA_2_io);
	io_pin_out_clr(LTDC_BLUE_DATA_3_io);
	io_pin_out_clr(LTDC_BLUE_DATA_4_io);
	io_pin_out_clr(LTDC_BLUE_DATA_5_io);
	io_pin_out_clr(LTDC_BLUE_DATA_6_io);
	io_pin_out_clr(LTDC_BLUE_DATA_7_io);
}

void lcd_solid_color_test_blue()
{
		/*Enable pin alternate function for all the LTDC pins.*/
		io_set_pin_dir_out(LTDC_RED_DATA_2_io);
		io_set_pin_dir_out(LTDC_RED_DATA_3_io);
		io_set_pin_dir_out(LTDC_RED_DATA_4_io);
		io_set_pin_dir_out(LTDC_RED_DATA_5_io);
		io_set_pin_dir_out(LTDC_RED_DATA_6_io);
		io_set_pin_dir_out(LTDC_RED_DATA_7_io);
		io_set_pin_dir_out(LTDC_GREEN_DATA_2_io);
		io_set_pin_dir_out(LTDC_GREEN_DATA_3_io);
		io_set_pin_dir_out(LTDC_GREEN_DATA_4_io);
		io_set_pin_dir_out(LTDC_GREEN_DATA_5_io);
		io_set_pin_dir_out(LTDC_GREEN_DATA_6_io);
		io_set_pin_dir_out(LTDC_GREEN_DATA_7_io);
		io_set_pin_dir_out(LTDC_BLUE_DATA_2_io);
		io_set_pin_dir_out(LTDC_BLUE_DATA_3_io);
		io_set_pin_dir_out(LTDC_BLUE_DATA_4_io);
		io_set_pin_dir_out(LTDC_BLUE_DATA_5_io);
		io_set_pin_dir_out(LTDC_BLUE_DATA_6_io);
		io_set_pin_dir_out(LTDC_BLUE_DATA_7_io);

		io_pin_out_clr(LTDC_RED_DATA_2_io);
		io_pin_out_clr(LTDC_RED_DATA_3_io);
		io_pin_out_clr(LTDC_RED_DATA_4_io);
		io_pin_out_clr(LTDC_RED_DATA_5_io);
		io_pin_out_clr(LTDC_RED_DATA_6_io);
		io_pin_out_clr(LTDC_RED_DATA_7_io);
		io_pin_out_clr(LTDC_GREEN_DATA_2_io);
		io_pin_out_clr(LTDC_GREEN_DATA_3_io);
		io_pin_out_clr(LTDC_GREEN_DATA_4_io);
		io_pin_out_clr(LTDC_GREEN_DATA_5_io);
		io_pin_out_clr(LTDC_GREEN_DATA_6_io);
		io_pin_out_clr(LTDC_GREEN_DATA_7_io);
		io_pin_out_set(LTDC_BLUE_DATA_2_io);
		io_pin_out_set(LTDC_BLUE_DATA_3_io);
		io_pin_out_set(LTDC_BLUE_DATA_4_io);
		io_pin_out_set(LTDC_BLUE_DATA_5_io);
		io_pin_out_set(LTDC_BLUE_DATA_6_io);
		io_pin_out_set(LTDC_BLUE_DATA_7_io);
}

void lcd_solid_color_test_black()
{
		/*Enable pin alternate function for all the LTDC pins.*/
		io_set_pin_dir_out(LTDC_RED_DATA_2_io);
		io_set_pin_dir_out(LTDC_RED_DATA_3_io);
		io_set_pin_dir_out(LTDC_RED_DATA_4_io);
		io_set_pin_dir_out(LTDC_RED_DATA_5_io);
		io_set_pin_dir_out(LTDC_RED_DATA_6_io);
		io_set_pin_dir_out(LTDC_RED_DATA_7_io);
		io_set_pin_dir_out(LTDC_GREEN_DATA_2_io);
		io_set_pin_dir_out(LTDC_GREEN_DATA_3_io);
		io_set_pin_dir_out(LTDC_GREEN_DATA_4_io);
		io_set_pin_dir_out(LTDC_GREEN_DATA_5_io);
		io_set_pin_dir_out(LTDC_GREEN_DATA_6_io);
		io_set_pin_dir_out(LTDC_GREEN_DATA_7_io);
		io_set_pin_dir_out(LTDC_BLUE_DATA_2_io);
		io_set_pin_dir_out(LTDC_BLUE_DATA_3_io);
		io_set_pin_dir_out(LTDC_BLUE_DATA_4_io);
		io_set_pin_dir_out(LTDC_BLUE_DATA_5_io);
		io_set_pin_dir_out(LTDC_BLUE_DATA_6_io);
		io_set_pin_dir_out(LTDC_BLUE_DATA_7_io);

		io_pin_out_clr(LTDC_RED_DATA_2_io);
		io_pin_out_clr(LTDC_RED_DATA_3_io);
		io_pin_out_clr(LTDC_RED_DATA_4_io);
		io_pin_out_clr(LTDC_RED_DATA_5_io);
		io_pin_out_clr(LTDC_RED_DATA_6_io);
		io_pin_out_clr(LTDC_RED_DATA_7_io);
		io_pin_out_clr(LTDC_GREEN_DATA_2_io);
		io_pin_out_clr(LTDC_GREEN_DATA_3_io);
		io_pin_out_clr(LTDC_GREEN_DATA_4_io);
		io_pin_out_clr(LTDC_GREEN_DATA_5_io);
		io_pin_out_clr(LTDC_GREEN_DATA_6_io);
		io_pin_out_clr(LTDC_GREEN_DATA_7_io);
		io_pin_out_clr(LTDC_BLUE_DATA_2_io);
		io_pin_out_clr(LTDC_BLUE_DATA_3_io);
		io_pin_out_clr(LTDC_BLUE_DATA_4_io);
		io_pin_out_clr(LTDC_BLUE_DATA_5_io);
		io_pin_out_clr(LTDC_BLUE_DATA_6_io);
		io_pin_out_clr(LTDC_BLUE_DATA_7_io);
}

void lcd_solid_color_test_inputs()
{
	io_set_pin_dir_in(LTDC_RED_DATA_2_io);
	io_set_pin_dir_in(LTDC_RED_DATA_3_io);
	io_set_pin_dir_in(LTDC_RED_DATA_4_io);
	io_set_pin_dir_in(LTDC_RED_DATA_5_io);
	io_set_pin_dir_in(LTDC_RED_DATA_6_io);
	io_set_pin_dir_in(LTDC_RED_DATA_7_io);
	io_set_pin_dir_in(LTDC_GREEN_DATA_2_io);
	io_set_pin_dir_in(LTDC_GREEN_DATA_3_io);
	io_set_pin_dir_in(LTDC_GREEN_DATA_4_io);
	io_set_pin_dir_in(LTDC_GREEN_DATA_5_io);
	io_set_pin_dir_in(LTDC_GREEN_DATA_6_io);
	io_set_pin_dir_in(LTDC_GREEN_DATA_7_io);
	io_set_pin_dir_in(LTDC_BLUE_DATA_2_io);
	io_set_pin_dir_in(LTDC_BLUE_DATA_3_io);
	io_set_pin_dir_in(LTDC_BLUE_DATA_4_io);
	io_set_pin_dir_in(LTDC_BLUE_DATA_5_io);
	io_set_pin_dir_in(LTDC_BLUE_DATA_6_io);
	io_set_pin_dir_in(LTDC_BLUE_DATA_7_io);
}


#endif //CORE_CM7
