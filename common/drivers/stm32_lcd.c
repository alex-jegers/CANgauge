/*
 * stm32_lcd.c
 *
 *  Created on: Feb 17, 2024
 *      Author: awjpp
 */

#include "stm32_lcd.h"
#include "stm32_rcc.h"		//For enabling PLL clock.
#include "stm32_io.h"		//For controlling IO related to the lcd.
#include "stm32_dma2d.h"	//For frame buffer control.
#include "stm32_timer.h"	//For lcd backlight PWM.
#include "stm32_spi.h"		//For initializing the ST7701 driver used on CANgauge config.
#include <assert.h>

/*******STATIC VARIABLES************/

/*******	STATIC FUNCTION DECLARATIONS	************/
static void lcd_st7701_init();
static void lcd_st7701_send_cmd(uint8_t cmd, bool hold_ss_low);
static void lcd_st7701_send_param(uint8_t param, bool hold_ss_low);
static void lcd_st7701_read_param(uint32_t* data, bool hold_ss_low);
static bool lcd_st7701_check_id();
static uint8_t lcd_st7701_self_diag();

/*TODO: Rename this.*/
static void lcd_st7701_adafruit_spi_config();

/*******		STATIC FUNCTION DEFINITIONS		********/
static void lcd_st7701_init()
{
#ifdef TARGET_HARDWARE_CANGAUGE

	//Hardware reset.
	//TODO: Make sure this stays low for 10ms.
	io_set_pin_dir_out(LTDC_RESET_io);
	//io_pin_out_clr(LTDC_RESET_io);
	//timer_delay_ms(10);
	io_pin_out_set(LTDC_RESET_io);
	//timer_delay_ms(200);

	//Enable SPI.
	spi4_enable_clocks();
	spi4_enable_io();
	spi_set_sck_div(SPI4, SPI_SCK_DIV_256);
	spi_init(SPI4);
	spi_enable(SPI4);

	lcd_st7701_adafruit_spi_config();

	spi_deinit(SPI4);

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


#ifdef TARGET_HARDWARE_CANGAUGE
static void lcd_st7701_adafruit_spi_config()
{
	/*NOTES:
	 * - Operating in "DE mode".
	 *
	 */
	lcd_st7701_send_cmd(0x01, false);		//Software reset.

	vTaskDelay(100);

	lcd_st7701_send_cmd(0xFF, true);		//Command2 BKx Selection
	lcd_st7701_send_param(0x77, true);		//Nothing.
	lcd_st7701_send_param(0x01, true);		//Nothing.
	lcd_st7701_send_param(0x00, true);		//Nothing.
	lcd_st7701_send_param(0x00, true);		//Nothing.
	lcd_st7701_send_param(0x10, true);		//Selects and enables BK0.


	lcd_st7701_send_cmd(0xC0, true);		//Display line setting (LNESET).
	lcd_st7701_send_param(0x3B, true);		//0b0011 1011, display line setting = 59, no extra line.
	lcd_st7701_send_param(0x00, true);		//No delta line.


	lcd_st7701_send_cmd(0xC1, true);							//Porch control (PORCTRL)
	lcd_st7701_send_param(LTDC_VERT_BK_PORCH_px, true);			//Vertical back porch.
	lcd_st7701_send_param(LTDC_VERT_FRNT_PORCH_px, true);		//Vertical front porch.

	lcd_st7701_send_cmd(0xC2, true);		//Inversion and frame rate control.
	lcd_st7701_send_param(0x00, true);		//"1 dot inversion"
	lcd_st7701_send_param(0x02, true);		//Sets minimum number of PCLK per line to 2.

	lcd_st7701_send_cmd(0xCC, true);		//THIS IS NOT IN THE DATASHEET????
	lcd_st7701_send_param(0x10, true);

	lcd_st7701_send_cmd(0xCD, true);		//Color control (COLCTRL)
	lcd_st7701_send_param(0x08, true);		//Pixel collect to DB[17:0] (see table 17), copy self MSB.

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
	lcd_st7701_send_param(0x1D, true);

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
	lcd_st7701_send_param(0x1D, true);

	lcd_st7701_send_cmd(0xFF, true);		//Command2 BKx Selection.
	lcd_st7701_send_param(0x77, true);		//Nothing.
	lcd_st7701_send_param(0x01, true);		//Nothing.
	lcd_st7701_send_param(0x00, true);		//Nothing.
	lcd_st7701_send_param(0x00, true);		//Nothing.
	lcd_st7701_send_param(0x11, true);		//Selects BK1 mode.

	lcd_st7701_send_cmd(0xB0, true);		//Vop amplitude setting.
	lcd_st7701_send_param(0x5D, true);

	lcd_st7701_send_cmd(0xB1, true);		//VCOM amplitude setting.
	lcd_st7701_send_param(0x43, true);

	lcd_st7701_send_cmd(0xB2, true);		//VGH voltage setting.
	lcd_st7701_send_param(0x81, true);

	lcd_st7701_send_cmd(0xB3, true);		//TEST command setting..?
	lcd_st7701_send_param(0x80, true);

	lcd_st7701_send_cmd(0xB5, true);		//VGL voltage setting.
	lcd_st7701_send_param(0x43, true);

	lcd_st7701_send_cmd(0xB7, true);		//Power control 1.
	lcd_st7701_send_param(0x85, true);

	lcd_st7701_send_cmd(0xB8, true);		//Power control 2.
	lcd_st7701_send_param(0x20, true);

	lcd_st7701_send_cmd(0xC1, true);		//Source pre drive timing set 1.
	lcd_st7701_send_param(0x78, true);

	lcd_st7701_send_cmd(0xC2, true);		//Source EQ2 setting.
	lcd_st7701_send_param(0x78, true);

	lcd_st7701_send_cmd(0xD0, true);		//MIPI setting. (why are we doing this)
	lcd_st7701_send_param(0x88, true);		//Enable EOTP report error, disable ERR pin output.

	lcd_st7701_send_cmd(0xE0, true);		//Not in datasheet???
	lcd_st7701_send_param(0x00, true);
	lcd_st7701_send_param(0x00, true);
	lcd_st7701_send_param(0x02, true);

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
	lcd_st7701_send_param(0x20, true);

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
	lcd_st7701_send_param(0x00, true);

	lcd_st7701_send_cmd(0xE3, true);		//Color calibration control.
	lcd_st7701_send_param(0x00, true);		//Disable.
	lcd_st7701_send_param(0x00, true);		//Nothing.
	lcd_st7701_send_param(0x11, true);		//Nothing.
	lcd_st7701_send_param(0x00, true);		//Nothing.

	lcd_st7701_send_cmd(0xE4, true);		//Skin tone preservation control.
	lcd_st7701_send_param(0x22, true);		//Enable and set mode?
	lcd_st7701_send_param(0x00, true);		//Does nothing.

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
	lcd_st7701_send_param(0x00, true);

	lcd_st7701_send_cmd(0xE6, true);		//Not in datasheet???
	lcd_st7701_send_param(0x00, true);
	lcd_st7701_send_param(0x00, true);
	lcd_st7701_send_param(0x11, true);
	lcd_st7701_send_param(0x00, true);

	lcd_st7701_send_cmd(0xE7, true);		//Not in datasheet???
	lcd_st7701_send_param(0x22, true);
	lcd_st7701_send_param(0x00, true);

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
	lcd_st7701_send_param(0x00, true);

	lcd_st7701_send_cmd(0xEB, true);		//Not in datasheet???
	lcd_st7701_send_param(0x00, true);
	lcd_st7701_send_param(0x00, true);
	lcd_st7701_send_param(0x40, true);
	lcd_st7701_send_param(0x40, true);
	lcd_st7701_send_param(0x00, true);
	lcd_st7701_send_param(0x00, true);
	lcd_st7701_send_param(0x00, true);

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
	lcd_st7701_send_param(0xFF, true);

	lcd_st7701_send_cmd(0xEF, true);		//Not in datasheet???
	lcd_st7701_send_param(0x10, true);
	lcd_st7701_send_param(0x0D, true);
	lcd_st7701_send_param(0x04, true);
	lcd_st7701_send_param(0x08, true);
	lcd_st7701_send_param(0x3F, true);
	lcd_st7701_send_param(0x1F, true);

	lcd_st7701_send_cmd(0xFF, true);		//Command2 BKx Selection.
	lcd_st7701_send_param(0x77, true);		//Nothing.
	lcd_st7701_send_param(0x01, true);		//Nothing.
	lcd_st7701_send_param(0x00, true);		//Nothing.
	lcd_st7701_send_param(0x00, true);		//Nothing.
	lcd_st7701_send_param(0x13, true);		//Selects BK3 setting.

	lcd_st7701_send_cmd(0xEF, true);		//Not in datasheet...
	lcd_st7701_send_param(0x08, true);

	lcd_st7701_send_cmd(0xFF, true);		//Command2 BKx Selection.
	lcd_st7701_send_param(0x77, true);		//Nothing.
	lcd_st7701_send_param(0x01, true);		//Nothing.
	lcd_st7701_send_param(0x00, true);		//Nothing.
	lcd_st7701_send_param(0x00, true);		//Nothing.
	lcd_st7701_send_param(0x00, true);		//Disable BK function of command 2.

	lcd_st7701_send_cmd(0x36, true);		//Display data access control. MADCTL
	lcd_st7701_send_param(0x00, true);		//Normal scan, RGB not BGR.

	lcd_st7701_send_cmd(0x3A, true);		//Interface pixel format (COLMOD).
	lcd_st7701_send_param(0x60, true);		//18 bits per pixel.

	lcd_st7701_send_cmd(0x11, true);		//Sleep out (SLPOUT).

	lcd_st7701_send_cmd(0x29, false);		//Display on (DISPON).
}
#endif //TARGET_HARDWARE_CANGAUGE

/******		GLOBAL FUNCTION DEFINITIONS		*******/
void lcd_init()
{
	//Initialize the ST7701 SPI driver.
#ifdef TARGET_HARDWARE_CANGAUGE
	io_set_output_speed(LTDC_RESET_io, GPIO_OSPEEDR_VERY_HIGH);
	io_set_pin_dir_out(LTDC_RESET_io);
	io_pin_out_clr(LTDC_RESET_io);
	vTaskDelay(20);
	io_pin_out_set(LTDC_RESET_io);
	vTaskDelay(500);
	lcd_st7701_init();
#endif	//TARGET_HARDWARE_CANGAUGE

	/*Enable pixel clock.*/
	rcc_enable_pll_3r();

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
	//LTDC_Layer1->CFBAR = (uint32_t)LTDC_DISP_BUFFER_ADDR;

	/*Set the buffer size registers.*/
	LTDC_Layer1->CFBLR = (LTDC_LxCFBLR_BUFFER_PITCH_Val << LTDC_LxCFBLR_CFBP_Pos)
						| (LTDC_LxCFBLR_LINE_LENGTH_Val << LTDC_LxCFBLR_CFBLL_Pos);
	LTDC_Layer1->CFBLNR = LTDC_LxCFBLNR_NUM_LINES_Val;

	/*Set color mode.*/
	LTDC_Layer1->PFCR = LTDC_LxPFCR_PF_RGB565;

	/*Enable layer 1.*/
	LTDC_Layer1->CR = LTDC_LxCR_LEN;

	/*Reload the layer registers.*/
	LTDC->SRCR = LTDC_SRCR_IMR;

	/*Enable register reload interrupt.*/
	LTDC->IER = LTDC_IER_RRIE;
	//NVIC_EnableIRQ(LTDC_IRQn);

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

	/*Enable the LCD.*/
	lcd_enable();

}

/*Enables the LTDC module and the LCD screen itself.*/
void lcd_enable()
{
#ifdef TARGET_HARDWARE_STM32H745DISCO
	io_pin_out_set(LTDC_DISP_io);
#endif //TARGET_HARDWARE_STM32H745DISCO

	LTDC->GCR |= LTDC_GCR_LTDCEN;
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

void lcd_solid_color_test_white()
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
		io_pin_out_set(LTDC_GREEN_DATA_2_io);
		io_pin_out_set(LTDC_GREEN_DATA_3_io);
		io_pin_out_set(LTDC_GREEN_DATA_4_io);
		io_pin_out_set(LTDC_GREEN_DATA_5_io);
		io_pin_out_set(LTDC_GREEN_DATA_6_io);
		io_pin_out_set(LTDC_GREEN_DATA_7_io);
		io_pin_out_set(LTDC_BLUE_DATA_2_io);
		io_pin_out_set(LTDC_BLUE_DATA_3_io);
		io_pin_out_set(LTDC_BLUE_DATA_4_io);
		io_pin_out_set(LTDC_BLUE_DATA_5_io);
		io_pin_out_set(LTDC_BLUE_DATA_6_io);
		io_pin_out_set(LTDC_BLUE_DATA_7_io);
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


void LTDC_IRQHandler()
{

}


