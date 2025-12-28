/**********     INCLUDES        **********/
#include "ui_gauges.h"
#include "ui_car_menu.h"
#include <stdbool.h>

/**********		DEFINES		**********/
#define NUMBER_OF_GAUGES		8

#define GAUGE_SELECT_CONTAINER_Y_POS	0
#define BACK_BTN_Y_POS					320
#define HIDDEN_LABEL_Y_POS				600
/**********		EXTERNAL VARIABLE DEFINITIONS		**********/

/**********		STATIC VARIABLES		**********/
static bool _is_init = false;

/*LVGL/UI variables.*/
static lv_obj_t* _main_scr;
static lv_obj_t* _gauge_select_btn_container;
static lv_obj_t* _gauge_select_btn[NUMBER_OF_GAUGES];
static const char* _gauge_select_btn_lbl[NUMBER_OF_GAUGES] = { "Coolant Temp", "Fuel Pressure", 
															"Intake Air Pressure", "Timing Advance", 
															"Intake Air Temp", "MAF Flow Rate", 
															"Fuel Rail Pressure", "Air/Fuel Ratio"};
static lv_obj_t* _back_btn;


/*All the info for the gauge currently being displayed.*/
static lv_obj_t* _gauge_scr;			//The screen being the parent to the lv_scale that is the gauge.
static lv_obj_t* _gauge;				//The lv_scale that is the gauge. 
static lv_obj_t* _gauge_needle;			//The lv_line that acts as the needle, set to -1 if not using a needle.
static lv_obj_t* _gauge_data_lbl;		//The label on the gauge face that displays the number on the gauge.
static lv_obj_t* _gauge_info_lbl;		//Label that tells the user about what data is being displayed.
static int32_t _gauge_min_value;		//The min value that can be displayed on the current gauge.
static int32_t _gauge_max_value;		//The max value that can be displayed on the current gauge.

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
static void (*_gauge_cb)(lv_event_t* e) = NULL;

/**********		STATIC FUNCTION DECLRATIONS		**********/
static void _init();
static void _gauge_select_btn_handler(lv_event_t* e);
static void _back_btn_handler(lv_event_t* e);
static void _gauge_hanlder(lv_event_t* e);
static void _gauge_anim_map(void* obj, uint32_t val);

static void _load_coolant_temp_gauge();
static void _load_fuel_pressure_gauge();
static void _load_intake_air_pressure_gauge();
static void _load_timing_advance_gauge();
static void _load_intake_air_temp_gauge();
static void _load_maf_flow_rate_gauge();
static void _load_fuel_rail_pressure_gauge();
static void _load_air_fuel_ratio_gauge();
static void _air_fuel_ratio_gauge_modifier(int32_t val);

