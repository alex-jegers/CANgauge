

#include "ui_gauges.h"
#include <stdlib.h>
#include <math.h>
#include "components/ui_coolant_temp_symbol.h"
#include "ui_dtc_reader.h"
#include "stdbool.h"

/**********		DEFINES		**********/
#define UI_GAUGE_SIZE_PX		250

/**********		EXTERNAL VARIABLE DEFINITIONS		**********/
lv_obj_t* ui_main_screen;
lv_obj_t* ui_main_scr_panel;
lv_obj_t* ui_settings_btn_panel;
ui_gauge_t ui_coolant_temp;
ui_gauge_t ui_oil_level;
ui_gauge_t ui_boost_pressure;
ui_gauge_t ui_af_ratio;
ui_gauge_t* ui_gauge_list[] = { &ui_coolant_temp,
								&ui_boost_pressure,
								&ui_oil_level,
								&ui_af_ratio };


/**********		STATIC VARIABLES		**********/
void (*ui_gauge_clicked_cb)(lv_event_t* e) = NULL;
void (*ui_dtc_btn_clicked_cb)(lv_event_t* e) = NULL;
lv_obj_t* ui_load_dtc_btn;
static bool screen_initialized = false;

/**********		STATIC FUNCTION DECLRATIONS.		**********/
static void ui_coolant_temp_gauge_init(ui_gauge_t* obj);
static void ui_boost_gauge_init(ui_gauge_t* obj);
static void ui_oil_level_gauge_init(ui_gauge_t* obj);
static void ui_af_gauge_init(ui_gauge_t* obj);
static void ui_coolant_temp_anim_cb(void* indic, int32_t val);
static int32_t ui_coolant_temp_convert_value(uint32_t raw_can_value);
static void ui_boost_anim_cb(void* indic, int32_t val);
static int32_t ui_boost_convert_value(uint32_t raw_can_value);
static void ui_oil_level_anim_cb(void* indic, int32_t val);
static void ui_af_ratio_anim_cb(void* indic, int32_t val);
static int32_t ui_af_convert_value(uint32_t raw_can_val);
static void ui_gauge_clicked(lv_event_t* event_sender);
static void ui_assign_gauge_clicked_event(ui_gauge_t* obj);
static void ui_dtc_btn_clicked(lv_event_t* event_sender);
static void ui_settings_screen_init();

