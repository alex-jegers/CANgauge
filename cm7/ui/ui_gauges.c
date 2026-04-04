/**********     INCLUDES        **********/
#include "ui_gauges.h"
#include "ui_car_menu.h"
#include <stdbool.h>
#include <stdio.h>

/**********		DEFINES		**********/
#define GAUGE_SELECT_CONTAINER_Y_POS	0
#define BACK_BTN_Y_POS					215
#define SETTINGS_BTN_Y_POS				300
#define HIDDEN_LABEL_Y_POS				600
/**********		EXTERNAL VARIABLE DEFINITIONS		**********/

/**********		STATIC VARIABLES		**********/
static bool prv_is_init = false;

/*LVGL/UI variables.*/
static lv_obj_t* _main_scr;
static lv_obj_t* _gauge_select_btn_container;
static lv_obj_t* _back_btn;
static lv_obj_t* prv_settings_btn;
static lv_obj_t* prv_other_scr;
static lv_obj_t* prv_settings_back_btn;
static lv_obj_t* prv_brightness_slider;
static lv_obj_t* prv_other_btn_container;


/*All the info for the gauge currently being displayed.*/
static lv_obj_t* _gauge_scr;			//The screen being the parent to the lv_scale that is the gauge.
static lv_obj_t* _gauge;				//The lv_scale that is the gauge. 
static lv_obj_t* _gauge_needle;			//The lv_line that acts as the needle, set to -1 if not using a needle.
static lv_obj_t* _gauge_data_lbl;		//The label on the gauge face that displays the number on the gauge.
static lv_obj_t* _gauge_info_lbl;		//Label that tells the user about what data is being displayed.
static float _gauge_scaling_factor;	//How the value is multiplied by when decimals are needed.
/**
Some gauges need their value modified to fit an alternate scale when the resolution of the 
scale is too small or the value uses decimal points. When we cant plug the value straight 
into the lv_scale object we call this function to modify it and put it on a hidden scale.
*/
static void (*_gauge_value_modifier)(int32_t val);

static lv_anim_t _gauge_demo_animation;	//Animation that runs in demo mode, used to test new gauges


/*Event function pointers.*/
static void (*_gauge_select_btn_cb)(lv_event_t* e) = NULL;
static void (*_back_btn_cb)(lv_event_t* e) = NULL;
static void (*settings_btn_cb)(lv_event_t* e) = NULL;
static void (*_gauge_cb)(lv_event_t* e) = NULL;
static lv_event_cb_t _scr_load_cb = NULL;
static lv_event_cb_t prv_slider_event_cb = NULL;
static lv_event_cb_t prv_settings_scr_load_event_cb = NULL;
static lv_event_cb_t prv_demo_mode_checkbox_event_cb = NULL;
static lv_event_cb_t prv_settings_back_btn_event_cb = NULL;

/**********		STATIC FUNCTION DECLRATIONS		**********/
static void _init();
static void _gauge_select_btn_handler(lv_event_t* e);
static void _back_btn_handler(lv_event_t* e);
static void prv_settings_btn_handler(lv_event_t* e);
static void _gauge_hanlder(lv_event_t* e);
static void _scr_load_handler(lv_event_t* e);
static void _gauge_anim_map(void* obj, int32_t val);
static void prv_init_other_screen();
static void prv_load_other_screen();
static void prv_create_brightness_slider();

static void prv_slider_event(lv_event_t* e);
static void prv_settings_scr_load_event(lv_event_t* e);
static void prv_settings_back_btn_event(lv_event_t* e);
static void prv_settings_demo_mode_checkbox_event(lv_event_t* e);


static void _load_gauge(int32_t min_val, int32_t max_val, const char* primary_lbl, const char* secondary_lbl);


