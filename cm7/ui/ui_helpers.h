
#ifndef _UI_HELPERS_H_
#define _UI_HELPERS_H_

#ifdef __cplusplus
extern "C" {
#endif

/**********     INCLUDES        **********/
#include "lvgl/lvgl.h"
/**********     DEFINES      **********/
#define UI_COLOR_RED		lv_color_hex(0xdb1616)
#define UI_COLOR_WHITE		lv_color_hex(0xfafafa)
#define UI_COLOR_BLACK		lv_color_hex(0x000000)
#define UI_COLOR_BLUE		lv_color_hex(0x0c65b3)
#define UI_COLOR_TRUE_BLACK	lv_color_hex(0x000000)
#define UI_COLOR_GRAY		lv_color_hex(0x3b3b3b)
#define UI_COLOR_DARK_GRAY	lv_color_hex(0x262626)

#define UI_BTN_FONT			&lv_font_montserrat_20

/**********     GLOBAL VARIABLE DECLRATIONS     **********/

/**********		GLOBAL FUNCTION DECLRATIONS		**********/
void ui_helpers_init_styles();
bool ui_helpers_is_demo_mode();
void ui_helpers_set_demo_mode(bool demo_mode);
lv_obj_t* ui_helpers_create_btn_with_text(lv_obj_t* parent, char* text, lv_font_t* font);
lv_obj_t* ui_helpers_create_checkbox_with_text(lv_obj_t* parent, char* text, lv_font_t* font);
lv_obj_t* ui_helpers_create_gauge(lv_obj_t* parent, int32_t min_val, int32_t max_val, uint32_t angle_range, uint32_t angle_rotation, lv_obj_t** needle);
void ui_helpers_create_gauge_animation(lv_anim_t* anim, lv_obj_t* obj, lv_anim_exec_xcb_t func, uint32_t duration, uint32_t min, uint32_t max);
lv_obj_t* ui_helpers_add_text_to_act_scr(char* text, lv_align_t alignment, uint32_t x, uint32_t y);
lv_obj_t* ui_helpers_show_msgbox(const char* text, const char* btn_text, lv_event_cb_t func);
void ui_helpers_add_msgbox_close_btn(lv_obj_t* msgbox, void (*func)());

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif  //_UI_HELPERS_H_
