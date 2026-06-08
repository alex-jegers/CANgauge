/**********     INCLUDES        **********/
#include "disp.h"

#include "lvgl/lvgl.h"

#include "ui/ui_helpers.h"
#include "ui/ui_helpers.h"

#include "common/drivers/drivers.h"

#include "system/system_cm7.h"

/**********		DEFINES		**********/

/**********		EXTERNAL VARIABLE DEFINITIONS		**********/

/**********		STATIC VARIABLES		**********/
static lv_display_t* disp;
SYS_MEM_REGION_EXTERN_RAM static uint8_t ltdc_lvgl_buffer1[LTDC_BUFFER_SIZE];
SYS_MEM_REGION_RAM_D1 static uint8_t ltdc_lvgl_buffer2[LTDC_BUFFER_SIZE];

/**********		STATIC FUNCTION DECLRATIONS		**********/
static void lcd_lvgl_disp_flush(lv_display_t* display, const lv_area_t* area, uint8_t* px_map);

/**********		STATIC FUNCTION DEFINITIONS		**********/
static void lcd_lvgl_disp_flush(lv_display_t* display, const lv_area_t* area, uint8_t* px_map)
{
	/*Swap the active display pointer in the LTDC.*/
	volatile int32_t is_last = lv_display_flush_is_last(display);
	uint32_t addr = (uint32_t)lv_display_get_buf_active(display)->data;
	if (is_last == 1) {
		LTDC->ICR = LTDC_ICR_CRRIF;
#if SYS_ENABLE_CACHE == 1
		SCB_CleanDCache();
#endif
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

/**********		GLOBAL FUNCTION DEFINITIONS		**********/
void disp_init()
{
    /*Create the display object.*/
	disp = lv_display_create(LTDC_SCREEN_SIZE_X_px, LTDC_SCREEN_SIZE_Y_px);

	/*Set up the buffers.*/
	lv_display_set_buffers(disp, (void*)&ltdc_lvgl_buffer1, (void*)&ltdc_lvgl_buffer2, LTDC_SCREEN_SIZE_X_px * LTDC_SCREEN_SIZE_Y_px * LTDC_BYTES_PER_PIXEL, LV_DISPLAY_RENDER_MODE_DIRECT);

	/*Set the display flush callback.*/
	lv_display_set_flush_cb(disp, lcd_lvgl_disp_flush);


}



