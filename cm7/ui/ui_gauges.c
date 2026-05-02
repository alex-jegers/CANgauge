/**********     INCLUDES        **********/
#include "ui_gauges.h"
#include <stdbool.h>
#include <stdio.h>

/**********		DEFINES		**********/
#define GAUGE_SELECT_CONTAINER_Y_POS	0
#define VIEW_BTN_Y_POS					215
#define SETTINGS_BTN_Y_POS				285
#define HIDDEN_LABEL_Y_POS				600
/**********		EXTERNAL VARIABLE DEFINITIONS		**********/

/**********		STATIC VARIABLES		**********/
static bool prv_is_init = false;
static lv_obj_t* prv_gauge_select_checkboxes[4];		//Holds pointers to the gauge select checkboxes that are checked.
static uint8_t prv_num_gauges = 0;

/*LVGL/UI variables and their associated callback functions.*/
static lv_obj_t* _main_scr;

static lv_obj_t* prv_gauge_select_btn_container;
static lv_event_cb_t _gauge_select_btn_cb = NULL;
static void prv_gauge_select_checkbox_handler(lv_event_t* e);


static lv_obj_t* prv_settings_btn;
static lv_event_cb_t settings_btn_cb = NULL;
static void prv_settings_btn_handler(lv_event_t* e);

static lv_obj_t* prv_settings_screen;
static lv_event_cb_t prv_settings_scr_load_event_cb = NULL;
static void prv_settings_scr_load_event(lv_event_t* e);

static lv_obj_t* prv_settings_back_btn;
static lv_event_cb_t prv_settings_back_btn_event_cb = NULL;
static void prv_settings_back_btn_event(lv_event_t* e);

static lv_obj_t* prv_brightness_slider;
static lv_event_cb_t prv_slider_event_cb = NULL;
static void prv_slider_event(lv_event_t* e);
static void prv_slider_event(lv_event_t* e)		//Sends a point to prv_gauge_select_checkboxes in the user data.
{
	if (prv_slider_event_cb)
	{
		prv_slider_event_cb(e);
	}
}

/*** VIEW BUTTON. ***/
static lv_obj_t* prv_view_btn;
static lv_event_cb_t prv_view_btn_event_cb = NULL;
void ui_gauges_set_view_btn_cb(lv_event_cb_t func) { prv_view_btn_event_cb = func; }
static void prv_view_btn_event(lv_event_t* e);


/*All the info for the gauge currently being displayed.*/
static lv_obj_t* _gauge_scr;			//The screen being the parent to the lv_scale that is the gauge.
static lv_obj_t* _gauge[4];				//The lv_scale that is the gauge. 
static lv_obj_t* _gauge_needle[4];			//The lv_line that acts as the needle, set to -1 if not using a needle.
static lv_obj_t* _gauge_data_lbl[4];		//The label on the gauge face that displays the number on the gauge.
static lv_obj_t* _gauge_info_lbl[4];		//Label that tells the user about what data is being displayed.
static lv_obj_t* gauge_units_lbl[4];
static float _gauge_scaling_factor[4];	//How the value is multiplied by when decimals are needed.
static void (*prv_gauge_pressed_cb)(lv_event_t* e) = NULL;
static void prv_gauge_pressed_hanlder(lv_event_t* e);

static lv_anim_t _gauge_demo_animation;	//Animation that runs in demo mode, used to test new gauges


/*Event function pointers.*/

static lv_event_cb_t _scr_load_cb = NULL;

static lv_event_cb_t prv_demo_mode_checkbox_event_cb = NULL;

/**********		STATIC FUNCTION DECLRATIONS		**********/
static void _init();
static void _scr_load_handler(lv_event_t* e);
static void _gauge_anim_map(void* obj, int32_t val);
static void prv_init_settings_screen();
static void prv_load_settings_screen();
static void prv_create_brightness_slider();


static void prv_settings_demo_mode_checkbox_event(lv_event_t* e);


static void _load_gauge(int32_t min_val, int32_t max_val, const char* primary_lbl, const char* secondary_lbl, uint8_t gauge_idx);


