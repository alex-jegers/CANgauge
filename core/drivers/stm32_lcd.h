 /*
 * stm32_lcd.h
 *
 *  Created on: Feb 17, 2024
 *      Author: awjpp
 */

#ifdef CORE_CM7

#ifndef INC_STM32_LCD_H_
#define INC_STM32_LCD_H_

#include "stm32h745xx.h"
#include "../lvgl/lvgl.h"

#ifdef TARGET_HARDWARE_STM32H745DISCO

#define LTDC_SCREEN_SIZE_X_px				480
#define LTDC_SCREEN_SIZE_Y_px				272
#define LTDC_SCREEN_AREA_px					LTDC_SCREEN_SIZE_X_px * LTDC_SCREEN_SIZE_Y_px
#define LTDC_BYTES_PER_PIXEL				4
#define LTDC_BUFFER_SIZE                    (LTDC_SCREEN_AREA_px * LTDC_BYTES_PER_PIXEL)

#define LTDC_DISP_BUFFER_ADDR               (0xD0000000)
#define LTDC_LVGL_BUFFER1_ADDR              (LTDC_DISP_BUFFER_ADDR + (LTDC_BUFFER_SIZE))	//0xD007F800
#define LTDC_LVGL_BUFFER2_ADDR              (LTDC_LVGL_BUFFER1_ADDR + (LTDC_BUFFER_SIZE))

#define LTDC_SSCR_VSYNC_HEIGHT_px			10
#define LTDC_SSCR_HSYNC_WIDTH_px			41
#define LTDC_BPCR_VERT_BK_PORCH_px			LTDC_SSCR_VSYNC_HEIGHT_px + 2		//12
#define LTDC_BPCR_HORZ_BK_PORCH_px			LTDC_SSCR_HSYNC_WIDTH_px + 2		//43
#define LTDC_AWCR_ACT_HEIGHT_px				LTDC_BPCR_VERT_BK_PORCH_px + LTDC_SCREEN_SIZE_Y_px 	//284
#define LTDC_AWCR_ACT_WIDTH_px				LTDC_BPCR_HORZ_BK_PORCH_px + LTDC_SCREEN_SIZE_X_px	//523
#define LTDC_TWCR_TOT_HEIGHT_px				LTDC_AWCR_ACT_HEIGHT_px + 4			//286
#define LTDC_TWCR_TOT_WIDTH_px				LTDC_AWCR_ACT_WIDTH_px + 8			//531
#define LTDC_GCR_DBW_Val					0x2
#define LTDC_GCR_DGW_Val					0x2
#define LTDC_GCR_DRW_Val					0x2
#define LTDC_BCCR_Val						0xffffff
#define LTDC_LxWHPCR_HORZ_START_Val			LTDC_BPCR_HORZ_BK_PORCH_px
#define LTDC_LxWHPCR_HORZ_STOP_Val			LTDC_AWCR_ACT_WIDTH_px
#define LTDC_LxWVPCR_VERT_START_Val			LTDC_BPCR_VERT_BK_PORCH_px
#define LTDC_LxWVPCR_VERT_STOP_Val			LTDC_AWCR_ACT_HEIGHT_px
#define LTDC_LxCACR_Val						0xFF
#define LTDC_LxBFCR_BF1_PXxCONST			0x6
#define LTDC_LxBFCR_BF2_PXxCONST			0x7
#define LTDC_LxCFBLR_BUFFER_PITCH_Val		LTDC_SCREEN_SIZE_X_px * LTDC_BYTES_PER_PIXEL
#define LTDC_LxCFBLR_LINE_LENGTH_Val		(LTDC_SCREEN_SIZE_X_px * LTDC_BYTES_PER_PIXEL)+7
#define LTDC_LxCFBLNR_NUM_LINES_Val			LTDC_SCREEN_SIZE_Y_px

