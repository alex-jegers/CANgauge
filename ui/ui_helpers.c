
#include "ui_helpers.h"
#include "ui_loading.h"
#include "ui_gauges.h"
#include "ui_test.h"

bool ui_demo_mode = true;

static lv_style_t ui_btn_default_style;
static lv_style_t ui_btn_clicked_style;


void ui_assign_button_style(lv_obj_t* btn)
{
	/*Button default style.*/
	lv_style_init(&ui_btn_default_style);
	lv_style_set_border_color(&ui_btn_default_style, UI_COLOR_RED);
	lv_style_set_bg_color(&ui_btn_default_style, UI_COLOR_RED);

	lv_obj_add_style(btn, &ui_btn_default_style, LV_STATE_DEFAULT);
	lv_obj_add_style(btn, &ui_btn_clicked_style, LV_STATE_PRESSED);
}

void ui_gauge_assign_styles(ui_gauge_t* obj)
{
	lv_obj_set_style_bg_color(obj->gauge, UI_COLOR_BLACK, LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(obj->gauge, 0, LV_STATE_DEFAULT);

	lv_obj_set_style_border_width(obj->gauge, 8, LV_STATE_PRESSED);
	lv_obj_set_style_border_color(obj->gauge, UI_COLOR_TRUE_BLACK, LV_STATE_PRESSED);
	lv_obj_set_style_bg_color(obj->gauge, UI_COLOR_TRUE_BLACK, LV_STATE_PRESSED);

}

void ui_assign_checkbox_styles(lv_obj_t* checkbox)
{
	lv_obj_set_style_text_color(checkbox, UI_COLOR_WHITE, LV_STATE_DEFAULT);
	lv_obj_set_style_border_color(checkbox, UI_COLOR_WHITE, LV_PART_INDICATOR);
	lv_obj_set_style_bg_color(checkbox, UI_COLOR_WHITE, LV_PART_INDICATOR);

	lv_obj_set_style_border_color(checkbox, UI_COLOR_RED, LV_PART_INDICATOR | LV_STATE_CHECKED);
	lv_obj_set_style_bg_color(checkbox, UI_COLOR_RED, LV_PART_INDICATOR | LV_STATE_CHECKED);
}