/**********		STATIC FUNCTION DEFINITIONS		**********/
static void _init()
{
	/*MAIN SCREEN.*/
	_main_scr = lv_obj_create(NULL);
	lv_obj_set_style_bg_color(_main_scr, UI_COLOR_BLACK, LV_STATE_DEFAULT);
	
	/* Initialize the settings screen as well. */
	prv_init_settings_screen(); 

	/*BUTTON CONTAINER.*/
	prv_gauge_select_btn_container = lv_obj_create(_main_scr);
	lv_obj_align(prv_gauge_select_btn_container, LV_ALIGN_CENTER, 0, GAUGE_SELECT_CONTAINER_Y_POS);
	lv_obj_set_size(prv_gauge_select_btn_container, 340, 280);
	lv_obj_set_style_pad_left(prv_gauge_select_btn_container, 80, LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(prv_gauge_select_btn_container, UI_COLOR_GRAY, LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(prv_gauge_select_btn_container, 0, LV_STATE_DEFAULT);
	lv_obj_set_layout(prv_gauge_select_btn_container, LV_LAYOUT_FLEX);
	lv_obj_set_flex_flow(prv_gauge_select_btn_container, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_flex_align(prv_gauge_select_btn_container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_SPACE_EVENLY);


	/* VIEW BUTTON. */
	prv_view_btn = ui_helpers_create_btn_with_text(_main_scr, "View", LV_FONT_DEFAULT);
	lv_obj_align(prv_view_btn, LV_ALIGN_CENTER, 0, VIEW_BTN_Y_POS);
	lv_obj_add_event(prv_view_btn, prv_view_btn_event, LV_EVENT_RELEASED, &prv_gauge_select_checkboxes);

	/* SETTINGS BUTTON. */
	prv_settings_btn = ui_helpers_create_btn_with_text(_main_scr, "Settings", LV_FONT_DEFAULT);
	lv_obj_align(prv_settings_btn, LV_ALIGN_CENTER, 0, SETTINGS_BTN_Y_POS);
	lv_obj_add_event(prv_settings_btn, prv_settings_btn_handler, LV_EVENT_RELEASED, NULL);

	/* HIDDEN LABEL. */
	lv_obj_t* hidden_lbl = lv_label_create(_main_scr);
	lv_obj_align(hidden_lbl, LV_ALIGN_CENTER, 0, HIDDEN_LABEL_Y_POS);

	/* GAUGE SCREEN. */
	_gauge_scr = lv_obj_create(NULL);
	lv_obj_set_style_bg_color(_gauge_scr, UI_COLOR_BLACK, LV_PART_MAIN);


	/*Bind the controls and event function handlers.*/
	lv_obj_add_event(_main_scr, _scr_load_handler, LV_EVENT_SCREEN_LOAD_START, NULL);

	/*Create an array for the points of the line*/
	static lv_point_precise_t line_points[] = { {5, 5}, {70, 70}, {120, 10}, {180, 60}, {240, 10} };


	/* Check if were in demo mode and make some dummy buttons if we are. */
	if (ui_helpers_is_demo_mode())
	{
		ui_gauges_create_gauge_btn("Air/Fuel Ratio");
		ui_gauges_create_gauge_btn("Boost Pressure");
		ui_gauges_create_gauge_btn("Ignition Timing Angle");
	}
}

static void prv_init_settings_screen()
{
	/* Init the screen. */
	prv_settings_screen = lv_obj_create(NULL);
	lv_obj_set_style_bg_color(prv_settings_screen, UI_COLOR_BLACK, LV_STATE_DEFAULT);
	lv_obj_add_event_cb(prv_settings_screen, prv_settings_scr_load_event, LV_EVENT_SCREEN_LOADED, &prv_brightness_slider);

	/* Format the flex flow. */
	lv_obj_set_layout(prv_settings_screen, LV_LAYOUT_FLEX);
	lv_obj_set_flex_flow(prv_settings_screen, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_flex_align(prv_settings_screen, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_SPACE_EVENLY);
	lv_obj_set_style_pad_top(prv_settings_screen, 50, LV_PART_MAIN);
	lv_obj_set_style_pad_row(prv_settings_screen, 30, LV_STATE_DEFAULT);

	lv_obj_t* settings_lbl = lv_label_create(prv_settings_screen);
	lv_label_set_text(settings_lbl, "Settings");
	lv_obj_set_style_text_color(settings_lbl, UI_COLOR_WHITE, LV_PART_MAIN);
	lv_obj_set_style_text_font(settings_lbl, &lv_font_montserrat_28, LV_PART_MAIN);

	prv_create_brightness_slider();

	lv_obj_t* demo_mode_checkbox = lv_checkbox_create(prv_settings_screen);
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
	prv_settings_back_btn = ui_helpers_create_btn_with_text(prv_settings_screen, "Back", LV_FONT_DEFAULT);
	lv_obj_add_event_cb(prv_settings_back_btn, prv_settings_back_btn_event, LV_EVENT_RELEASED, NULL);
}

static void prv_view_btn_event(lv_event_t* e)
{
	if (prv_view_btn_event_cb != NULL)
	{
		prv_view_btn_event_cb(e);
	}

	if (prv_num_gauges == 0)
	{
		return;
	}

	if (_gauge != NULL)
	{
		lv_screen_load(_gauge_scr);
	}
}

void prv_load_settings_screen()
{
	lv_scr_load(prv_settings_screen);
}

static void prv_gauge_select_checkbox_handler(lv_event_t* e)
{
	lv_obj_t* checkbox = lv_event_get_target_obj(e);
	bool checked = lv_obj_has_state(checkbox, LV_STATE_CHECKED);

	static uint8_t selected_gauges_count = 0;
	if (checked)
	{
		prv_gauge_select_checkboxes[selected_gauges_count] = checkbox;
		selected_gauges_count++;
		if (selected_gauges_count == 4)
		{
			//Deactivate all the checkboxes.
			uint32_t child_idx = 0;
			lv_obj_t* child_checkbox = lv_obj_get_child(prv_gauge_select_btn_container, child_idx);
			while (child_checkbox != NULL)
			{
				/* If it's not checked, disable it.*/
				if (!lv_obj_has_state(child_checkbox, LV_STATE_CHECKED))
				{
					lv_obj_add_state(child_checkbox, LV_STATE_DISABLED);
				}
				child_idx++;
				child_checkbox = lv_obj_get_child(prv_gauge_select_btn_container, child_idx);
			}
		}
	}
	else
	{
		bool removed = false;
		for (uint8_t i = 0; i < selected_gauges_count; i++)
		{
			if (removed == true)
			{
				prv_gauge_select_checkboxes[i - 1] = prv_gauge_select_checkboxes[i];
			}
			if (prv_gauge_select_checkboxes[i] == checkbox)
			{
				prv_gauge_select_checkboxes[i] = NULL;
				removed = true;
			}
		}
		/* If there was 4 selected, reactivate all the checkboxes. */
		if (selected_gauges_count == 4)
		{
			uint32_t child_idx = 0;
			lv_obj_t* child_checkbox = lv_obj_get_child(prv_gauge_select_btn_container, child_idx);
			while (child_checkbox != NULL)
			{
				/* If it's not checked, disable it.*/		
				lv_obj_remove_state(child_checkbox, LV_STATE_DISABLED);
				child_idx++;
				child_checkbox = lv_obj_get_child(prv_gauge_select_btn_container, child_idx);
			}
		}
		prv_gauge_select_checkboxes[selected_gauges_count - 1] = NULL;
		selected_gauges_count--;
	}
}

static void prv_settings_btn_handler(lv_event_t* e)
{
	lv_event_code_t event_code = lv_event_get_code(e);
	if (event_code == LV_EVENT_RELEASED)
	{
		prv_load_settings_screen();
		lv_obj_delete_async(_main_scr);
		prv_is_init = false;
	}

	/*Check if there's a function CB assign and call it if there is.*/
	if (settings_btn_cb != NULL)
	{
		settings_btn_cb(e);
	}
}

static void prv_gauge_pressed_hanlder(lv_event_t* e)
{
	lv_event_code_t event_code = lv_event_get_code(e);
	if (event_code == LV_EVENT_RELEASED)
	{
		ui_gauges_load();
		lv_obj_clean(_gauge_scr);
		_gauge[0] = NULL;
		_gauge[1] = NULL;
		_gauge[2] = NULL;
		_gauge[3] = NULL;

		lv_anim_delete_all();			//Delete all animations in case we're in demo mode.

		prv_num_gauges = 0;

		/*Check if there's a function CB assign and call it if there is.*/
		if (prv_gauge_pressed_cb != NULL)
		{
			prv_gauge_pressed_cb(e);
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
	//ui_gauges_set_gauge_value((float)val / _gauge_scaling_factor);
}

static void _load_gauge(int32_t min_val, int32_t max_val, const char* primary_lbl, const char* secondary_lbl, uint8_t gauge_idx)
{
	/**
	 * Determine the scaling factor and if we need a second, hidden gauge.
	 * If gauges have less than 100 tick marks the needle motion isn't smooth going from tick to tick.
	 * If a gauge has more than 500 tick marks performance starts to degrade bc LVGL tries to render
	 * all 500 tick marks even if the minor ticks arent visible.
	 */
	int32_t number_of_ticks = abs(max_val - min_val);
	int32_t og_max = max_val;
	int32_t og_min = min_val;
	_gauge_scaling_factor[gauge_idx] = 1.0;
	while (number_of_ticks < 100)
	{
		max_val *= 10;
		min_val *= 10;
		_gauge_scaling_factor[gauge_idx] *= 10;
		number_of_ticks = abs(max_val - min_val);
	}

	while (number_of_ticks > 500)
	{
		max_val /= 10;
		min_val /= 10;
		_gauge_scaling_factor[gauge_idx] /= 10;
		number_of_ticks = abs(max_val - min_val);
	}

	/**
	 *  Determine the starting position and total angle span based on
	 * how many gauges there are going to be total and which one were
	 * creating right now.
	 */
	uint32_t span = 0;
	uint32_t start = 0;
	static uint8_t index = 0;
	switch (prv_num_gauges) {
		/* If there's only one gauge we dont need to check the index.*/
		case 1:
			span = 270;
			start = 135;
			break;
		case 2:
			span = 154;
			start = (index * 180) + 103;
			break;

		case 3: case 4:
			/** 
			*	Want:
			*	Index 0 --> Rotated 180.
			*	Index 1 --> Rotated 270.
			*	Index 2 --> Rotated 90.
			*	Index 3 --> Rotated 0.
			*/
			span = 60;
			if (gauge_idx == 0) { start = 180; }
			else if (gauge_idx == 1) { start = 270; }
			else if (gauge_idx == 2) { start = 90;  }
			else if (gauge_idx == 3) { start = 0;  }
			start += 15;
			break;

		default:
			break;
	}

	/* For values with decimal points or small range we need 2 gauges, one hidden that's scaled up and one visible
	with the values that were given. */
	if ((uint32_t)_gauge_scaling_factor[gauge_idx] > 1)
	{
		lv_obj_t* visible_gauge = ui_helpers_create_gauge(_gauge_scr, og_min, og_max, span, start, NULL);
		_gauge[gauge_idx] = ui_helpers_create_gauge(_gauge_scr, min_val, max_val, span, start, &_gauge_needle[gauge_idx]);
		lv_obj_set_style_arc_width(_gauge[gauge_idx], 0, LV_PART_MAIN);
		lv_obj_set_style_line_width(_gauge[gauge_idx], 0, LV_PART_INDICATOR);
		lv_obj_set_style_line_width(_gauge[gauge_idx], 0, LV_PART_ITEMS);
		lv_scale_set_label_show(_gauge[gauge_idx], false);
	}
	else
	{
		_gauge[gauge_idx] = ui_helpers_create_gauge(_gauge_scr, min_val, max_val, span, start, &_gauge_needle[gauge_idx]);
	}
	
	/* Set the event callback. */
	lv_obj_add_event_cb(_gauge[gauge_idx], prv_gauge_pressed_hanlder, LV_EVENT_RELEASED, NULL);

	/* Make the label that shows the data. */
	_gauge_data_lbl[gauge_idx] = lv_label_create(_gauge_scr);
	int32_t data_lbl_pos_y = 80;
	int32_t data_lbl_pos_x = 0;
	if (prv_num_gauges == 2)
	{
		data_lbl_pos_y = 0;
		if (gauge_idx == 0) { data_lbl_pos_x = -65; }
		else { data_lbl_pos_x = 65; }
	}
	else if (prv_num_gauges >= 3)
	{
		data_lbl_pos_x = -60;
		data_lbl_pos_y = -65;
		switch (gauge_idx)
		{
		case 0:
			break;
		case 1:
			data_lbl_pos_x *= -1;
			break;
		case 2:
			data_lbl_pos_y *= -1;
			break;
		case 3:
			data_lbl_pos_x *= -1;
			data_lbl_pos_y *= -1;
			break;
		}
	}
	lv_obj_align(_gauge_data_lbl[gauge_idx], LV_ALIGN_CENTER, data_lbl_pos_x, data_lbl_pos_y);
	lv_label_set_text(_gauge_data_lbl[gauge_idx], "");
	lv_obj_set_style_text_font(_gauge_data_lbl[gauge_idx], &lv_font_montserrat_34, LV_PART_MAIN);
	lv_obj_set_style_text_color(_gauge_data_lbl[gauge_idx], UI_COLOR_WHITE, LV_PART_MAIN);
	lv_obj_set_style_text_align(_gauge_data_lbl[gauge_idx], LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
	
	/* Make the label that says what is being displayed. */
	_gauge_info_lbl[gauge_idx] = lv_label_create(_gauge_scr);
	int32_t desc_lbl_pos_y = 140;
	int32_t desc_lbl_pos_x = 0;
	int32_t desc_lbl_width = 200;
	if (prv_num_gauges == 2)
	{
		desc_lbl_pos_y = 35;
		desc_lbl_width = 110;
		if (gauge_idx == 0) { desc_lbl_pos_x = -65; }
		else { desc_lbl_pos_x = 65; }
	}
	else if (prv_num_gauges >= 3)
	{
		desc_lbl_width = 200;
		desc_lbl_pos_x = -110;
		desc_lbl_pos_y = -15;
		switch (gauge_idx)
		{
		case 0:
			break;
		case 1:
			desc_lbl_pos_x *= -1;
			break;
		case 2:
			desc_lbl_pos_y *= -1;
			break;
		case 3:
			desc_lbl_pos_x *= -1;
			desc_lbl_pos_y *= -1;
			break;
		}
	}
	lv_obj_align(_gauge_info_lbl[gauge_idx], LV_ALIGN_CENTER, desc_lbl_pos_x, desc_lbl_pos_y);
	lv_label_set_text(_gauge_info_lbl[gauge_idx], primary_lbl);
	lv_label_set_long_mode(_gauge_info_lbl[gauge_idx], LV_LABEL_LONG_MODE_SCROLL);
	lv_obj_set_width(_gauge_info_lbl[gauge_idx], desc_lbl_width);
	lv_obj_set_style_text_font(_gauge_info_lbl[gauge_idx], &lv_font_montserrat_24, LV_PART_MAIN);
	lv_obj_set_style_text_color(_gauge_info_lbl[gauge_idx], UI_COLOR_WHITE, LV_PART_MAIN);
	lv_obj_set_style_text_align(_gauge_info_lbl[gauge_idx], LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
	lv_obj_move_foreground(_gauge_info_lbl[gauge_idx]);

	/* Make the label that displays the units. */
	gauge_units_lbl[gauge_idx] = lv_label_create(_gauge_scr);
	int32_t units_lbl_pos_y = 180;
	int32_t units_lbl_pos_x = 0;

	if (prv_num_gauges == 2)
	{
		units_lbl_pos_y = 60;
		if (gauge_idx == 0)
		{
			units_lbl_pos_x = -65;

		}
		else
		{
			units_lbl_pos_x = 65;
		}
	}
	else if (prv_num_gauges >= 3)
	{
		units_lbl_pos_x = -75;
		units_lbl_pos_y = -35;
		switch (gauge_idx)
		{
		case 0:
			break;
		case 1:
			units_lbl_pos_x *= -1;
			break;
		case 2:
			units_lbl_pos_y *= -1;
			break;
		case 3:
			units_lbl_pos_x *= -1;
			units_lbl_pos_y *= -1;
			break;
		}
	}
	lv_label_set_text(gauge_units_lbl[gauge_idx], secondary_lbl);
	lv_obj_set_style_text_font(gauge_units_lbl[gauge_idx], &lv_font_montserrat_20, LV_PART_MAIN);
	lv_obj_set_style_text_color(gauge_units_lbl[gauge_idx], UI_COLOR_WHITE, LV_PART_MAIN);
	lv_obj_set_style_text_align(gauge_units_lbl[gauge_idx], LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
	lv_obj_align(gauge_units_lbl[gauge_idx], LV_ALIGN_CENTER, units_lbl_pos_x, units_lbl_pos_y);

	index++;
	if (gauge_idx == prv_num_gauges - 1)
	{
		/* Make a circle the same color as the background to cover up the inside of the needles. */
		if (gauge_idx == prv_num_gauges - 1)
		{
			lv_obj_t* circle = lv_obj_create(_gauge_scr);
			lv_obj_set_size(circle, 250, 250);
			lv_obj_center(circle);
			lv_obj_set_style_radius(circle, LV_RADIUS_CIRCLE, 0);
			lv_obj_set_style_bg_color(circle, lv_obj_get_style_bg_color(lv_screen_active(), LV_PART_MAIN), 0);
			lv_obj_set_style_bg_opa(circle, LV_OPA_COVER, 0);
			lv_obj_set_style_border_width(circle, 0, LV_PART_MAIN);
			/* Move all the labels in front. */
			for (uint8_t x = 0; x < prv_num_gauges; x++)
			{
				lv_obj_move_foreground(_gauge_info_lbl[x]);
				lv_obj_move_foreground(_gauge_data_lbl[x]);
				lv_obj_move_foreground(gauge_units_lbl[x]);
			}
		}
		/* Draw a line or 2 to divide the gauges. */
		if (prv_num_gauges >= 2)
		{

			lv_obj_t* divider_line = lv_obj_create(_gauge_scr);
			lv_obj_set_size(divider_line, 7, 400);
			lv_obj_set_style_bg_color(divider_line, UI_COLOR_GRAY, LV_PART_MAIN);
			lv_obj_set_style_border_color(divider_line, UI_COLOR_GRAY, LV_PART_MAIN);
			lv_obj_center(divider_line);
		}
		if (prv_num_gauges >= 3)
		{

			lv_obj_t* divider_line = lv_obj_create(_gauge_scr);
			lv_obj_set_size(divider_line, 400, 7);
			lv_obj_set_style_bg_color(divider_line, UI_COLOR_GRAY, LV_PART_MAIN);
			lv_obj_set_style_border_color(divider_line, UI_COLOR_GRAY, LV_PART_MAIN);
			lv_obj_center(divider_line);
		}
		index = 0;
	}

	if (ui_helpers_is_demo_mode())
	{
		ui_helpers_create_gauge_animation(&_gauge_demo_animation, _gauge[gauge_idx], &_gauge_anim_map, 2500, min_val, max_val + 1);
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
	lv_obj_t* container = lv_obj_create(prv_settings_screen);
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

void ui_gauges_set_gauge_value(float val, uint8_t idx)
{
	if (_gauge[idx] == NULL)
	{
		return;
	}

	lv_scale_set_line_needle_value(_gauge[idx], _gauge_needle[idx], 200, (int32_t)(val * _gauge_scaling_factor[idx]));
	if ((uint32_t)_gauge_scaling_factor[idx] == 100)
	{
		lv_label_set_text_fmt(_gauge_data_lbl[idx], "%.2f", val);
	}
	else
	{
		lv_label_set_text_fmt(_gauge_data_lbl[idx], "%.1f", val);
	}

}

void ui_gauges_create_gauge_btn(const char* name)
{
	lv_obj_t* btn = ui_helpers_create_btn_with_text(prv_gauge_select_btn_container, name, LV_FONT_DEFAULT);
	//lv_obj_add_event(btn, prv_gauge_select_btn_handler, LV_EVENT_CLICKED, NULL);
}

void ui_gauges_create_gauge_checkbox(const char* name)
{
	lv_obj_t* checkbox = ui_helpers_create_checkbox_with_text(prv_gauge_select_btn_container, name, LV_FONT_DEFAULT);
	lv_obj_add_event(checkbox, prv_gauge_select_checkbox_handler, LV_EVENT_CLICKED, NULL);
}

void ui_gauges_set_number_of_gauges(uint8_t num_gauges)
{
	prv_num_gauges = num_gauges;
}

void ui_gauges_create_gauge(const char* name, const char* units, int32_t min, int32_t max, uint8_t gauge_idx)
{
	_load_gauge(min, max, name, units, gauge_idx);
}

void ui_gauges_set_gauge_select_btn_cb(void (*func)(lv_event_t* e))
{
	_gauge_select_btn_cb = func;
}

void ui_gauges_set_gauge_cb(void (*func)(lv_event_t* e))
{
	prv_gauge_pressed_cb = func;
}

void ui_gauges_set_scr_load_cb(lv_event_cb_t func)
{
	_scr_load_cb = func;
}

void ui_set_brightness_slider_event_cb(lv_event_cb_t func)
{
	prv_slider_event_cb = func;
}

void ui_set_settings_scr_load_event_cb(lv_event_cb_t func)
{
	prv_settings_scr_load_event_cb = func;
}

void ui_set_demo_mode_checkbox_event_cb(lv_event_cb_t func)
{
	prv_demo_mode_checkbox_event_cb = func;
}

void ui_set_settings_btn_event_cb(lv_event_cb_t func)
{
	settings_btn_cb = func;
}

void ui_set_settings_back_btn_event_cb(lv_event_cb_t func)
{
	prv_settings_back_btn_event_cb = func;
}