/**********		STATIC FUNCTION DEFINITIONS		**********/
static void _init()
{
	/*MAIN SCREEN.*/
	_main_scr = lv_obj_create(NULL);
	lv_obj_set_style_bg_color(_main_scr, UI_COLOR_BLACK, LV_STATE_DEFAULT);
	
	/* Initialize the settings screen as well. */
	prv_init_other_screen(); 

	/*BUTTON CONTAINER.*/
	_gauge_select_btn_container = lv_obj_create(_main_scr);
	lv_obj_align(_gauge_select_btn_container, LV_ALIGN_CENTER, 0, GAUGE_SELECT_CONTAINER_Y_POS);
	lv_obj_set_size(_gauge_select_btn_container, 300, 280);
	lv_obj_set_style_bg_color(_gauge_select_btn_container, UI_COLOR_GRAY, LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(_gauge_select_btn_container, 0, LV_STATE_DEFAULT);
	lv_obj_set_layout(_gauge_select_btn_container, LV_LAYOUT_FLEX);
	lv_obj_set_flex_flow(_gauge_select_btn_container, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_flex_align(_gauge_select_btn_container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_SPACE_EVENLY);

	/*BACK BUTTON.*/
	_back_btn = ui_helpers_create_btn_with_text(_main_scr, "Back", LV_FONT_DEFAULT);
	lv_obj_align(_back_btn, LV_ALIGN_CENTER, 0, BACK_BTN_Y_POS);

	/*SETTINGS BUTTON.*/
	prv_settings_btn = ui_helpers_create_btn_with_text(_main_scr, "Settings", LV_FONT_DEFAULT);
	lv_obj_align(prv_settings_btn, LV_ALIGN_CENTER, 0, SETTINGS_BTN_Y_POS);

	/*HIDDEN LABEL.*/
	lv_obj_t* hidden_lbl = lv_label_create(_main_scr);
	lv_obj_align(hidden_lbl, LV_ALIGN_CENTER, 0, HIDDEN_LABEL_Y_POS);

	/*GAUGE SCREEN.*/
	_gauge_scr = lv_obj_create(NULL);
	lv_obj_set_style_bg_color(_gauge_scr, UI_COLOR_BLACK, LV_PART_MAIN);


	/*Bind the controls and event function handlers.*/
	/*BACK BUTTON EVENT.*/
	lv_obj_add_event(_back_btn, _back_btn_handler, LV_EVENT_CLICKED, NULL);
	lv_obj_add_event(_main_scr, _scr_load_handler, LV_EVENT_SCREEN_LOAD_START, NULL);
	lv_obj_add_event(prv_settings_btn, prv_settings_btn_handler, LV_EVENT_CLICKED, NULL);

	/* Check if were in demo mode and make some dummy buttons if we are. */
	if (ui_helpers_is_demo_mode())
	{
		ui_gauges_create_gauge_btn("Air/Fuel Ratio");
		ui_gauges_create_gauge_btn("Boost Pressure");
		ui_gauges_create_gauge_btn("Ignition Timing Angle");
	}
}

static void prv_init_other_screen()
{
	/* Init the screen. */
	prv_other_scr = lv_obj_create(NULL);
	lv_obj_set_style_bg_color(prv_other_scr, UI_COLOR_BLACK, LV_STATE_DEFAULT);
	lv_obj_add_event_cb(prv_other_scr, prv_settings_scr_load_event, LV_EVENT_SCREEN_LOADED, &prv_brightness_slider);

	/* Format the flex flow. */
	lv_obj_set_layout(prv_other_scr, LV_LAYOUT_FLEX);
	lv_obj_set_flex_flow(prv_other_scr, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_flex_align(prv_other_scr, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_SPACE_EVENLY);
	lv_obj_set_style_pad_top(prv_other_scr, 50, LV_PART_MAIN);
	lv_obj_set_style_pad_row(prv_other_scr, 30, LV_STATE_DEFAULT);

	lv_obj_t* settings_lbl = lv_label_create(prv_other_scr);
	lv_label_set_text(settings_lbl, "Settings");
	lv_obj_set_style_text_color(settings_lbl, UI_COLOR_WHITE, LV_PART_MAIN);
	lv_obj_set_style_text_font(settings_lbl, &lv_font_montserrat_28, LV_PART_MAIN);

	prv_create_brightness_slider();

	lv_obj_t* demo_mode_checkbox = lv_checkbox_create(prv_other_scr);
	lv_checkbox_set_text(demo_mode_checkbox, "Demo Mode");
	lv_obj_set_style_text_color(demo_mode_checkbox, UI_COLOR_WHITE, LV_STATE_DEFAULT);
	if (ui_helpers_is_demo_mode())
	{
		lv_obj_set_state(demo_mode_checkbox, LV_STATE_CHECKED, true);
	}
	else
	{
		lv_obj_set_state(demo_mode_checkbox, LV_STATE_CHECKED, false);
	}
	lv_obj_add_event(demo_mode_checkbox, prv_settings_demo_mode_checkbox_event, LV_EVENT_VALUE_CHANGED, NULL);


	/* Make a button to go back. */
	prv_settings_back_btn = ui_helpers_create_btn_with_text(prv_other_scr, "Back", LV_FONT_DEFAULT);
	lv_obj_add_event_cb(prv_settings_back_btn, prv_settings_back_btn_event, LV_EVENT_RELEASED, NULL);
}

void prv_load_other_screen()
{
	lv_scr_load(prv_other_scr);
}

static void _gauge_select_btn_handler(lv_event_t* e)
{	
	/*
	* Check if there's a function CB assign and call it if there is.
	* The external CB should be responsible for creating a gauge with
	* ui_gauges_create_gauge.
	* */
	if (_gauge_select_btn_cb != NULL)
	{
		_gauge_select_btn_cb(e);
	}
	
	/* Check to see if were in demo mode. */
	if (ui_helpers_is_demo_mode())
	{
		/* Get the button text. */
		lv_obj_t* btn = lv_event_get_target_obj(e);
		lv_obj_t* lbl = lv_obj_get_child(btn, 0);
		char* txt = lv_label_get_text(lbl);
		if (strcmp(txt, "Boost Pressure") == 0)
		{
			ui_gauges_create_gauge(txt, "PSI", - 30, 30);
		}
		if (strcmp(txt, "Air/Fuel Ratio") == 0)
		{
			ui_gauges_create_gauge(txt, "Lambda", 0, 2);
		}
		if (strcmp(txt, "Ignition Timing Angle") == 0)
		{
			ui_gauges_create_gauge(txt, "Degrees", - 64, 64);
		}
	}

	if (_gauge == NULL)
	{
		return;
	}

	lv_screen_load(_gauge_scr);
	lv_obj_add_event(_gauge, _gauge_hanlder, LV_EVENT_CLICKED, NULL);	//Bind the event to go back and clean the gauge if it's clicked.
}

static void _back_btn_handler(lv_event_t* e)
{
	lv_event_code_t event_code = lv_event_get_code(e);
	if (event_code == LV_EVENT_CLICKED)
	{
		ui_menu_load();
		lv_obj_delete_async(_main_scr);
		prv_is_init = false;
	}

	/*Check if there's a function CB assign and call it if there is.*/
	if (_back_btn_cb != NULL)
	{
		_back_btn_cb(e);
	}
}

static void prv_settings_btn_handler(lv_event_t* e)
{
	lv_event_code_t event_code = lv_event_get_code(e);
	if (event_code == LV_EVENT_CLICKED)
	{
		prv_load_other_screen();
		lv_obj_delete_async(_main_scr);
		prv_is_init = false;
	}

	/*Check if there's a function CB assign and call it if there is.*/
	if (settings_btn_cb != NULL)
	{
		settings_btn_cb(e);
	}
}

static void _gauge_hanlder(lv_event_t* e)
{
	lv_event_code_t event_code = lv_event_get_code(e);
	if (event_code == LV_EVENT_CLICKED)
	{
		ui_gauges_load();
		lv_obj_clean(_gauge_scr);
		_gauge_value_modifier = NULL;
		_gauge = NULL;

		lv_anim_delete_all();			//Delete all animations in case we're in demo mode.

		/*Check if there's a function CB assign and call it if there is.*/
		if (_gauge_cb != NULL)
		{
			_gauge_cb(e);
		}
	}
}

static void _scr_load_handler(lv_event_t* e)
{
	if (_scr_load_cb != NULL)
	{
		_scr_load_cb(e);
	}
}

static void _gauge_anim_map(void* obj, int32_t val)
{
	ui_gauges_set_gauge_value((float)val / _gauge_scaling_factor);
}

static void _load_gauge(int32_t min_val, int32_t max_val, const char* primary_lbl, const char* secondary_lbl)
{
	uint32_t number_of_ticks = max_val - min_val;
	uint32_t og_max = max_val;
	uint32_t og_min = min_val;
	_gauge_scaling_factor = 1.0;
	while (number_of_ticks < 100)
	{
		max_val *= 10;
		min_val *= 10;
		_gauge_scaling_factor *= 10;
		number_of_ticks = max_val - min_val;
	}

	while (number_of_ticks > 500)
	{
		max_val /= 10;
		min_val /= 10;
		_gauge_scaling_factor /= 10;
		number_of_ticks = max_val - min_val;
	}

	/* For values with decimal points or small range we need 2 gauges, one hidden that's scaled up and one visible
	with the values that were given. */
	if ((uint32_t)_gauge_scaling_factor > 1)
	{
		lv_obj_t* visible_gauge = ui_helpers_create_gauge(_gauge_scr, og_min, og_max, 270, 135, NULL);
		_gauge_data_lbl = lv_label_create(visible_gauge);
		_gauge_info_lbl = lv_label_create(visible_gauge);
		_gauge = ui_helpers_create_gauge(_gauge_scr, min_val, max_val, 270, 135, &_gauge_needle);
		lv_obj_set_style_arc_width(_gauge, 0, LV_PART_MAIN);
		lv_obj_set_style_line_width(_gauge, 0, LV_PART_INDICATOR);
		lv_obj_set_style_line_width(_gauge, 0, LV_PART_ITEMS);
		lv_scale_set_label_show(_gauge, false);
	}
	else
	{
		_gauge = ui_helpers_create_gauge(_gauge_scr, min_val, max_val, 270, 135, &_gauge_needle);
		_gauge_data_lbl = lv_label_create(_gauge);
		_gauge_info_lbl = lv_label_create(_gauge);
	}

	/* Make the label that shows the data. */
	lv_obj_align(_gauge_data_lbl, LV_ALIGN_CENTER, 0, 90);
	lv_label_set_text(_gauge_data_lbl, "");
	lv_obj_set_style_text_font(_gauge_data_lbl, &lv_font_montserrat_34, LV_PART_MAIN);
	lv_obj_set_style_text_color(_gauge_data_lbl, UI_COLOR_WHITE, LV_PART_MAIN);
	lv_obj_set_style_text_align(_gauge_data_lbl, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
	
	/* Make the label that says what is being displayed. */
	lv_obj_align(_gauge_info_lbl, LV_ALIGN_CENTER, 0, 140);
	lv_label_set_text(_gauge_info_lbl, primary_lbl);
	lv_label_set_long_mode(_gauge_info_lbl, LV_LABEL_LONG_MODE_SCROLL);
	lv_obj_set_width(_gauge_info_lbl, 200);
	lv_obj_set_style_text_font(_gauge_info_lbl, &lv_font_montserrat_24, LV_PART_MAIN);
	lv_obj_set_style_text_color(_gauge_info_lbl, UI_COLOR_WHITE, LV_PART_MAIN);
	lv_obj_set_style_text_align(_gauge_info_lbl, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
	
	/* Make the label that displays the units. */
	lv_obj_t* units_lbl = lv_label_create(_gauge);
	lv_label_set_text(units_lbl, secondary_lbl);
	lv_obj_set_style_text_font(units_lbl, &lv_font_montserrat_20, LV_PART_MAIN);
	lv_obj_set_style_text_color(units_lbl, UI_COLOR_WHITE, LV_PART_MAIN);
	lv_obj_set_style_text_align(units_lbl, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
	lv_obj_align(units_lbl, LV_ALIGN_CENTER, 0, 180);

	if (ui_helpers_is_demo_mode())
	{
		ui_helpers_create_gauge_animation(&_gauge_demo_animation, _gauge, &_gauge_anim_map, 2500, min_val, max_val + 1);
	}
}

static void prv_slider_event(lv_event_t* e)
{
	if (prv_slider_event_cb)
	{
		prv_slider_event_cb(e);
	}
}

static void prv_settings_scr_load_event(lv_event_t* e)
{
	if (prv_settings_scr_load_event_cb)
	{
		prv_settings_scr_load_event_cb(e);
	}
}

static void prv_settings_back_btn_event(lv_event_t* e)
{
	ui_gauges_load();
	if (prv_settings_back_btn_event_cb != NULL)
	{
		prv_settings_back_btn_event_cb(e);
	}
}

static void prv_settings_demo_mode_checkbox_event(lv_event_t* e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t* obj = lv_event_get_target_obj(e);
	lv_state_t state = lv_obj_get_state(obj);

	if (state & LV_STATE_CHECKED == LV_STATE_CHECKED)
	{
		ui_helpers_set_demo_mode(true);
	}
	else
	{
		ui_helpers_set_demo_mode(false);
	}

	if (prv_demo_mode_checkbox_event_cb != NULL)
	{
		prv_demo_mode_checkbox_event_cb(e);
	}
}

static void prv_create_brightness_slider()
{
	/* Container to hold label and slider. */
	lv_obj_t* container = lv_obj_create(prv_other_scr);
	lv_obj_set_size(container, 400, 120);
	lv_obj_set_style_bg_color(container, UI_COLOR_GRAY, LV_PART_MAIN);
	lv_obj_set_style_border_color(container, UI_COLOR_DARK_GRAY, LV_PART_MAIN);
	lv_obj_clear_flag(container, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_set_scrollbar_mode(container, LV_SCROLLBAR_MODE_OFF);

	/* Create and style the slider object. */
	prv_brightness_slider = lv_slider_create(container);
	lv_obj_align(prv_brightness_slider, LV_ALIGN_CENTER, 0, 15);
	lv_obj_set_width(prv_brightness_slider, 300);
	lv_obj_set_style_bg_color(prv_brightness_slider, UI_COLOR_RED, LV_PART_MAIN);
	lv_obj_set_style_bg_color(prv_brightness_slider, UI_COLOR_RED, LV_PART_KNOB);
	lv_obj_set_style_bg_color(prv_brightness_slider, UI_COLOR_RED, LV_PART_INDICATOR);

	/* Create and style the label. */
	lv_obj_t* lbl = lv_label_create(container);
	lv_obj_align(lbl, LV_ALIGN_CENTER, 0, -30);
	lv_obj_set_style_text_color(lbl, UI_COLOR_WHITE, LV_STATE_DEFAULT);
	lv_label_set_text(lbl, "Brightness");

	/* Bind the event callback. */
	lv_obj_add_event_cb(prv_brightness_slider, prv_slider_event, LV_EVENT_VALUE_CHANGED, NULL);
}


/**********		GLOBAL FUNCTION DEFINITIONS		**********/
void ui_gauges_load()
{
	if (prv_is_init == false)
	{
		_init();
		prv_is_init = true;
	}
	lv_scr_load(_main_scr);
	lv_obj_remove_event(_main_scr, 0);


}

void ui_gauges_set_gauge_value(float val)
{
	if (_gauge == NULL)
	{
		return;
	}

	lv_scale_set_line_needle_value(_gauge, _gauge_needle, 160, (int32_t)(val * _gauge_scaling_factor));
	if ((uint32_t)_gauge_scaling_factor == 100)
	{
		lv_label_set_text_fmt(_gauge_data_lbl, "%.2f", val);
	}
	else
	{
		lv_label_set_text_fmt(_gauge_data_lbl, "%.1f", val);
	}

}

void ui_gauges_create_gauge_btn(const char* name)
{
	lv_obj_t* btn = ui_helpers_create_btn_with_text(_gauge_select_btn_container, name, LV_FONT_DEFAULT);
	lv_obj_add_event(btn, _gauge_select_btn_handler, LV_EVENT_CLICKED, NULL);
}

void ui_gauges_create_gauge(const char* name, const char* units, uint32_t min, uint32_t max)
{
	_load_gauge(min, max, name, units);
}

void ui_gauges_set_gauge_select_btn_cb(void (*func)(lv_event_t* e))
{
	_gauge_select_btn_cb = func;
}

void ui_gauges_set_back_btn_cb(void (*func)(lv_event_t* e))
{
	_back_btn_cb = func;
}

void ui_gauges_set_gauge_cb(void (*func)(lv_event_t* e))
{
	_gauge_cb = func;
}

void ui_gauges_set_scr_load_cb(lv_event_cb_t func)
{
	_scr_load_cb = func;
}