/**********		STATIC FUNCTION DEFINITIONS		**********/
static void _init()
{
	/*MAIN SCREEN.*/
	_main_scr = lv_obj_create(NULL);
	lv_obj_set_style_bg_color(_main_scr, UI_COLOR_BLACK, LV_STATE_DEFAULT);
	
	/*BUTTON CONTAINER.*/
	_gauge_select_btn_container = lv_obj_create(_main_scr);
	lv_obj_align(_gauge_select_btn_container, LV_ALIGN_CENTER, 0, GAUGE_SELECT_CONTAINER_Y_POS);
	lv_obj_set_size(_gauge_select_btn_container, 300, 280);
	lv_obj_set_style_bg_color(_gauge_select_btn_container, UI_COLOR_GRAY, LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(_gauge_select_btn_container, 0, LV_STATE_DEFAULT);
	lv_obj_set_layout(_gauge_select_btn_container, LV_LAYOUT_FLEX);
	lv_obj_set_flex_flow(_gauge_select_btn_container, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_flex_align(_gauge_select_btn_container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_SPACE_EVENLY);

	/*GAUGE SELECT BUTTONS.*/
	for (uint8_t i = 0; i < NUMBER_OF_GAUGES; i++)
	{
		_gauge_select_btn[i] = ui_helpers_create_btn_with_text(_gauge_select_btn_container, _gauge_select_btn_lbl[i], LV_FONT_DEFAULT);
	}

	/*BACK BUTTON.*/
	_back_btn = ui_helpers_create_btn_with_text(_main_scr, "Back", LV_FONT_DEFAULT);
	lv_obj_align(_back_btn, LV_ALIGN_CENTER, 0, BACK_BTN_Y_POS);

	/*HIDDEN LABEL.*/
	lv_obj_t* hidden_lbl = lv_label_create(_main_scr);
	lv_obj_align(hidden_lbl, LV_ALIGN_CENTER, 0, HIDDEN_LABEL_Y_POS);

	/*GAUGE SCREEN.*/
	_gauge_scr = lv_obj_create(NULL);
	lv_obj_set_style_bg_color(_gauge_scr, UI_COLOR_BLACK, LV_PART_MAIN);


	/*Bind the controls and event function handlers.*/
	/*BACK BUTTON EVENT.*/
	lv_obj_add_event(_back_btn, _back_btn_handler, LV_EVENT_CLICKED, NULL);

	/*GAUGE SELECT BUTTONS EVENT.*/
	for (uint8_t i = 0; i < NUMBER_OF_GAUGES; i++)
	{
		lv_obj_add_event(_gauge_select_btn[i], _gauge_select_btn_handler, LV_EVENT_CLICKED, NULL);
	}
	
}

static void _gauge_select_btn_handler(lv_event_t* e)
{
	lv_obj_t* sender = lv_event_get_target_obj(e);
	const char* btn_txt = NULL;
	for (uint8_t i = 0; i < NUMBER_OF_GAUGES; i++)
	{
		if (_gauge_select_btn[i] == sender)
		{
			btn_txt = _gauge_select_btn_lbl[i];
			break;
		}
	}
	
	if (strcmp(btn_txt, "Coolant Temp") == 0)
	{
		_load_coolant_temp_gauge();
	}
	else if (strcmp(btn_txt, "Fuel Pressure") == 0)
	{
		_load_fuel_pressure_gauge();
	}
	else if (strcmp(btn_txt, "Intake Air Pressure") == 0)
	{
		_load_intake_air_pressure_gauge();
	}
	else if (strcmp(btn_txt, "Timing Advance") == 0)
	{
		_load_timing_advance_gauge();
	}
	else if (strcmp(btn_txt, "Intake Air Temp") == 0)
	{
		_load_intake_air_temp_gauge();
	}
	else if (strcmp(btn_txt, "MAF Flow Rate") == 0)
	{
		_load_maf_flow_rate_gauge();
	}
	else if (strcmp(btn_txt, "Fuel Rail Pressure") == 0)
	{
		_load_fuel_rail_pressure_gauge();
	}
	else if (strcmp(btn_txt, "Air/Fuel Ratio") == 0)
	{
		_load_air_fuel_ratio_gauge();
	}

	if (_gauge == NULL)
	{
		return;
	}

	lv_screen_load(_gauge_scr);
	lv_obj_add_event(_gauge, _gauge_hanlder, LV_EVENT_CLICKED, NULL);	//Bind the event to go back and clean the gauge if it's clicked.

	/*Check if there's a function CB assign and call it if there is.*/
	if (_gauge_select_btn_cb != NULL)
	{
		_gauge_select_btn_cb(e);
	}
}

static void _back_btn_handler(lv_event_t* e)
{
	lv_event_code_t event_code = lv_event_get_code(e);
	if (event_code == LV_EVENT_CLICKED)
	{
		ui_car_load_menu_screen();
	}

	/*Check if there's a function CB assign and call it if there is.*/
	if (_back_btn_cb != NULL)
	{
		_back_btn_cb(e);
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

		/*Check if there's a function CB assign and call it if there is.*/
		if (_gauge_cb != NULL)
		{
			_gauge_cb(e);
		}
	}
}

static void _gauge_anim_map(void* obj, uint32_t val)
{
	ui_gauges_set_gauge_value(val);
}

static void _load_coolant_temp_gauge()
{
	const int32_t min_val = -40;
	const int32_t max_val = 215;
	_gauge = ui_helpers_create_gauge(_gauge_scr, min_val, max_val, 270, 135, &_gauge_needle);
	_gauge_data_lbl = lv_label_create(_gauge);
	_gauge_info_lbl = lv_label_create(_gauge);
	lv_obj_align(_gauge_data_lbl, LV_ALIGN_CENTER, 0, 90);
	lv_obj_align(_gauge_info_lbl, LV_ALIGN_CENTER, 0, 150);
	lv_label_set_text(_gauge_data_lbl, "-40");
	lv_obj_set_style_text_font(_gauge_data_lbl, &lv_font_montserrat_26, LV_PART_MAIN);
	lv_label_set_text(_gauge_info_lbl, "Coolant Temp (C)");
	lv_obj_set_style_text_color(_gauge_data_lbl, UI_COLOR_WHITE, LV_PART_MAIN);
	lv_obj_set_style_text_color(_gauge_info_lbl, UI_COLOR_WHITE, LV_PART_MAIN);
	lv_obj_set_style_text_align(_gauge_data_lbl, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
	lv_obj_set_style_text_align(_gauge_info_lbl, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);

	if (ui_helpers_is_demo_mode())
	{
		ui_helpers_create_gauge_animation(&_gauge_demo_animation, _gauge, &_gauge_anim_map, 2250, min_val, max_val);
	}

}

static void _load_fuel_pressure_gauge()
{
	const int32_t min_val = 0;
	const int32_t max_val = 100;
	_gauge_min_value = min_val;
	_gauge_max_value = max_val;
	_gauge = ui_helpers_create_gauge(_gauge_scr, min_val, max_val, 270, 135, &_gauge_needle);
	_gauge_data_lbl = lv_label_create(_gauge);
	_gauge_info_lbl = lv_label_create(_gauge);
	lv_obj_align(_gauge_data_lbl, LV_ALIGN_CENTER, 0, 90);
	lv_obj_align(_gauge_info_lbl, LV_ALIGN_CENTER, 0, 150);
	lv_label_set_text(_gauge_data_lbl, "-0");
	lv_obj_set_style_text_font(_gauge_data_lbl, &lv_font_montserrat_26, LV_PART_MAIN);
	lv_label_set_text(_gauge_info_lbl, "Fuel Pressure (psi)");
	lv_obj_set_style_text_color(_gauge_data_lbl, UI_COLOR_WHITE, LV_PART_MAIN);
	lv_obj_set_style_text_color(_gauge_info_lbl, UI_COLOR_WHITE, LV_PART_MAIN);
	lv_obj_set_style_text_align(_gauge_data_lbl, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
	lv_obj_set_style_text_align(_gauge_info_lbl, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);

	if (ui_helpers_is_demo_mode())
	{
		ui_helpers_create_gauge_animation(&_gauge_demo_animation, _gauge, &_gauge_anim_map, 2250, min_val, max_val);
	}
}

static void _load_intake_air_pressure_gauge()
{
	const int32_t min_val = 0;
	const int32_t max_val = 255;
	_gauge_min_value = min_val;
	_gauge_max_value = max_val;
	_gauge = ui_helpers_create_gauge(_gauge_scr, min_val, max_val, 270, 135, &_gauge_needle);
	_gauge_data_lbl = lv_label_create(_gauge);
	_gauge_info_lbl = lv_label_create(_gauge);
	lv_obj_align(_gauge_data_lbl, LV_ALIGN_CENTER, 0, 90);
	lv_obj_align(_gauge_info_lbl, LV_ALIGN_CENTER, 0, 150);
	lv_label_set_text(_gauge_data_lbl, "0");
	lv_obj_set_style_text_font(_gauge_data_lbl, &lv_font_montserrat_26, LV_PART_MAIN);
	lv_label_set_text(_gauge_info_lbl, "Intake Manifold Pressure (psi)");
	lv_obj_set_style_text_color(_gauge_data_lbl, UI_COLOR_WHITE, LV_PART_MAIN);
	lv_obj_set_style_text_color(_gauge_info_lbl, UI_COLOR_WHITE, LV_PART_MAIN);
	lv_obj_set_style_text_align(_gauge_data_lbl, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
	lv_obj_set_style_text_align(_gauge_info_lbl, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
}

static void _load_timing_advance_gauge()
{
	const int32_t min_val = -64;
	const int32_t max_val = 64;
	_gauge_min_value = min_val;
	_gauge_max_value = max_val;
	_gauge = ui_helpers_create_gauge(_gauge_scr, -64, 64, 270, 135, &_gauge_needle);
	_gauge_data_lbl = lv_label_create(_gauge);
	_gauge_info_lbl = lv_label_create(_gauge);
	lv_obj_align(_gauge_data_lbl, LV_ALIGN_CENTER, 0, 90);
	lv_obj_align(_gauge_info_lbl, LV_ALIGN_CENTER, 0, 150);
	lv_label_set_text(_gauge_data_lbl, "0");
	lv_obj_set_style_text_font(_gauge_data_lbl, &lv_font_montserrat_26, LV_PART_MAIN);
	lv_label_set_text(_gauge_info_lbl, "Timing Advance\n(deg before TDC)");
	lv_obj_set_style_text_color(_gauge_data_lbl, UI_COLOR_WHITE, LV_PART_MAIN);
	lv_obj_set_style_text_color(_gauge_info_lbl, UI_COLOR_WHITE, LV_PART_MAIN);
	lv_obj_set_style_text_align(_gauge_data_lbl, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
	lv_obj_set_style_text_align(_gauge_info_lbl, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
}

static void _load_intake_air_temp_gauge()
{
	const int32_t min_val = -40;
	const int32_t max_val = 215;
	_gauge_min_value = min_val;
	_gauge_max_value = max_val;
	_gauge = ui_helpers_create_gauge(_gauge_scr, -40, 215, 270, 135, &_gauge_needle);
	_gauge_data_lbl = lv_label_create(_gauge);
	_gauge_info_lbl = lv_label_create(_gauge);
	lv_obj_align(_gauge_data_lbl, LV_ALIGN_CENTER, 0, 90);
	lv_obj_align(_gauge_info_lbl, LV_ALIGN_CENTER, 0, 150);
	lv_label_set_text(_gauge_data_lbl, "-40");
	lv_obj_set_style_text_font(_gauge_data_lbl, &lv_font_montserrat_26, LV_PART_MAIN);
	lv_label_set_text(_gauge_info_lbl, "Intake Air Temp (C)");
	lv_obj_set_style_text_color(_gauge_data_lbl, UI_COLOR_WHITE, LV_PART_MAIN);
	lv_obj_set_style_text_color(_gauge_info_lbl, UI_COLOR_WHITE, LV_PART_MAIN);
	lv_obj_set_style_text_align(_gauge_data_lbl, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
	lv_obj_set_style_text_align(_gauge_info_lbl, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
}

static void _load_maf_flow_rate_gauge()
{
	const int32_t min_val = 0;
	const int32_t max_val = 655;
	_gauge_min_value = min_val;
	_gauge_max_value = max_val;
	_gauge = ui_helpers_create_gauge(_gauge_scr, 0, 655, 270, 135, &_gauge_needle);
	_gauge_data_lbl = lv_label_create(_gauge);
	_gauge_info_lbl = lv_label_create(_gauge);
	lv_obj_align(_gauge_data_lbl, LV_ALIGN_CENTER, 0, 90);
	lv_obj_align(_gauge_info_lbl, LV_ALIGN_CENTER, 0, 150);
	lv_label_set_text(_gauge_data_lbl, "0");
	lv_obj_set_style_text_font(_gauge_data_lbl, &lv_font_montserrat_26, LV_PART_MAIN);
	lv_label_set_text(_gauge_info_lbl, "MAF Flow Rate (g/s)");
	lv_obj_set_style_text_color(_gauge_data_lbl, UI_COLOR_WHITE, LV_PART_MAIN);
	lv_obj_set_style_text_color(_gauge_info_lbl, UI_COLOR_WHITE, LV_PART_MAIN);
	lv_obj_set_style_text_align(_gauge_data_lbl, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
	lv_obj_set_style_text_align(_gauge_info_lbl, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
}

static void _load_fuel_rail_pressure_gauge()
{
	const int32_t min_val = 0;
	const int32_t max_val = 760;
	_gauge_min_value = min_val;
	_gauge_max_value = max_val;
	_gauge = ui_helpers_create_gauge(_gauge_scr, 0, 760, 270, 135, &_gauge_needle);
	_gauge_data_lbl = lv_label_create(_gauge);
	_gauge_info_lbl = lv_label_create(_gauge);
	lv_obj_align(_gauge_data_lbl, LV_ALIGN_CENTER, 0, 90);
	lv_obj_align(_gauge_info_lbl, LV_ALIGN_CENTER, 0, 150);
	lv_label_set_text(_gauge_data_lbl, "0");
	lv_obj_set_style_text_font(_gauge_data_lbl, &lv_font_montserrat_26, LV_PART_MAIN);
	lv_label_set_text(_gauge_info_lbl, "Fuel Rail Pressure (psi)");
	lv_obj_set_style_text_color(_gauge_data_lbl, UI_COLOR_WHITE, LV_PART_MAIN);
	lv_obj_set_style_text_color(_gauge_info_lbl, UI_COLOR_WHITE, LV_PART_MAIN);
	lv_obj_set_style_text_align(_gauge_data_lbl, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
	lv_obj_set_style_text_align(_gauge_info_lbl, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
}

static void _load_air_fuel_ratio_gauge()
{
	const int32_t min_val = 60;
	const int32_t max_val = 220;
	lv_obj_t* visible_gauge = ui_helpers_create_gauge(_gauge_scr, 6, 22, 180, 180, NULL);
	_gauge = ui_helpers_create_gauge(_gauge_scr, min_val, max_val, 180, 180, &_gauge_needle);

	/* Hide the hidden gauge elements. */
	lv_obj_set_style_arc_opa(_gauge, 0, LV_PART_MAIN);
	lv_obj_set_style_line_opa(_gauge, 0, LV_PART_INDICATOR);
	lv_obj_set_style_line_opa(_gauge, 0, LV_PART_ITEMS);
	lv_scale_set_label_show(_gauge, false);

	/* Create the labels. */
	_gauge_data_lbl = lv_label_create(visible_gauge);
	_gauge_info_lbl = lv_label_create(visible_gauge);
	lv_obj_align(_gauge_data_lbl, LV_ALIGN_CENTER, 0, 90);
	lv_obj_align(_gauge_info_lbl, LV_ALIGN_CENTER, 0, 150);
	lv_label_set_text(_gauge_data_lbl, "8");
	lv_obj_set_style_text_font(_gauge_data_lbl, &lv_font_montserrat_26, LV_PART_MAIN);
	lv_label_set_text(_gauge_info_lbl, "Air/Fuel Ratio");
	lv_obj_set_style_text_color(_gauge_data_lbl, UI_COLOR_WHITE, LV_PART_MAIN);
	lv_obj_set_style_text_color(_gauge_info_lbl, UI_COLOR_WHITE, LV_PART_MAIN);
	lv_obj_set_style_text_align(_gauge_data_lbl, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
	lv_obj_set_style_text_align(_gauge_info_lbl, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);

	if (ui_helpers_is_demo_mode())
	{
		ui_helpers_create_gauge_animation(&_gauge_demo_animation, _gauge, &_gauge_anim_map, 2250, min_val, max_val);
	}
	_gauge_value_modifier = _air_fuel_ratio_gauge_modifier;
}

static void _air_fuel_ratio_gauge_modifier(int32_t val)
{
	float val_float = (float)val / 10;
	lv_scale_set_line_needle_value(_gauge, _gauge_needle, 160, val);
	lv_label_set_text_fmt(_gauge_data_lbl, "%2.1f", val_float);
}

/**********		GLOBAL FUNCTION DEFINITIONS		**********/
void ui_gauges_load()
{
	if (_is_init == false)
	{
		_init();
		_is_init = true;
	}
	lv_scr_load(_main_scr);
}

void ui_gauges_set_gauge_value(int32_t val)
{
	if (_gauge_value_modifier != NULL)
	{
		_gauge_value_modifier(val);
		return;
	}

	if (_gauge == NULL)
	{
		return;
	}

	lv_scale_set_line_needle_value(_gauge, _gauge_needle, 160, val);
	lv_label_set_text_fmt(_gauge_data_lbl, "%d", (int)val);
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