#define LTDC_RED_DATA_0_io				    GPIOI, GPIO_PIN15_Msk
#define LTDC_RED_DATA_1_io                  GPIOJ, GPIO_PIN0_Msk
#define LTDC_RED_DATA_2_io                  GPIOJ, GPIO_PIN1_Msk
#define LTDC_RED_DATA_3_io                  GPIOH, GPIO_PIN9_Msk
#define LTDC_RED_DATA_4_io                  GPIOJ, GPIO_PIN3_Msk
#define LTDC_RED_DATA_5_io                  GPIOJ, GPIO_PIN4_Msk
#define LTDC_RED_DATA_6_io                  GPIOJ, GPIO_PIN5_Msk
#define LTDC_RED_DATA_7_io                  GPIOJ, GPIO_PIN6_Msk
#define LTDC_GREEN_DATA_0_io                GPIOJ, GPIO_PIN7_Msk
#define LTDC_GREEN_DATA_1_io                GPIOJ, GPIO_PIN8_Msk
#define LTDC_GREEN_DATA_2_io                GPIOJ, GPIO_PIN9_Msk
#define LTDC_GREEN_DATA_3_io                GPIOJ, GPIO_PIN10_Msk
#define LTDC_GREEN_DATA_4_io                GPIOJ, GPIO_PIN11_Msk
#define LTDC_GREEN_DATA_5_io                GPIOI, GPIO_PIN0_Msk
#define LTDC_GREEN_DATA_6_io                GPIOI, GPIO_PIN1_Msk
#define LTDC_GREEN_DATA_7_io                GPIOK, GPIO_PIN2_Msk
#define LTDC_BLUE_DATA_0_io                 GPIOJ, GPIO_PIN12_Msk
#define LTDC_BLUE_DATA_1_io                 GPIOJ, GPIO_PIN13_Msk
#define LTDC_BLUE_DATA_2_io                 GPIOJ, GPIO_PIN14_Msk
#define LTDC_BLUE_DATA_3_io                 GPIOJ, GPIO_PIN15_Msk
#define LTDC_BLUE_DATA_4_io                 GPIOK, GPIO_PIN3_Msk
#define LTDC_BLUE_DATA_5_io                 GPIOK, GPIO_PIN4_Msk
#define LTDC_BLUE_DATA_6_io                 GPIOK, GPIO_PIN5_Msk
#define LTDC_BLUE_DATA_7_io                 GPIOK, GPIO_PIN6_Msk
#define LTDC_CLK_io                         GPIOI, GPIO_PIN14_Msk
#define LTDC_DISP_io                        GPIOD, GPIO_PIN7_Msk
#define LTDC_HSYNC_io                       GPIOI, GPIO_PIN14_Msk                
#define LTDC_VSYNC_io                       GPIOI, GPIO_PIN9_Msk                                                                
#define LTDC_DE_io                          GPIOK, GPIO_PIN7_Msk                                                                           
#define LTDC_INT_io                         GPIOG, GPIO_PIN2_Msk                                       
#define LTDC_SCL_io                         GPIOD, GPIO_PIN12_Msk                    
#define LTDC_SDA_io                         GPIOD, GPIO_PIN13_Msk                    

#endif	//TARGET_HARDWARE_STM32H745DISCO


#ifdef TARGET_HARDWARE_CANGAUGE

#define LTDC_SCREEN_SIZE_X_px				480
#define LTDC_SCREEN_SIZE_Y_px				480
#define LTDC_SCREEN_AREA_px					(LTDC_SCREEN_SIZE_X_px * LTDC_SCREEN_SIZE_Y_px)
#define LTDC_BYTES_PER_PIXEL				4
#define LTDC_BUFFER_SIZE                    (LTDC_SCREEN_AREA_px * LTDC_BYTES_PER_PIXEL)

#define LTDC_DISP_BUFFER_ADDR             	0xD0000000
#define LTDC_LVGL_BUFFER1_ADDR              (LTDC_DISP_BUFFER_ADDR + (LTDC_BUFFER_SIZE))
#define LTDC_LVGL_BUFFER2_ADDR              (LTDC_LVGL_BUFFER1_ADDR + (LTDC_BUFFER_SIZE))

