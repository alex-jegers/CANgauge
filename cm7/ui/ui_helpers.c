/**********     INCLUDES        **********/
#include "ui_helpers.h"
/**********		DEFINES		**********/

/**********		EXTERNAL VARIABLE DEFINITIONS		**********/
static bool ui_demo_mode = true;
/**********		STATIC VARIABLES		**********/

/**********		STATIC FUNCTION DECLRATIONS		**********/

/**********		STATIC FUNCTION DEFINITIONS		**********/

/**********		GLOBAL FUNCTION DEFINITIONS		**********/
void ui_helpers_init_styles()
{

}

bool ui_helpers_is_demo_mode()
{
	return ui_demo_mode;
}
void ui_helpers_set_demo_mode(bool demo_mode)
{
	ui_demo_mode = demo_mode;
}

lv_obj_t* ui_helpers_create_btn_with_text(lv_obj_t* parent, char* text, lv_font_t* font)
{
	lv_obj_t* _temp_btn;
	lv_obj_t* _temp_lbl;
	_temp_btn = lv_button_create(parent);
	_temp_lbl = lv_label_create(_temp_btn);
	lv_label_set_text(_temp_lbl, text);
	lv_obj_align(_temp_lbl, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_style_text_font(_temp_lbl, font, LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(_temp_btn, UI_COLOR_RED, LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(_temp_btn, UI_COLOR_GRAY, LV_STATE_PRESSED | LV_STATE_CHECKED | LV_STATE_FOCUSED);
	lv_obj_set_style_shadow_width(_temp_btn, 0, LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(_temp_btn, 8, LV_STATE_DEFAULT);
	lv_obj_set_style_pad_right(_temp_btn, 8, LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(_temp_btn, 12, LV_STATE_DEFAULT);
	lv_obj_set_style_pad_bottom(_temp_btn, 12, LV_STATE_DEFAULT);

	return _temp_btn;
}

lv_obj_t* ui_helpers_create_gauge(lv_obj_t* parent, int32_t min_val, int32_t max_val, uint32_t angle_range, uint32_t angle_rotation, lv_obj_t** needle)
{
	uint32_t total_tick_count = max_val - min_val;						
	uint32_t major_tick_increment = (max_val - min_val) / (angle_range / 30);		//Major tick every 30 degrees.

	
	lv_obj_t* temp_gauge = lv_scale_create(parent);
	lv_obj_set_size(temp_gauge, 400, 400);
	lv_obj_align(temp_gauge, LV_ALIGN_CENTER, 0, 0);
	lv_scale_set_mode(temp_gauge, LV_SCALE_MODE_ROUND_INNER);
	lv_scale_set_label_show(temp_gauge, true);
	lv_scale_set_total_tick_count(temp_gauge, total_tick_count);
	lv_scale_set_major_tick_every(temp_gauge, major_tick_increment);

	/*Main arc style.*/
	lv_obj_set_style_arc_color(temp_gauge, UI_COLOR_WHITE, LV_PART_MAIN);
	lv_obj_set_style_arc_width(temp_gauge, 0, LV_PART_MAIN);

	/*Major tick marks and labels.*/
	lv_obj_set_style_line_color(temp_gauge, UI_COLOR_WHITE, LV_PART_INDICATOR);
	lv_obj_set_style_text_color(temp_gauge, UI_COLOR_WHITE, LV_PART_INDICATOR);
	lv_obj_set_style_length(temp_gauge, 20, LV_PART_INDICATOR);
	lv_obj_set_style_line_width(temp_gauge, 5, LV_PART_INDICATOR);
	lv_obj_set_style_line_rounded(temp_gauge, true, LV_PART_INDICATOR);
	lv_obj_set_style_translate_x(temp_gauge, -20, LV_PART_INDICATOR);
	lv_obj_set_style_text_font(temp_gauge, &lv_font_montserrat_16, LV_PART_INDICATOR);

	/*Minor tick marks (just invisible).*/
	lv_obj_set_style_length(temp_gauge, 0, LV_PART_ITEMS);
	lv_scale_set_range(temp_gauge, min_val, max_val);

	/*Scale rotation.*/
	lv_scale_set_angle_range(temp_gauge, angle_range);
	lv_scale_set_rotation(temp_gauge, angle_rotation);
	
	/*Needle, if were using one.*/
	if (needle != NULL)
	{
		lv_obj_t* center_circle = lv_obj_create(temp_gauge);
		lv_obj_set_size(center_circle, 50, 50);
		lv_obj_set_style_radius(center_circle, LV_RADIUS_CIRCLE, LV_PART_MAIN);
		lv_obj_center(center_circle);
		lv_obj_set_style_bg_color(center_circle, UI_COLOR_GRAY, LV_PART_MAIN);
		lv_obj_set_style_border_width(center_circle, 0, LV_PART_MAIN);

		*needle = lv_line_create(temp_gauge);
		lv_obj_set_style_line_width(*needle, 8, LV_PART_MAIN);
		lv_obj_set_style_line_color(*needle, UI_COLOR_RED, LV_PART_MAIN);
		lv_scale_set_line_needle_value(temp_gauge, *needle, 160, min_val);
	}
	return temp_gauge;
}

void ui_helpers_create_gauge_animation(lv_anim_t* anim, lv_obj_t* obj, lv_anim_exec_xcb_t func, uint32_t duration, uint32_t min, uint32_t max)
{
	lv_anim_init(anim);
	lv_anim_set_var(anim, obj);
	lv_anim_set_exec_cb(anim, func);
	lv_anim_set_duration(anim, duration);
	lv_anim_set_repeat_count(anim, LV_ANIM_REPEAT_INFINITE);
	lv_anim_set_reverse_duration(anim, duration);
	lv_anim_set_values(anim, min, max);
	lv_anim_start(anim);
}
