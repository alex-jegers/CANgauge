/**********     INCLUDES        **********/
#include "ui_helpers.h"
/**********		DEFINES		**********/

/**********		EXTERNAL VARIABLE DEFINITIONS		**********/
static bool ui_demo_mode = false;
/**********		STATIC VARIABLES		**********/

/**********		STATIC FUNCTION DECLRATIONS		**********/
static lv_event_cb_t ui_helpers_msgbox_close(lv_event_t* e);
/**********		STATIC FUNCTION DEFINITIONS		**********/
static lv_event_cb_t ui_helpers_msgbox_close(lv_event_t* e)
{
	lv_obj_t* btn = lv_event_get_target(e);
	lv_obj_t* footer = lv_obj_get_parent(btn);
	lv_obj_t* msgbox = lv_obj_get_parent(footer);
	void* func = lv_event_get_user_data(e);
	if (func != NULL)
	{
		((void(*)())func)();
	}
	lv_obj_delete(msgbox);
}
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
	lv_obj_t* temp_btn;
	lv_obj_t* temp_lbl;
	temp_btn = lv_button_create(parent);
	temp_lbl = lv_label_create(temp_btn);
	lv_label_set_text(temp_lbl, text);
	lv_obj_align(temp_lbl, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_style_text_font(temp_lbl, font, LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(temp_btn, UI_COLOR_RED, LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(temp_btn, UI_COLOR_GRAY, LV_STATE_PRESSED | LV_STATE_CHECKED | LV_STATE_FOCUSED);
	lv_obj_set_style_shadow_width(temp_btn, 0, LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(temp_btn, 12, LV_STATE_DEFAULT);
	lv_obj_set_style_pad_right(temp_btn, 12, LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(temp_btn, 16, LV_STATE_DEFAULT);
	lv_obj_set_style_pad_bottom(temp_btn, 16, LV_STATE_DEFAULT);

	return temp_btn;
}

lv_obj_t* ui_helpers_create_checkbox_with_text(lv_obj_t* parent, char* text, lv_font_t* font)
{
	lv_obj_t* temp_checkbox;
	temp_checkbox = lv_checkbox_create(parent);
	lv_checkbox_set_text(temp_checkbox, text);
	lv_obj_align(temp_checkbox, LV_ALIGN_LEFT_MID, 0, 0);
	lv_obj_set_style_text_color(temp_checkbox, UI_COLOR_WHITE, LV_STATE_DEFAULT);
	lv_obj_set_style_border_color(temp_checkbox, UI_COLOR_RED, LV_STATE_DEFAULT | LV_PART_INDICATOR);
	lv_obj_set_style_bg_color(temp_checkbox, UI_COLOR_RED, LV_STATE_CHECKED | LV_PART_INDICATOR);
	lv_obj_set_style_pad_left(temp_checkbox, 5, LV_PART_INDICATOR);
	lv_obj_set_style_pad_right(temp_checkbox, 5, LV_PART_INDICATOR);
	lv_obj_set_style_pad_top(temp_checkbox, 5, LV_PART_INDICATOR);
	lv_obj_set_style_pad_bottom(temp_checkbox, 5, LV_PART_INDICATOR);

	return temp_checkbox;
}

lv_obj_t* ui_helpers_create_gauge(lv_obj_t* parent, int32_t min_val, int32_t max_val, uint32_t angle_range, uint32_t angle_rotation, lv_obj_t** needle)
{
	uint32_t total_tick_count = abs(max_val - min_val);

	uint32_t major_tick_increment = 1000;

	if (angle_range < 90)
	{
		major_tick_increment = total_tick_count;
	}

	if (angle_range < 120)
	{
		major_tick_increment = total_tick_count / 2;
	}

	/**
	* If angle range is less than 120 the tick increment was already 
	* determined above so skip this loop.
	*/
	while (angle_range >= 120)
	{
		/* If were at 2, we cant go any lower so break.*/
		if (major_tick_increment == 1)
		{
			break;
		}
		/* If there is fewer than 5 ticks skip it, skip it. */
		if ((total_tick_count / major_tick_increment) < 5)
		{
			major_tick_increment--;
			continue;
		}

		/* If the increment is the same as the total amt of ticks, skip it. */
		if (major_tick_increment == total_tick_count)
		{
			major_tick_increment--;
			continue;
		}
		
		/* If it reaches here and the remainder is zero, exit the loop. */
		if (total_tick_count % major_tick_increment == 0)
		{
			break;
		}
		major_tick_increment--;
	}

	
	lv_obj_t* temp_gauge = lv_scale_create(parent);
	lv_obj_set_size(temp_gauge, 420, 420);
	lv_obj_align(temp_gauge, LV_ALIGN_CENTER, 0, 0);
	lv_scale_set_mode(temp_gauge, LV_SCALE_MODE_ROUND_INNER);
	lv_scale_set_label_show(temp_gauge, true);
	lv_scale_set_total_tick_count(temp_gauge, total_tick_count + 1);
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
	lv_obj_set_style_text_font(temp_gauge, &lv_font_montserrat_24, LV_PART_INDICATOR);

	/*Minor tick marks (just invisible).*/
	lv_obj_set_style_opa(temp_gauge, 0, LV_PART_ITEMS);
	lv_scale_set_range(temp_gauge, min_val, max_val);

	/*Scale rotation.*/
	lv_scale_set_angle_range(temp_gauge, angle_range);
	lv_scale_set_rotation(temp_gauge, angle_rotation);
	
	/*Needle, if were using one.*/
	if (needle != NULL)
	{
		/* Make the center circle. */
		/*
		lv_obj_t* center_circle = lv_obj_create(temp_gauge);
		lv_obj_set_size(center_circle, 50, 50);
		lv_obj_set_style_radius(center_circle, LV_RADIUS_CIRCLE, LV_PART_MAIN);
		lv_obj_center(center_circle);
		lv_obj_set_style_bg_color(center_circle, UI_COLOR_DARK_GRAY, LV_PART_MAIN);
		lv_obj_set_style_border_width(center_circle, 0, LV_PART_MAIN);
		lv_obj_set_style_border_color(center_circle, UI_COLOR_BLACK, LV_PART_MAIN);
		lv_obj_set_scrollbar_mode(center_circle, LV_SCROLLBAR_MODE_OFF);
		*/


		/* Add shadow to center circle. */
		/*
		lv_obj_set_style_shadow_color(center_circle, UI_COLOR_RED, LV_PART_MAIN);
		lv_obj_set_style_shadow_width(center_circle, 25, LV_PART_MAIN);
		lv_obj_set_style_shadow_opa(center_circle, 175, LV_PART_MAIN);
		lv_obj_set_style_shadow_spread(center_circle, 10, LV_PART_MAIN);
		*/

		/* Create the needle. */
		*needle = lv_line_create(temp_gauge);
		lv_obj_set_style_line_width(*needle, 8, LV_PART_MAIN);
		lv_obj_set_style_line_color(*needle, UI_COLOR_RED, LV_PART_MAIN);
		lv_scale_set_line_needle_value(temp_gauge, *needle, 200, min_val);
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

lv_obj_t* ui_helpers_add_text_to_act_scr(char* text, lv_align_t alignment, uint32_t x, uint32_t y)
{
	lv_obj_t* lbl = lv_label_create(lv_screen_active());
	lv_label_set_text(lbl, text);
	lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_CENTER, LV_STATE_DEFAULT);
	lv_obj_align(lbl, alignment, x, y);
	lv_obj_set_style_text_color(lbl, UI_COLOR_WHITE, LV_STATE_DEFAULT);
	return lbl;
}

lv_obj_t* ui_helpers_show_msgbox(const char* text, const char* btn_text, lv_event_cb_t func)
{
	lv_obj_t* msg_box = lv_msgbox_create(lv_layer_top());
	lv_msgbox_add_text(msg_box, text);
	//lv_msgbox_add_close_button(msg_box);


	if (btn_text != NULL)
	{
		lv_obj_t* user_btn = lv_msgbox_add_footer_button(msg_box, btn_text);
		if (func != NULL)
		{
			lv_obj_add_event_cb(user_btn, func, LV_EVENT_RELEASED, NULL);
		}
	}

	return msg_box;
}

void ui_helpers_add_msgbox_close_btn(lv_obj_t* msgbox, void (*func)())
{
	lv_obj_t* close_btn = lv_msgbox_add_footer_button(msgbox, "Close");
	lv_obj_add_event_cb(close_btn, ui_helpers_msgbox_close, LV_EVENT_RELEASED, (void*)func);
}