/**********		STATIC FUNCTION DEFINITIONS		**********/
static void ui_coolant_temp_gauge_init(ui_gauge_t* obj)
{
	/*Create a button for the menu screen.*/
	obj->menu_btn = lv_btn_create(ui_settings_btn_panel);
	lv_obj_t* menu_btn_lbl = lv_label_create(obj->menu_btn);
	lv_label_set_text(menu_btn_lbl, "Coolant Temp");
	lv_obj_align(menu_btn_lbl, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_width(obj->menu_btn, lv_pct(100));
	lv_obj_add_flag(obj->menu_btn, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_CLICK_FOCUSABLE);
	lv_obj_set_style_bg_color(obj->menu_btn, UI_COLOR_BLACK, LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(obj->menu_btn, UI_COLOR_GRAY, LV_STATE_PRESSED | LV_STATE_CHECKED |LV_STATE_FOCUSED);
	lv_obj_set_style_shadow_width(obj->menu_btn, 0, LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(obj->menu_btn, 2, LV_STATE_DEFAULT);
	lv_obj_set_style_border_color(obj->menu_btn, UI_COLOR_TRUE_BLACK, LV_STATE_DEFAULT);

	/*Create the actual gauge itself.*/
	obj->gauge = lv_meter_create(ui_main_scr_panel);
	lv_obj_center(obj->gauge);
	lv_obj_set_size(obj->gauge, UI_GAUGE_SIZE_PX, UI_GAUGE_SIZE_PX);
	lv_obj_add_flag(obj->gauge, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
	lv_obj_clear_flag(obj->gauge, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);

	/*Add a scale first*/
	lv_meter_scale_t* scale = lv_meter_add_scale(obj->gauge);
	lv_meter_set_scale_ticks(obj->gauge, scale, 21, 2, 10, lv_color_hex(0xfafafa));
	lv_meter_set_scale_major_ticks(obj->gauge, scale, 5, 4, 12, lv_color_hex(0xfafafa), 50);
	lv_meter_set_scale_range(obj->gauge, scale, 100, 260, 270, 135);
	lv_obj_set_style_text_font(obj->gauge, &lv_font_montserrat_14, LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(obj->gauge, lv_color_hex(0xfafafa), LV_STATE_DEFAULT);

	/*Add the hidden scale with more ticks = better resolution.*/
	lv_meter_scale_t* hidden_scale = lv_meter_add_scale(obj->gauge);
	lv_meter_set_scale_ticks(obj->gauge, hidden_scale, 1601, 0, 0, lv_color_hex(0x0000ff));
	lv_meter_set_scale_range(obj->gauge, hidden_scale, 1000, 2600, 270, 135);

	/*Add the red indicating arc*/
	obj->indic = lv_meter_add_arc(obj->gauge, hidden_scale, 25, lv_color_hex(0xdb1616), -15);
	lv_meter_set_indicator_start_value(obj->gauge, obj->indic, 0);
	lv_meter_set_indicator_end_value(obj->gauge, obj->indic, 60);
	lv_obj_remove_style(obj->gauge, NULL, LV_PART_INDICATOR);

	/*Add the symbol*/
	lv_obj_t* ui_coolant_temp_symbol_img = lv_img_create(obj->gauge);
	lv_img_set_src(ui_coolant_temp_symbol_img, &ui_coolant_temp_symbol);
	lv_obj_align(ui_coolant_temp_symbol_img, LV_ALIGN_BOTTOM_MID, 0, 0);

	/*Add a label in the middle to display the actual temperature.*/
	obj->lbl = lv_label_create(obj->gauge);
	lv_obj_set_style_text_font(obj->lbl, &lv_font_montserrat_30, LV_STATE_DEFAULT);
	lv_obj_align(obj->lbl, LV_ALIGN_CENTER, 0, 5);
	lv_obj_set_style_text_align(obj->lbl, LV_TEXT_ALIGN_CENTER, LV_STATE_DEFAULT);
	lv_label_set_text(obj->lbl, "0");

	/*Assign the default gauge styles.*/
	ui_gauge_assign_styles(obj);

	/*Assign the gauge clicked event.*/
	ui_assign_gauge_clicked_event(obj);

	/*Sets the function pointer for the function that sets the value of the gauge.*/
	obj->set_value = ui_coolant_temp_set_value;

	/*Hide the gauge as the default setting.*/
	lv_obj_add_flag(obj->gauge, LV_OBJ_FLAG_HIDDEN);

	if (ui_demo_mode)
	{
		obj->animation = (lv_anim_t*)lv_mem_alloc(sizeof(lv_anim_t));
		lv_anim_init(obj->animation);
		lv_anim_set_exec_cb(obj->animation, ui_coolant_temp_anim_cb);
		lv_anim_set_values(obj->animation, 1000, 2600);
		lv_anim_set_time(obj->animation, 2000);
		lv_anim_set_repeat_delay(obj->animation, 100);
		lv_anim_set_playback_time(obj->animation, 500);
		lv_anim_set_playback_delay(obj->animation, 100);
		lv_anim_set_repeat_count(obj->animation, LV_ANIM_REPEAT_INFINITE);
		lv_anim_start(obj->animation);
	}
}

static void ui_boost_gauge_init(ui_gauge_t* obj)
{
	/*Create a button for the menu screen.*/
	obj->menu_btn = lv_btn_create(ui_settings_btn_panel);
	lv_obj_t* menu_btn_lbl = lv_label_create(obj->menu_btn);
	lv_label_set_text(menu_btn_lbl, "Boost Pressure");
	lv_obj_align(menu_btn_lbl, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_width(obj->menu_btn, lv_pct(100));
	lv_obj_add_flag(obj->menu_btn, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_CLICK_FOCUSABLE);
	lv_obj_set_style_bg_color(obj->menu_btn, UI_COLOR_BLACK, LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(obj->menu_btn, UI_COLOR_GRAY, LV_STATE_PRESSED | LV_STATE_CHECKED | LV_STATE_FOCUSED);
	lv_obj_set_style_shadow_width(obj->menu_btn, 0, LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(obj->menu_btn, 2, LV_STATE_DEFAULT);
	lv_obj_set_style_border_color(obj->menu_btn, UI_COLOR_TRUE_BLACK, LV_STATE_DEFAULT);


	obj->gauge = lv_meter_create(ui_main_scr_panel);
	lv_obj_center(obj->gauge);
	lv_obj_set_size(obj->gauge, UI_GAUGE_SIZE_PX, UI_GAUGE_SIZE_PX);
	lv_obj_add_flag(obj->gauge, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
	lv_obj_clear_flag(obj->gauge, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);

	/*Add the positive pressure scale*/
	lv_meter_scale_t* positive_scale = lv_meter_add_scale(obj->gauge);
	lv_meter_set_scale_ticks(obj->gauge, positive_scale, 46, 2, 10, lv_color_hex(0xfafafa));
	lv_meter_set_scale_major_ticks(obj->gauge, positive_scale, 5, 4, 12, lv_color_hex(0xfafafa), 20);
	lv_meter_set_scale_range(obj->gauge, positive_scale, -15, 30, 270, 90);
	lv_obj_set_style_text_font(obj->gauge, &lv_font_montserrat_20, LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(obj->gauge, lv_color_hex(0xfafafa), LV_STATE_DEFAULT);

	/*Add the hidden scale used for indicating.*/
	lv_meter_scale_t* hidden_scale = lv_meter_add_scale(obj->gauge);
	lv_meter_set_scale_ticks(obj->gauge, hidden_scale, 451, 0, 0, lv_color_hex(0x0000ff));
	lv_meter_set_scale_range(obj->gauge, hidden_scale, -150, 300, 270, 90);

	obj->indic = lv_meter_add_scale_lines(obj->gauge, positive_scale, lv_color_hex(0x454545), lv_color_hex(0x454545), false, 0);
	lv_meter_set_indicator_start_value(obj->gauge, obj->indic, -15);
	lv_meter_set_indicator_end_value(obj->gauge, obj->indic, 0);

	/*Add the red indicator needle*/
	obj->indic = lv_meter_add_needle_line(obj->gauge, hidden_scale, 5, lv_color_hex(0xdb1616), -10);
	lv_obj_set_style_height(obj->gauge, 25, LV_PART_INDICATOR);
	lv_obj_set_style_width(obj->gauge, 25, LV_PART_INDICATOR);


	/*Add a label to display the actual pressure.*/
	obj->lbl = lv_label_create(obj->gauge);
	lv_obj_set_style_text_font(obj->lbl, &lv_font_montserrat_48, LV_STATE_DEFAULT);
	lv_obj_align(obj->lbl, LV_ALIGN_RIGHT_MID, -25, 40);
	lv_obj_set_style_text_align(obj->lbl, LV_TEXT_ALIGN_RIGHT, LV_STATE_DEFAULT);
	lv_label_set_text(obj->lbl, "0");
	lv_obj_t* psi_lbl = lv_label_create(obj->gauge);
	lv_obj_set_style_text_font(psi_lbl, &lv_font_montserrat_14, LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(psi_lbl, LV_TEXT_ALIGN_RIGHT, LV_STATE_DEFAULT);
	lv_obj_align(psi_lbl, LV_ALIGN_RIGHT_MID, -30, 65);
	lv_label_set_text(psi_lbl, "PSI");

	/*Assign the default gauge style.*/
	ui_gauge_assign_styles(obj);

	/*Assign default clicked event handler.*/
	ui_assign_gauge_clicked_event(obj);

	/*Set the function point for setting the gauge value.*/
	obj->set_value = ui_boost_set_value;

	/*Hide the gauge as the default setting.*/
	lv_obj_add_flag(obj->gauge, LV_OBJ_FLAG_HIDDEN);

	if (ui_demo_mode)
	{
		obj->animation = (lv_anim_t*)lv_mem_alloc(sizeof(lv_anim_t));
		lv_anim_init(obj->animation);
		lv_anim_set_exec_cb(obj->animation, ui_boost_anim_cb);
		lv_anim_set_values(obj->animation, -150, 300);
		lv_anim_set_time(obj->animation, 2000);
		lv_anim_set_repeat_delay(obj->animation, 100);
		lv_anim_set_playback_time(obj->animation, 500);
		lv_anim_set_playback_delay(obj->animation, 100);
		lv_anim_set_repeat_count(obj->animation, LV_ANIM_REPEAT_INFINITE);
		lv_anim_start(obj->animation);
	}

}

static void ui_oil_level_gauge_init(ui_gauge_t* obj)
{
	obj->menu_btn = lv_btn_create(ui_settings_btn_panel);
	lv_obj_t* menu_btn_lbl = lv_label_create(obj->menu_btn);
	lv_label_set_text(menu_btn_lbl, "Oil Level");
	lv_obj_align(menu_btn_lbl, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_width(obj->menu_btn, lv_pct(100));
	lv_obj_add_flag(obj->menu_btn, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_CLICK_FOCUSABLE);
	lv_obj_set_style_bg_color(obj->menu_btn, UI_COLOR_BLACK, LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(obj->menu_btn, UI_COLOR_GRAY, LV_STATE_PRESSED | LV_STATE_CHECKED | LV_STATE_FOCUSED);
	lv_obj_set_style_shadow_width(obj->menu_btn, 0, LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(obj->menu_btn, 2, LV_STATE_DEFAULT);
	lv_obj_set_style_border_color(obj->menu_btn, UI_COLOR_TRUE_BLACK, LV_STATE_DEFAULT);


	obj->gauge = lv_meter_create(ui_main_scr_panel);
	lv_obj_center(obj->gauge);
	lv_obj_set_size(obj->gauge, UI_GAUGE_SIZE_PX, UI_GAUGE_SIZE_PX);
	lv_obj_add_flag(obj->gauge, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
	lv_obj_clear_flag(obj->gauge, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);

	/*Add a scale first*/
	lv_meter_scale_t* scale = lv_meter_add_scale(obj->gauge);
	lv_meter_set_scale_ticks(obj->gauge, scale, 21, 2, 10, lv_color_hex(0xfafafa));
	lv_meter_set_scale_major_ticks(obj->gauge, scale, 2, 4, 12, lv_color_hex(0xfafafa), 50);
	lv_meter_set_scale_range(obj->gauge, scale, 0, 100, 270, 135);
	lv_obj_set_style_text_font(obj->gauge, &lv_font_montserrat_14, LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(obj->gauge, lv_color_hex(0xfafafa), LV_STATE_DEFAULT);

	/*Add the hidden scale with more ticks = better resolution.*/
	lv_meter_scale_t* hidden_scale = lv_meter_add_scale(obj->gauge);
	lv_meter_set_scale_ticks(obj->gauge, hidden_scale, 101, 0, 0, lv_color_hex(0x0000ff));
	lv_meter_set_scale_range(obj->gauge, hidden_scale, 0, 100, 270, 135);

	/*Add the red indicating arc*/
	obj->indic = lv_meter_add_scale_lines(obj->gauge, scale, UI_COLOR_RED, UI_COLOR_RED, true, 2);
	lv_meter_set_indicator_start_value(obj->gauge, obj->indic, 0);
	lv_meter_set_indicator_end_value(obj->gauge, obj->indic, 25);
	obj->indic = lv_meter_add_arc(obj->gauge, hidden_scale, 25, UI_COLOR_RED, -15);
	lv_meter_set_indicator_start_value(obj->gauge, obj->indic, 0);
	lv_meter_set_indicator_end_value(obj->gauge, obj->indic, 0);
	lv_obj_remove_style(obj->gauge, NULL, LV_PART_INDICATOR);

	/*Add the symbol*/
// 	lv_obj_t* ui_oil_level_symbol_img = lv_img_create(obj->gauge);
// 	lv_img_set_src(ui_oil_level_symbol_img, &ui_oil_level_symbol);
// 	lv_obj_align(ui_oil_level_symbol_img, LV_ALIGN_CENTER, 0, 100);

	/*Add a label in the middle to display the actual temperature.*/
	obj->lbl = lv_label_create(obj->gauge);
	lv_obj_set_style_text_font(obj->lbl, &lv_font_montserrat_20, LV_STATE_DEFAULT);
	lv_obj_align(obj->lbl, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_style_text_align(obj->lbl, LV_TEXT_ALIGN_CENTER, LV_STATE_DEFAULT);
	lv_label_set_text(obj->lbl, "0%");

	/*Assign the default gauge style.*/
	ui_gauge_assign_styles(obj);

	/*Assign default clicked event handler.*/
	ui_assign_gauge_clicked_event(obj);

	/*Set the function pointer for setting the gauge value.*/
	obj->set_value = ui_oil_level_set_value;

	/*Hide the gauge as the default setting.*/
	lv_obj_add_flag(obj->gauge, LV_OBJ_FLAG_HIDDEN);

	if (ui_demo_mode)
	{
		obj->animation = (lv_anim_t*)lv_mem_alloc(sizeof(lv_anim_t));
		lv_anim_init(obj->animation);
		lv_anim_set_exec_cb(obj->animation, ui_oil_level_anim_cb);
		lv_anim_set_values(obj->animation, 0, 1000);
		lv_anim_set_time(obj->animation, 2000);
		lv_anim_set_repeat_delay(obj->animation, 100);
		lv_anim_set_playback_time(obj->animation, 500);
		lv_anim_set_playback_delay(obj->animation, 100);
		lv_anim_set_repeat_count(obj->animation, LV_ANIM_REPEAT_INFINITE);
		//lv_anim_start(&obj->animation);
	}

}

static void ui_af_gauge_init(ui_gauge_t* obj)
{
	obj->menu_btn = lv_btn_create(ui_settings_btn_panel);
	lv_obj_t* menu_btn_lbl = lv_label_create(obj->menu_btn);
	lv_label_set_text(menu_btn_lbl, "Air/Fuel Ratio");
	lv_obj_align(menu_btn_lbl, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_width(obj->menu_btn, lv_pct(100));
	lv_obj_add_flag(obj->menu_btn, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_CLICK_FOCUSABLE);
	lv_obj_set_style_bg_color(obj->menu_btn, UI_COLOR_BLACK, LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(obj->menu_btn, UI_COLOR_GRAY, LV_STATE_PRESSED | LV_STATE_CHECKED | LV_STATE_FOCUSED);
	lv_obj_set_style_shadow_width(obj->menu_btn, 0, LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(obj->menu_btn, 2, LV_STATE_DEFAULT);
	lv_obj_set_style_border_color(obj->menu_btn, UI_COLOR_TRUE_BLACK, LV_STATE_DEFAULT);


	obj->gauge = lv_meter_create(ui_main_scr_panel);
	lv_obj_center(obj->gauge);
	lv_obj_set_size(obj->gauge, UI_GAUGE_SIZE_PX, UI_GAUGE_SIZE_PX);
	lv_obj_add_flag(obj->gauge, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
	lv_obj_clear_flag(obj->gauge, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);

	/*Add the positive pressure scale*/
	lv_meter_scale_t* positive_scale = lv_meter_add_scale(obj->gauge);
	lv_meter_set_scale_ticks(obj->gauge, positive_scale, 13, 2, 10, lv_color_hex(0xfafafa));
	lv_meter_set_scale_major_ticks(obj->gauge, positive_scale, 3, 4, 12, lv_color_hex(0xfafafa), 20);
	lv_meter_set_scale_range(obj->gauge, positive_scale, 8, 20, 225, 155);
	lv_obj_set_style_text_font(obj->gauge, &lv_font_montserrat_20, LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(obj->gauge, lv_color_hex(0xfafafa), LV_STATE_DEFAULT);

	/*Add the hidden scale used for indicating.*/
	lv_meter_scale_t* hidden_scale = lv_meter_add_scale(obj->gauge);
	lv_meter_set_scale_ticks(obj->gauge, hidden_scale, 121, 0, 0, lv_color_hex(0x0000ff));
	lv_meter_set_scale_range(obj->gauge, hidden_scale, 80, 200, 225, 155);
	lv_obj_remove_style(obj->gauge, NULL, LV_PART_INDICATOR);

	/*Add the red indicator bar.*/
	obj->indic = lv_meter_add_arc(obj->gauge, hidden_scale, 25, lv_color_hex(0xdb1616), 12);


	/*Add a label to display the actual pressure.*/
	obj->lbl = lv_label_create(obj->gauge);
	lv_obj_set_style_text_font(obj->lbl, &lv_font_montserrat_48, LV_STATE_DEFAULT);
	lv_obj_align(obj->lbl, LV_ALIGN_BOTTOM_MID, 0, -60);
	lv_obj_set_style_text_align(obj->lbl, LV_TEXT_ALIGN_CENTER, LV_STATE_DEFAULT);
	lv_label_set_text(obj->lbl, "0");
	lv_obj_t* psi_lbl = lv_label_create(obj->gauge);
	lv_obj_set_style_text_font(psi_lbl, &lv_font_montserrat_14, LV_STATE_DEFAULT);
	lv_obj_align(psi_lbl, LV_ALIGN_BOTTOM_MID, 0, -40);
	lv_label_set_text(psi_lbl, "A/F Ratio");

	lv_meter_set_indicator_end_value(obj->gauge, obj->indic, 140);

	/*Assign the default gauge style.*/
	ui_gauge_assign_styles(obj);

	/*Assign default clicked event handler.*/
	ui_assign_gauge_clicked_event(obj);

	/*Assign the function pointer for setting the gauge value.*/
	obj->set_value = ui_af_set_value;

	/*Hide the gauge as the default setting.*/
	lv_obj_add_flag(obj->gauge, LV_OBJ_FLAG_HIDDEN);

	if (ui_demo_mode)
	{
		obj->animation = (lv_anim_t*)lv_mem_alloc(sizeof(lv_anim_t));
		lv_anim_init(obj->animation);
		lv_anim_set_exec_cb(obj->animation, ui_af_ratio_anim_cb);
		lv_anim_set_values(obj->animation, 80, 200);
		lv_anim_set_time(obj->animation, 2000);
		lv_anim_set_repeat_delay(obj->animation, 100);
		lv_anim_set_playback_time(obj->animation, 500);
		lv_anim_set_playback_delay(obj->animation, 100);
		lv_anim_set_repeat_count(obj->animation, LV_ANIM_REPEAT_INFINITE);
	}
}

static void ui_settings_screen_init()
{
	/*Configure the container holding everything on the screen.*/
	lv_obj_t* ui_settings_panel = lv_obj_create(ui_main_scr_panel);
	lv_obj_set_size(ui_settings_panel, lv_pct(87), lv_pct(87));
	lv_obj_align(ui_settings_panel, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_style_bg_color(ui_settings_panel, UI_COLOR_BLACK, LV_STATE_DEFAULT);
	lv_obj_set_style_border_color(ui_settings_panel, UI_COLOR_BLACK, LV_STATE_DEFAULT);
	lv_obj_set_scrollbar_mode(ui_settings_panel, LV_SCROLLBAR_MODE_OFF);

	/*Configure the container holding the buttons.*/
	ui_settings_btn_panel = lv_obj_create(ui_settings_panel);
	lv_obj_set_size(ui_settings_btn_panel, lv_pct(82), lv_pct(77));
	lv_obj_align(ui_settings_btn_panel, LV_ALIGN_TOP_MID, 0, -20);
	lv_obj_set_flex_flow(ui_settings_btn_panel, LV_FLEX_FLOW_COLUMN_REVERSE);
	lv_obj_set_style_bg_color(ui_settings_btn_panel, UI_COLOR_GRAY, LV_STATE_DEFAULT);
	lv_obj_set_style_border_color(ui_settings_btn_panel, UI_COLOR_BLACK, LV_STATE_DEFAULT);
	lv_obj_add_flag(ui_settings_btn_panel, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_set_scrollbar_mode(ui_settings_btn_panel, LV_SCROLLBAR_MODE_OFF);
	lv_obj_set_style_pad_row(ui_settings_btn_panel, 5, LV_STATE_DEFAULT);

	/*Add a button to launch the code reader.*/
	ui_load_dtc_btn = lv_btn_create(ui_settings_panel);
	lv_obj_t* load_dtc_btn_lbl = lv_label_create(ui_load_dtc_btn);
	lv_label_set_text(load_dtc_btn_lbl, "DTC Reader");
	lv_obj_align(load_dtc_btn_lbl, LV_ALIGN_CENTER, 0, 0);
	lv_obj_align(ui_load_dtc_btn, LV_ALIGN_CENTER, 0, 100);
	lv_obj_add_flag(ui_load_dtc_btn, LV_OBJ_FLAG_CLICKABLE);
	lv_obj_set_style_bg_color(ui_load_dtc_btn, UI_COLOR_RED, LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui_load_dtc_btn, UI_COLOR_GRAY, LV_STATE_PRESSED | LV_STATE_CHECKED | LV_STATE_FOCUSED);
	lv_obj_set_style_shadow_width(ui_load_dtc_btn, 0, LV_STATE_DEFAULT);
	lv_obj_add_event_cb(ui_load_dtc_btn, ui_dtc_btn_clicked, LV_EVENT_PRESSED, NULL);
}

static void ui_coolant_temp_anim_cb(void* indic, int32_t val)
{
	ui_coolant_temp_set_value(val);
}

static int32_t ui_coolant_temp_convert_value(uint32_t raw_can_value)
{
	int32_t converted_value = (raw_can_value * 3) / 4;
	converted_value -= 48;
	converted_value *= 10;
	return converted_value;
}

static void ui_boost_anim_cb(void* indic, int32_t val)
{
	ui_boost_set_value(val);
}

static int32_t ui_boost_convert_value(uint32_t raw_can_value)
{
	float pres_kpa = (float)raw_can_value * (float)25.62857;
	float pres_psi = pres_kpa * (float).0145038;
	pres_psi -= (float)14.2;
	pres_psi *= 10;
	return (int32_t)pres_psi;
}

static void ui_oil_level_anim_cb(void* indic, int32_t val)
{
	ui_oil_level_set_value(val);
}

static void ui_af_ratio_anim_cb(void* indic, int32_t val)
{
	ui_af_set_value(val);
}

static int32_t ui_af_convert_value(uint32_t raw_can_val)
{
	float lambda = (float)raw_can_val / (float)4000.0;
	float air_fuel_ratio = lambda * (float)14.7;
	air_fuel_ratio *= (float)10.0;
	return (int32_t)air_fuel_ratio;
}

static void ui_gauge_clicked(lv_event_t* event_sender)
{
	ui_gauge_t* gauge_t = ui_get_gauge_from_lv_obj(event_sender->target);
	/*Show the of the button that was clicked and hide the rest.*/
	for (uint32_t i = 0; i < UI_NUMBER_OF_GAUGES; i++)
	{
		/*Hide all of gauges.*/
		ui_gauge_t* x = ui_gauge_list[i];
		lv_obj_add_flag(x->gauge, LV_OBJ_FLAG_HIDDEN);
	}
	lv_obj_clear_flag(gauge_t->gauge, LV_OBJ_FLAG_HIDDEN);

	/*If were in demo mode, start the animation.*/
	if (ui_demo_mode)
	{
		/*Stop all the animations.*/
		lv_anim_del_all();

		/*Start just the one animation.*/
		gauge_t = ui_get_gauge_from_lv_obj(event_sender->target);
		lv_anim_start(gauge_t->animation);
		return;
	}

	/*If the callback isn't null, call it.*/
	if (ui_gauge_clicked_cb != NULL)
	{
		ui_gauge_clicked_cb(event_sender);
	}
}

static void ui_assign_gauge_clicked_event(ui_gauge_t* obj)
{
	lv_obj_add_event_cb(obj->menu_btn, ui_gauge_clicked, LV_EVENT_PRESSED, NULL);
}

static void ui_dtc_btn_clicked(lv_event_t* event_sender)
{
	if (ui_demo_mode)
	{
		ui_load_dtc_reader();
		return;
	}

	if (ui_dtc_btn_clicked_cb != NULL)
	{
		ui_dtc_btn_clicked_cb(event_sender);
	}
}

/**********		GLOBAL FUNCTION DEFINITIONS.		**********/
void ui_init_gauge_screen(void)
{	
	/*Create the main screen, serves as the background for everything.*/
 	ui_main_screen = lv_obj_create(NULL);

	/*Create the main screen panel, serves as the container for gauges and menu.*/
	ui_main_scr_panel = lv_obj_create(ui_main_screen);
	lv_obj_set_size(ui_main_scr_panel, UI_SIZE_X, UI_SIZE_Y);
	lv_obj_set_style_bg_color(ui_main_scr_panel, UI_COLOR_BLACK, LV_STATE_DEFAULT);
	lv_obj_set_scroll_snap_x(ui_main_scr_panel, LV_SCROLL_SNAP_CENTER);
	lv_obj_set_flex_flow(ui_main_scr_panel, LV_FLEX_FLOW_ROW_REVERSE);
	lv_obj_set_style_pad_row(ui_main_scr_panel, 100, LV_STATE_DEFAULT);
	lv_obj_align(ui_main_scr_panel, LV_ALIGN_CENTER, 0, 0);
	lv_obj_update_snap(ui_main_scr_panel, LV_ANIM_OFF);
	lv_obj_set_scrollbar_mode(ui_main_scr_panel, LV_SCROLLBAR_MODE_OFF);

	/*Initialize the gauges and menu screen.*/
	ui_settings_screen_init();
	ui_coolant_temp_gauge_init(&ui_coolant_temp);
	ui_boost_gauge_init(&ui_boost_pressure);
	ui_af_gauge_init(&ui_af_ratio);
	ui_oil_level_gauge_init(&ui_oil_level);
}

void ui_load_gauge_screen()
{
	if (screen_initialized == false)
	{
		ui_init_gauge_screen();
		screen_initialized = true;
	}
	lv_disp_load_scr(ui_main_screen);
}

void ui_delete_gauge_screen()
{
	lv_obj_del(ui_main_screen);
}

void ui_coolant_temp_set_value(int32_t val)
{
	lv_meter_set_indicator_end_value(ui_coolant_temp.gauge, ui_coolant_temp.indic, val);
	val /= 10;
	lv_label_set_text_fmt(ui_coolant_temp.lbl, "%ld", val);
}

void ui_boost_set_value(int32_t val)
{
	if (val > 300)
	{
		val = 300;
	}
	lv_meter_set_indicator_value(ui_boost_pressure.gauge, ui_boost_pressure.indic, val);
	int32_t dec = abs(val % 10);
	int32_t num = val / 10;
	lv_label_set_text_fmt(ui_boost_pressure.lbl, "%d.%d%", num, dec);
}

void ui_oil_level_set_value(int32_t val)
{
	lv_meter_set_indicator_end_value(ui_oil_level.gauge, ui_oil_level.indic, val / 10);
	int32_t dec = val % 10;
	int32_t num = val / 10;
	lv_label_set_text_fmt(ui_oil_level.lbl, "%d.%d%%", num, dec);
}

void ui_af_set_value(int32_t val)
{
	int32_t dec = val % 10;
	int32_t num = val / 10;
	lv_label_set_text_fmt(ui_af_ratio.lbl, "%d.%d", num, dec);
	lv_meter_set_indicator_end_value(ui_af_ratio.gauge, ui_af_ratio.indic, val);
}


bool ui_get_saved_checkbox_state(ui_gauge_t* obj)
{
#if WINDOWS == 1
	return true;
#else // WINDOWS
	/*TODO: Code to actually fetch the saved state of the checkbox.*/
	return true;
#endif
}

ui_gauge_t* ui_get_gauge_from_lv_obj(lv_obj_t* obj)
{
	for (uint32_t i = 0; i < UI_NUMBER_OF_GAUGES; i++)
	{
		if (ui_gauge_list[i]->gauge == obj)
		{
			return ui_gauge_list[i];
		}

		if (ui_gauge_list[i]->lbl == obj)
		{
			return ui_gauge_list[i];
		}

		if (ui_gauge_list[i]->checkbox == obj)
		{
			return ui_gauge_list[i];
		}

		if (ui_gauge_list[i]->menu_btn == obj)
		{
			return ui_gauge_list[i];
		}
	}
	return NULL;
}

void ui_assign_gauge_clicked_cb(void (*func)(lv_event_t* e))
{
	ui_gauge_clicked_cb = func;
}

void ui_assign_dtc_reader_clicked_cb(void (*func)(lv_event_t* e))
{
	ui_dtc_btn_clicked_cb = func;
}



