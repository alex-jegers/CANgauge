/**********     INCLUDES        **********/
#include "disp.h"

#include "lvgl/lvgl.h"

#include "common/cangauge_common.h"

#include "common/drivers/drivers.h"

/**********		DEFINES		**********/

/**********		EXTERNAL VARIABLE DEFINITIONS		**********/

/**********		STATIC VARIABLES		**********/
static lv_display_t* disp;
__attribute__((__section__(".ext_mem_ram"))) static uint8_t ltdc_lvgl_buffer1[LTDC_BUFFER_SIZE];
__attribute__((__section__(".ram_d1"))) static uint8_t ltdc_lvgl_buffer2[LTDC_BUFFER_SIZE];

/**********		STATIC FUNCTION DECLRATIONS		**********/
static void lcd_lvgl_disp_flush(lv_display_t* display, const lv_area_t* area, uint8_t* px_map);
static void disp_clean_dcache(lv_display_t* drv);

/**********		STATIC FUNCTION DEFINITIONS		**********/
static void lcd_lvgl_disp_flush(lv_display_t* display, const lv_area_t* area, uint8_t* px_map)
{
	/*Swap the active display pointer in the LTDC.*/
	volatile int32_t is_last = lv_display_flush_is_last(display);
	uint32_t addr = (uint32_t)lv_display_get_buf_active(display)->data;
	if (is_last == 1) {
		LTDC->ICR = LTDC_ICR_CRRIF;
		SCB_CleanInvalidateDCache();
		// wait for VSYNC to avoid tearing
		//while ((LTDC->CDSR & LTDC_CDSR_VSYNCS) == 0){}
		// swap framebuffers (NOTE: LVGL will swap the buffers in the background, so here we can set the LCD framebuffer to the current LVGL buffer, which has been just completed)
		LTDC_Layer1->CFBAR = addr;
		LTDC->SRCR = LTDC_SRCR_VBR;
		/*Tell LVGL the display flush is done.*/
		while ((LTDC->ISR & LTDC_ISR_RRIF) == 0)
		{

		}
	}

	lv_display_flush_ready(display);
}

static void disp_clean_dcache(lv_display_t* drv)
{
	SCB_CleanInvalidateDCache();
}

/**********		GLOBAL FUNCTION DEFINITIONS		**********/
void disp_init()
{
    /*Create the display object.*/
	disp = lv_display_create(LTDC_SCREEN_SIZE_X_px, LTDC_SCREEN_SIZE_Y_px);

	/*Set up the buffers.*/
	lv_display_set_buffers(disp, (void*)&ltdc_lvgl_buffer1, (void*)&ltdc_lvgl_buffer2, LTDC_SCREEN_SIZE_X_px * LTDC_SCREEN_SIZE_Y_px * LTDC_BYTES_PER_PIXEL, LV_DISPLAY_RENDER_MODE_FULL);

	/*Set the display flush callback.*/
	lv_display_set_flush_cb(disp, lcd_lvgl_disp_flush);
}



