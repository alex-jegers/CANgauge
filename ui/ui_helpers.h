


#ifndef _SQUARELINE_PROJECT_UI_HELPERS_H
#define _SQUARELINE_PROJECT_UI_HELPERS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl/lvgl.h"
#include "stdbool.h"


#define WINDOWS					0

#define UI_COLOR_RED		lv_color_hex(0xdb1616)
#define UI_COLOR_WHITE		lv_color_hex(0xfafafa)
#define UI_COLOR_BLACK		lv_color_hex(0x121212)
#define UI_COLOR_BLUE		lv_color_hex(0x0c65b3)
#define UI_COLOR_TRUE_BLACK	lv_color_hex(0x000000)
#define UI_COLOR_GRAY		lv_color_hex(0x3b3b3b)
#define UI_COLOR_DARK_GRAY	lv_color_hex(0x262626)

#if defined(TARGET_HARDWARE_CANGAUGE)
	#define UI_SIZE_X			480
	#define UI_SIZE_Y			480
#elif defined(TARGET_HARDWARE_STM32H745DISCO)
	#define UI_SIZE_X			300
	#define UI_SIZE_Y			300
#else
	#define UI_SIZE_X			300
	#define UI_SIZE_Y			300
#endif

extern bool ui_demo_mode;

typedef struct ui_gauge_t
{
	lv_obj_t* gauge;
	lv_obj_t* lbl;
	lv_meter_indicator_t* indic;
	lv_obj_t* checkbox;					//TODO: remove eventually.
	bool checkbox_saved_state;			//TODO: remove eventually.
	lv_obj_t* menu_btn;					//switching to this vs the checkboxes in Ver 3.0.
	lv_anim_t* animation;
	void (*set_value)(int32_t value);
}ui_gauge_t;


void ui_assign_button_style(lv_obj_t* btn);				//pass a button and it sets the style.
void ui_gauge_assign_styles(ui_gauge_t* obj);			//pass a gauge and it sets the gauge style.
void ui_assign_checkbox_styles(lv_obj_t* checkbox);		//pass a checkbox and it sets the style

#ifdef __cplusplus
} /*extern "C"*/
#endif	//__cplusplus

#endif //_SQUARELINE_PROJECT_UI_HELPERS_H