#define LTDC_SSCR_VSYNC_HEIGHT_px			5
#define LTDC_SSCR_HSYNC_WIDTH_px			22
#define LTDC_BPCR_VERT_BK_PORCH_px			(LTDC_SSCR_VSYNC_HEIGHT_px + 5)
#define LTDC_BPCR_HORZ_BK_PORCH_px			(LTDC_SSCR_HSYNC_WIDTH_px + 22)
#define LTDC_AWCR_ACT_HEIGHT_px				(LTDC_BPCR_VERT_BK_PORCH_px + LTDC_SCREEN_SIZE_Y_px)
#define LTDC_AWCR_ACT_WIDTH_px				(LTDC_BPCR_HORZ_BK_PORCH_px + LTDC_SCREEN_SIZE_X_px)
#define LTDC_TWCR_TOT_HEIGHT_px				(LTDC_AWCR_ACT_HEIGHT_px + 5)			//This adds on the front porch.
#define LTDC_TWCR_TOT_WIDTH_px				(LTDC_AWCR_ACT_WIDTH_px + 22)			//This adds on the front porch.
#define LTDC_VERT_FRNT_PORCH_px				(LTDC_TWCR_TOT_HEIGHT_px - LTDC_AWCR_ACT_HEIGHT_px)
#define LTDC_VERT_BK_PORCH_px				(LTDC_BPCR_VERT_BK_PORCH_px - LTDC_SSCR_VSYNC_HEIGHT_px)
#define LTDC_HORZ_FRNT_PORCH_px				(LTDC_TWCR_TOT_WIDTH_px - LTDC_AWCR_ACT_WIDTH_px)
#define LTDC_GCR_DBW_Val					0x2
#define LTDC_GCR_DGW_Val					0x2
#define LTDC_GCR_DRW_Val					0x2
#define LTDC_BCCR_Val						0xffffff
#define LTDC_LxWHPCR_HORZ_START_Val			LTDC_BPCR_HORZ_BK_PORCH_px
#define LTDC_LxWHPCR_HORZ_STOP_Val			LTDC_AWCR_ACT_WIDTH_px
#define LTDC_LxWVPCR_VERT_START_Val			LTDC_BPCR_VERT_BK_PORCH_px
#define LTDC_LxWVPCR_VERT_STOP_Val			LTDC_AWCR_ACT_HEIGHT_px
#define LTDC_LxCACR_Val						0xFF
#define LTDC_LxBFCR_BF1_PXxCONST			0x6
#define LTDC_LxBFCR_BF2_PXxCONST			0x7
#define LTDC_LxCFBLR_BUFFER_PITCH_Val		LTDC_SCREEN_SIZE_X_px * LTDC_BYTES_PER_PIXEL
#define LTDC_LxCFBLR_LINE_LENGTH_Val		(LTDC_SCREEN_SIZE_X_px * LTDC_BYTES_PER_PIXEL)+7
#define LTDC_LxCFBLNR_NUM_LINES_Val			LTDC_SCREEN_SIZE_Y_px


#define LTDC_RED_DATA_2_io                  GPIOA, GPIO_PIN1_Msk
#define LTDC_RED_DATA_3_io                  GPIOB, GPIO_PIN0_Msk
#define LTDC_RED_DATA_4_io                  GPIOA, GPIO_PIN5_Msk
#define LTDC_RED_DATA_5_io                  GPIOA, GPIO_PIN9_Msk
#define LTDC_RED_DATA_6_io                  GPIOA, GPIO_PIN8_Msk
#define LTDC_RED_DATA_7_io                  GPIOG, GPIO_PIN6_Msk
#define LTDC_GREEN_DATA_2_io                GPIOA, GPIO_PIN6_Msk
#define LTDC_GREEN_DATA_3_io                GPIOC, GPIO_PIN9_Msk
#define LTDC_GREEN_DATA_4_io                GPIOJ, GPIO_PIN11_Msk
#define LTDC_GREEN_DATA_5_io                GPIOK, GPIO_PIN0_Msk
#define LTDC_GREEN_DATA_6_io                GPIOC, GPIO_PIN7_Msk
#define LTDC_GREEN_DATA_7_io                GPIOD, GPIO_PIN3_Msk
#define LTDC_BLUE_DATA_2_io                 GPIOG, GPIO_PIN10_Msk
#define LTDC_BLUE_DATA_3_io                 GPIOG, GPIO_PIN11_Msk
#define LTDC_BLUE_DATA_4_io                 GPIOG, GPIO_PIN12_Msk
#define LTDC_BLUE_DATA_5_io                 GPIOI, GPIO_PIN1_Msk
#define LTDC_BLUE_DATA_6_io                 GPIOB, GPIO_PIN8_Msk
#define LTDC_BLUE_DATA_7_io                 GPIOB, GPIO_PIN9_Msk
#define LTDC_CLK_io                         GPIOG, GPIO_PIN7_Msk
#define LTDC_RESET_io                       GPIOC, GPIO_PIN10_Msk	//???
#define LTDC_HSYNC_io                       GPIOC, GPIO_PIN6_Msk
#define LTDC_VSYNC_io                       GPIOI, GPIO_PIN9_Msk
#define LTDC_DE_io                          GPIOF, GPIO_PIN10_Msk
#define LTDC_INT_io                         GPIOI, GPIO_PIN6_Msk
#define LTDC_SCL_io                         GPIOD, GPIO_PIN12_Msk
#define LTDC_SDA_io                         GPIOD, GPIO_PIN13_Msk
#define LCD_SPI_SCK							GPIOE, GPIO_PIN2_Msk
#define LCD_SPI_SS							GPIOE, GPIO_PIN4_Msk
#define LCD_SPI_MISO						GPIOE, GPIO_PIN5_Msk
#define LCD_SPI_MOSI						GPIOE, GPIO_PIN6_Msk

#define ST7701_SWRESET_CMD					0x01
#define ST7701_RDDID_CMD					0x04
#define ST7701_RDDPM_CMD					0x0A
#define ST7701_RDDPM_PARAM					0x9C
#define ST7701_RDDSDR_CMD					0x0F
#define ST7701_RDDSDR_PARAM					0xC0
#define ST7701_CND2BKxSEL_CMD				0xff
#define ST7701_CND2BKxSEL_PARAM1			0x77
#define ST7701_CND2BKxSEL_PARAM2			0x01
#define ST7701_CND2BKxSEL_PARAM3			0x00
#define ST7701_CND2BKxSEL_PARAM4			0x00
#define ST7701_CND2BKxSEL_ENABLE_BK0		0x10
#define ST7701_CND2BKxSEL_ENABLE_BK1		0x11
#define ST7701_CND2BKxSEL_DISABLE           0x00
#define ST7701_LNESET_CMD                   0xC0
#define ST7701_LNESET_LINE                  0x3B
#define ST7701_LNESET_LINE_DELTA            0x00
#define ST7701_PORCTRL_CMD                  0xC1
#define ST7701_PORCTRL_VBP                  0xB
#define ST7701_PORCTRL_VFP                  0x2
#define ST7701_COLCTRL_CMD                  0xCD
#define ST7701_COLCTRL_MDT                  0x80
#define ST7701_MADCTL_CMD                   0x36
#define ST7701_MADCTL_ML_NORMAL_SCAN        0x00
#define ST7701_MADCTL_BGR_RGB               0x00
#define ST7701_COLMOD_CMD                   0x3A
#define ST7701_COLMOD_VIPF_18BPP            0x60
#define ST7701_SLPOUT_CMD                   0x11
#define ST7701_ALLPOFF_CMD					0x22
#define ST7701_ALLPON_CMD					0x23
#define ST7701_DISPON_CMD                   0x29

#endif //TARGET_HARDWARE_CANGAUGE


void lcd_init();														//Configures the LTDC Peripheral.
void lcd_enable();														//Enables the LTDC.
void lcd_lvgl_init();													//Initializes LVGL and configures LVGL to run with the LTDC.
void lcd_solid_color_test_red();
void lcd_solid_color_test_green();
void lcd_solid_color_test_blue();
void lcd_solid_color_test_black();
void lcd_solid_color_test_inputs();

#endif /* INC_STM32_LCD_H_ */

#endif //CORE_CM7
