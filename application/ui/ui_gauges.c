/**********     INCLUDES        **********/
#include "ui_gauges.h"
#include "ui_settings.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/**********		DEFINES		**********/

/**********		EXTERNAL VARIABLE DEFINITIONS		**********/

/**********		STATIC VARIABLES		**********/
static bool prv_is_init = false;
static lv_obj_t* prv_gauge_select_checkboxes[4];	//Holds pointers to the gauge select checkboxes that are checked.
static uint8_t prv_num_gauges = 0;					//Application sets this before calling load gauge.
static uint8_t prv_selected_checkboxes_count = 0;		//Tracks how many checkboxes are currently checked.
static lv_anim_t _gauge_demo_animation;				//Animation that runs in demo mode, used to test new gauges


/* UI Objects. */
static lv_obj_t* prv_main_scr;						//The parent object.
static lv_obj_t* prv_gauge_select_btn_container;	//The container that holds the checkboxes.
static lv_obj_t* prv_options_btn_container;			//The container that holds the options buttons
static lv_obj_t* prv_view_btn;						//The view button.
static lv_obj_t* prv_clear_btn;						//The clear button. 
static lv_obj_t* prv_refresh_btn;					//The refresh button.
static lv_obj_t* prv_settings_btn;					//The settings button.

/* Internal Event Handlers */
static void prv_gauge_select_checkbox_handler(lv_event_t* e);
static void prv_clear_btn_handler(lv_event_t* e);
static void prv_settings_btn_handler(lv_event_t* e);
static void prv_gauge_pressed_hanlder(lv_event_t* e);
static void prv_settings_btn_handler(lv_event_t* e);

/* External Event Handlers */
static lv_event_cb_t settings_btn_cb = NULL;
static lv_event_cb_t prv_view_gauges_event_cb = NULL;

/*All the info for the gauge currently being displayed.*/
static lv_obj_t* _gauge_scr;			//The screen being the parent to the lv_scale that is the gauge.
static lv_obj_t* _gauge[4];				//The lv_scale that is the gauge. 
static lv_obj_t* _gauge_needle[4];			//The lv_line that acts as the needle, set to -1 if not using a needle.
static lv_obj_t* _gauge_data_lbl[4];		//The label on the gauge face that displays the number on the gauge.
static lv_obj_t* _gauge_info_lbl[4];		//Label that tells the user about what data is being displayed.
static lv_obj_t* gauge_units_lbl[4];
static float _gauge_scaling_factor[4];	//How the value is multiplied by when decimals are needed.
static void (*prv_gauge_single_clicked_cb)(lv_event_t* e) = NULL;
static void (*prv_gauge_long_pressed_cb)(lv_event_t* e) = NULL;



/*Event function pointers.*/
static lv_event_cb_t _scr_load_cb = NULL;


/**********		STATIC FUNCTION DECLRATIONS		**********/
static void _scr_load_handler(lv_event_t* e);
static void _gauge_anim_map(void* obj, int32_t val);
static void prv_deactivate_all_checkboxes();
static void prv_activate_all_checkboxes();
static void prv_clear_all_checkboxes();


static void _load_gauge(int32_t min_val, int32_t max_val, const char* primary_lbl, const char* secondary_lbl, uint8_t gauge_idx);


/**********		STATIC FUNCTION DEFINITIONS		**********/
static void prv_gauge_select_checkbox_handler(lv_event_t* e)
{
	lv_event_code_t event = lv_event_get_code(e);
	lv_obj_t* checkbox = lv_event_get_target_obj(e);
	bool checked = lv_obj_has_state(checkbox, LV_STATE_CHECKED);
	static bool last_was_long_press = false;

	if (event == LV_EVENT_DOUBLE_CLICKED)
	{
		prv_clear_all_checkboxes();
		return;
	}

	if (event == LV_EVENT_LONG_PRESSED)
	{
		last_was_long_press = true;
	}

	if (event == LV_EVENT_CLICKED)
	{
		if (event == LV_EVENT_LONG_PRESSED) { checked = !checked;  }	//Have to invert the checked state if it's a long press event bc the box hasn't changed state yet.
		if (!lv_obj_has_flag(checkbox, LV_OBJ_FLAG_CHECKABLE))			//If the checkbox isnt checkable it's disabled and there's already 4 boxes checked.
		{
			return;
		}
		if (checked)
		{
			prv_gauge_select_checkboxes[prv_selected_checkboxes_count] = checkbox;
			prv_selected_checkboxes_count++;
			if (prv_selected_checkboxes_count >= 4)
			{
				//Deactivate all the checkboxes.
				prv_deactivate_all_checkboxes();
			}
		}
		else
		{
			bool removed = false;
			for (uint8_t i = 0; i < prv_selected_checkboxes_count; i++)
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
			if (prv_selected_checkboxes_count == 4)
			{
				prv_activate_all_checkboxes();
			}
			prv_gauge_select_checkboxes[prv_selected_checkboxes_count - 1] = NULL;
			prv_selected_checkboxes_count--;
		}
		if (last_was_long_press == true)
		{
			prv_view_gauges_event_cb(e);
			last_was_long_press = false;
		}
	}


}

static void prv_deactivate_all_checkboxes()
{
	//Deactivate all the checkboxes.
	uint32_t child_idx = 0;
	lv_obj_t* child_checkbox = lv_obj_get_child(prv_gauge_select_btn_container, child_idx);
	while (child_checkbox != NULL)
	{
		/* If it's not checked, disable it.*/
		if (!lv_obj_has_state(child_checkbox, LV_STATE_CHECKED))
		{
			lv_obj_remove_flag(child_checkbox, LV_OBJ_FLAG_CHECKABLE);
		}
		child_idx++;
		child_checkbox = lv_obj_get_child(prv_gauge_select_btn_container, child_idx);
	}
}

static void prv_activate_all_checkboxes()
{
	uint32_t child_idx = 0;
	lv_obj_t* child_checkbox = lv_obj_get_child(prv_gauge_select_btn_container, child_idx);
	while (child_checkbox != NULL)
	{
		/* If it's not checked, disable it.*/
		lv_obj_add_flag(child_checkbox, LV_OBJ_FLAG_CHECKABLE);
		child_idx++;
		child_checkbox = lv_obj_get_child(prv_gauge_select_btn_container, child_idx);
	}
}

static void prv_clear_btn_handler(lv_event_t* e)
{
	prv_clear_all_checkboxes();
}

static void prv_clear_all_checkboxes()
{
	uint8_t counter = 0;
	while (prv_gauge_select_checkboxes[counter] != NULL)
	{
		lv_obj_t* checkbox = prv_gauge_select_checkboxes[counter];
		lv_obj_remove_state(checkbox, LV_STATE_CHECKED);
		prv_gauge_select_checkboxes[counter] = NULL;
		counter++;
		if (counter == 4) { break; }
	}
	prv_activate_all_checkboxes();
	prv_selected_checkboxes_count = 0;
}

static void prv_gauge_pressed_hanlder(lv_event_t* e)
{
	lv_event_code_t event_code = lv_event_get_code(e);
	if (event_code == LV_EVENT_SINGLE_CLICKED)
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
		if (prv_gauge_single_clicked_cb != NULL)
		{
			prv_gauge_single_clicked_cb(e);
		}
	}

	if (event_code == LV_EVENT_LONG_PRESSED)
	{
		prv_gauge_long_pressed_cb(e);
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
	lv_obj_set_scrollbar_mode(_gauge_scr, LV_SCROLLBAR_MODE_OFF);
	lv_obj_remove_flag(_gauge_scr, LV_OBJ_FLAG_SCROLLABLE);

	/**
	 * Determine the scaling factor and if we need a second, hidden gauge.
	 * If gauges have less than 100 tick marks the needle motion isn't smooth going from tick to tick.
	 * If a gauge has more than 500 tick marks performance starts to degrade bc LVGL tries to render
	 * all 500 tick marks even if the minor ticks arent visible.
	 */

	int32_t number_of_ticks = abs(max_val - min_val);

	/* Make sure min and max are factors of 10 if there's more than 25 total ticks. */
	if (number_of_ticks >= 25)
	{
		if (max_val > min_val)
		{
			max_val -= (max_val % 10);
			min_val -= (min_val % 10);
		}
		else
		{
			min_val -= (min_val % 10);
			max_val -= (max_val % 10);
		}
		number_of_ticks = abs(max_val - min_val);
	}

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
	switch (prv_num_gauges) {
		/* If there's only one gauge we dont need to check the index.*/
		case 1:
			span = 270;
			start = 135;
			break;
		case 2:
			span = 154;
			start = (gauge_idx * 180) + 103;
			break;

		case 3: //Break into thirds.
			span = 100;			//120 degrees per gauge.
			if (gauge_idx == 0) {  start = 210; }
			else if (gauge_idx == 1) { start = -30; }
			else if (gauge_idx == 2) { start = 90; }
			start += 10;
			break;
		case 4:
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

	/* Make the label that shows the data. */
	_gauge_data_lbl[gauge_idx] = lv_label_create(_gauge_scr);
	int32_t data_lbl_pos_y = 0;
	int32_t data_lbl_pos_x = 0;
	lv_font_t* data_font = &lv_font_montserrat_42;
	if (prv_num_gauges == 2)
	{
		data_lbl_pos_y = 0;
		data_lbl_pos_x = 75;
		data_font = &lv_font_montserrat_42;
		if (gauge_idx == 0) { data_lbl_pos_x *= -1; }
	}
	else if (prv_num_gauges == 3)
	{
		/* This is for 1 & 2. X is just the negative of the other. */
		data_lbl_pos_x = 80;
		data_lbl_pos_y = 20;
		switch (gauge_idx)
		{
		case 0:
			data_lbl_pos_x = 0;
			data_lbl_pos_y = -65;
			break;
		case 2:
			data_lbl_pos_x *= -1;
			break;
		}
	}
	else if (prv_num_gauges == 4)
	{
		data_lbl_pos_x = -60;
		data_lbl_pos_y = -65;
		data_font = &lv_font_montserrat_34;
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
	lv_obj_set_style_text_font(_gauge_data_lbl[gauge_idx], data_font, LV_PART_MAIN);
	lv_obj_set_style_text_color(_gauge_data_lbl[gauge_idx], UI_COLOR_WHITE, LV_PART_MAIN);
	lv_obj_set_style_text_align(_gauge_data_lbl[gauge_idx], LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
	
	/* Make the label that says what is being displayed. */
	_gauge_info_lbl[gauge_idx] = lv_arclabel_create(_gauge_scr);
	int32_t desc_lbl_pos_y = 216;
	int32_t desc_lbl_pos_x = 0;
	int32_t desc_lbl_width = 340;
	int32_t desc_lbl_height = 340;
	int32_t desc_lbl_angle_start = 270;
	int32_t desc_lbl_span = 180;
	lv_arclabel_dir_t desc_lbl_direction = LV_ARCLABEL_DIR_COUNTER_CLOCKWISE;
	int8_t desc_lbl_letter_spacing = 4;
	const int32_t desc_lbl_radius = 240;
	if (prv_num_gauges == 2)
	{
		desc_lbl_pos_y = 0;
		desc_lbl_width = 340;
		desc_lbl_pos_x = -220 + (220 * 2 * gauge_idx);	//-216 if first gauge, 216 if second gauge.
		desc_lbl_angle_start = 180 * gauge_idx;			//0 if first gauge, 180 if second gauge.
		desc_lbl_height = 340;
	}
	else if (prv_num_gauges == 3)
	{
		desc_lbl_angle_start = (120 * gauge_idx) + 90;
		desc_lbl_direction = LV_ARCLABEL_DIR_CLOCKWISE;
		desc_lbl_pos_x = 0;
		desc_lbl_pos_y = -220;
		switch (gauge_idx)
		{
		case 0:
			break;
		case 1:
			desc_lbl_pos_x = 190;
			desc_lbl_pos_y = 110;
			desc_lbl_height = 400;
			desc_lbl_width = 400;
			desc_lbl_direction = LV_ARCLABEL_DIR_COUNTER_CLOCKWISE;
			break;
		case 2:
			desc_lbl_pos_x = -190;
			desc_lbl_pos_y = 110;
			desc_lbl_height = 400;
			desc_lbl_width = 400;
			desc_lbl_direction = LV_ARCLABEL_DIR_COUNTER_CLOCKWISE;
			break;
		}
	}
	else if (prv_num_gauges == 4)
	{
		desc_lbl_letter_spacing = 0;
		desc_lbl_width = 300;
		desc_lbl_height = 300;
		desc_lbl_pos_x = -155;
		desc_lbl_pos_y = -155;
		switch (gauge_idx)
		{
		case 0:
			desc_lbl_angle_start = 45;
			desc_lbl_direction = LV_ARCLABEL_DIR_CLOCKWISE;
			break;
		case 1:
			desc_lbl_pos_x *= -1;
			desc_lbl_angle_start = 135;
			desc_lbl_direction = LV_ARCLABEL_DIR_CLOCKWISE;
			break;
		case 2:
			desc_lbl_pos_y *= -1;
			desc_lbl_angle_start = -45;
			break;
		case 3:
			desc_lbl_pos_x *= -1;
			desc_lbl_pos_y *= -1;
			desc_lbl_angle_start = -135;
			break;
		}
	}

	lv_obj_set_size(_gauge_info_lbl[gauge_idx], desc_lbl_width, desc_lbl_height);
	lv_obj_align(_gauge_info_lbl[gauge_idx], LV_ALIGN_CENTER, desc_lbl_pos_x, desc_lbl_pos_y);
	lv_obj_set_style_text_letter_space(_gauge_info_lbl[gauge_idx], desc_lbl_letter_spacing, LV_PART_MAIN);
	lv_obj_set_style_text_color(_gauge_info_lbl[gauge_idx], UI_COLOR_WHITE, LV_PART_MAIN);
	lv_arclabel_set_text(_gauge_info_lbl[gauge_idx], primary_lbl);
	float offset_y = sin((float)((float)desc_lbl_angle_start * 3.14 / 180.0));
	float offset_x = cos((float)((float)desc_lbl_angle_start * 3.14 / 180.0));
	lv_arclabel_set_center_offset_y(_gauge_info_lbl[gauge_idx], desc_lbl_radius * offset_y);
	lv_arclabel_set_center_offset_x(_gauge_info_lbl[gauge_idx], desc_lbl_radius * offset_x);
	lv_arclabel_set_angle_start(_gauge_info_lbl[gauge_idx], desc_lbl_angle_start);
	lv_arclabel_set_radius(_gauge_info_lbl[gauge_idx], desc_lbl_radius);
	lv_arclabel_set_text_vertical_align(_gauge_info_lbl[gauge_idx], LV_ARCLABEL_TEXT_ALIGN_CENTER);
	lv_arclabel_set_text_horizontal_align(_gauge_info_lbl[gauge_idx], LV_ARCLABEL_TEXT_ALIGN_CENTER);
	lv_arclabel_set_dir(_gauge_info_lbl[gauge_idx], desc_lbl_direction);
	lv_obj_set_style_text_font(_gauge_info_lbl[gauge_idx], &lv_font_montserrat_24, LV_PART_MAIN);
	lv_obj_move_foreground(_gauge_info_lbl[gauge_idx]);
	/* For debugging. */
	//lv_obj_set_style_border_color(_gauge_info_lbl[gauge_idx], UI_COLOR_WHITE, LV_PART_MAIN);
	//lv_obj_set_style_border_width(_gauge_info_lbl[gauge_idx], 2, LV_PART_MAIN);
	//lv_obj_set_style_border_opa(_gauge_info_lbl[gauge_idx], 255, LV_PART_MAIN);

	/* Make the label that displays the units. */
	gauge_units_lbl[gauge_idx] = lv_label_create(_gauge_scr);
	int32_t units_lbl_pos_y = data_lbl_pos_y + 30;
	int32_t units_lbl_pos_x = data_lbl_pos_x;

	lv_label_set_text(gauge_units_lbl[gauge_idx], secondary_lbl);
	lv_obj_set_style_text_font(gauge_units_lbl[gauge_idx], &lv_font_montserrat_20, LV_PART_MAIN);
	lv_obj_set_style_text_color(gauge_units_lbl[gauge_idx], UI_COLOR_WHITE, LV_PART_MAIN);
	lv_obj_set_style_text_align(gauge_units_lbl[gauge_idx], LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
	lv_obj_align(gauge_units_lbl[gauge_idx], LV_ALIGN_CENTER, units_lbl_pos_x, units_lbl_pos_y);

	/* Do this if were on the last gauge to be loaded in. */
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

		/* Draw a line or 2 to divide the gauges. */
		if (prv_num_gauges == 2 || prv_num_gauges == 4)
		{

			lv_obj_t* divider_line = lv_obj_create(_gauge_scr);
			lv_obj_set_size(divider_line, 7, 400);
			lv_obj_set_style_bg_color(divider_line, UI_COLOR_GRAY, LV_PART_MAIN);
			lv_obj_set_style_border_color(divider_line, UI_COLOR_GRAY, LV_PART_MAIN);
			lv_obj_center(divider_line);
		}
		if (prv_num_gauges == 4)
		{

			lv_obj_t* divider_line = lv_obj_create(_gauge_scr);
			lv_obj_set_size(divider_line, 400, 7);
			lv_obj_set_style_bg_color(divider_line, UI_COLOR_GRAY, LV_PART_MAIN);
			lv_obj_set_style_border_color(divider_line, UI_COLOR_GRAY, LV_PART_MAIN);
			lv_obj_center(divider_line);
		}
		if (prv_num_gauges == 3)
		{
			/*Create an array for the points of the line*/
			static lv_point_precise_t line_points1[] = { {0, 200}, {0, 0} };
			static lv_point_precise_t line_points2[] = { {0, 0}, {173, 100}, {346, 0} };

			/*Create style*/
			static lv_style_t style_line;
			lv_style_init(&style_line);
			lv_style_set_line_width(&style_line, 7);
			lv_style_set_line_color(&style_line, UI_COLOR_GRAY);
			lv_style_set_line_rounded(&style_line, true);

			/*Create a line and apply the new style*/
			lv_obj_t* line1; 
			lv_obj_t* line2;
			line1 = lv_line_create(_gauge_scr);
			line2 = lv_line_create(_gauge_scr);
			//lv_obj_set_size(line1, 400, 400);
			//lv_obj_set_size(line2, 400, 400);
			lv_line_set_points(line1, line_points1, 2);     /*Set the points*/
			lv_line_set_points(line2, line_points2, 3);     /*Set the points*/
			lv_obj_add_style(line1, &style_line, 0);
			lv_obj_add_style(line2, &style_line, 0);
			lv_obj_align(line1, LV_ALIGN_CENTER, 0, 100);
			lv_obj_align(line2, LV_ALIGN_CENTER, 0, -50);
			lv_obj_move_foreground(line1);
			lv_obj_move_foreground(line2);
		}
	}

	/* Apply the event callback to all the children of _gauge_scr so
	when anything on the screen is pressed it will fire the event and we can
	close the gauge screen. */
	lv_obj_add_event_cb(_gauge_scr, prv_gauge_pressed_hanlder, LV_EVENT_SINGLE_CLICKED, NULL);
	lv_obj_add_event_cb(_gauge_scr, prv_gauge_pressed_hanlder, LV_EVENT_LONG_PRESSED, NULL);
	for (uint32_t child = 0;; child++)
	{
		lv_obj_t* obj = lv_obj_get_child(_gauge_scr, child);
		if (obj == NULL)
		{
			break;
		}
		lv_obj_add_event_cb(obj, prv_gauge_pressed_hanlder, LV_EVENT_SINGLE_CLICKED, NULL);
		lv_obj_add_event_cb(obj, prv_gauge_pressed_hanlder, LV_EVENT_LONG_PRESSED, NULL);
	}
	

	if (ui_helpers_is_demo_mode())
	{
		ui_helpers_create_gauge_animation(&_gauge_demo_animation, _gauge[gauge_idx], &_gauge_anim_map, 2500, min_val, max_val + 1);
	}
}


static void prv_settings_btn_handler(lv_event_t* e)
{
	lv_event_code_t event_code = lv_event_get_code(e);
	if (event_code == LV_EVENT_RELEASED)
	{
		ui_settings_load();
	}

	/*Check if there's a function CB assign and call it if there is.*/
	if (settings_btn_cb != NULL)
	{
		settings_btn_cb(e);
	}
}

/**********		GLOBAL FUNCTION DEFINITIONS		**********/
void ui_gauges_load()
{
	if (prv_is_init == false)
	{
		ui_gauges_init();
	}
	lv_scr_load(prv_main_scr);
	lv_obj_remove_event(prv_main_scr, 0);
}

void ui_gauges_init()
{
	if (prv_is_init)
	{
		return;	//Already created all the object, dont need to again.
	}
	/*MAIN SCREEN.*/
	prv_main_scr = lv_obj_create(NULL);
	lv_obj_set_style_bg_color(prv_main_scr, UI_COLOR_BLACK, LV_STATE_DEFAULT);
	lv_obj_set_layout(prv_main_scr, LV_LAYOUT_FLEX);
	lv_obj_set_flex_flow(prv_main_scr, LV_FLEX_FLOW_ROW);
	lv_obj_set_flex_align(prv_main_scr, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_SPACE_EVENLY);
	lv_obj_set_style_pad_top(prv_main_scr, 70, LV_STATE_DEFAULT);
	lv_obj_set_style_pad_bottom(prv_main_scr, 70, LV_STATE_DEFAULT);
	lv_obj_set_scroll_snap_x(prv_main_scr, LV_SCROLL_SNAP_CENTER);
	lv_obj_set_scroll_dir(prv_main_scr, LV_DIR_HOR);
	lv_obj_set_scrollbar_mode(prv_main_scr, LV_SCROLLBAR_MODE_OFF);

	/*CHECKBOXES CONTAINER. (page 1)*/
	prv_gauge_select_btn_container = lv_obj_create(prv_main_scr);
	lv_obj_set_size(prv_gauge_select_btn_container, 460, 480);
	lv_obj_set_style_pad_top(prv_gauge_select_btn_container, 65, LV_STATE_DEFAULT);
	lv_obj_set_style_pad_bottom(prv_gauge_select_btn_container, 100, LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(prv_gauge_select_btn_container, 80, LV_STATE_DEFAULT);
	lv_obj_set_style_pad_right(prv_gauge_select_btn_container, 80, LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(prv_gauge_select_btn_container, UI_COLOR_BLACK, LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(prv_gauge_select_btn_container, 0, LV_STATE_DEFAULT);
	lv_obj_set_layout(prv_gauge_select_btn_container, LV_LAYOUT_FLEX);
	lv_obj_set_flex_flow(prv_gauge_select_btn_container, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_flex_align(prv_gauge_select_btn_container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_SPACE_EVENLY);
	lv_obj_set_scroll_dir(prv_gauge_select_btn_container, LV_DIR_VER);
	lv_obj_set_scrollbar_mode(prv_gauge_select_btn_container, LV_SCROLLBAR_MODE_OFF);

	/*OPTIONS BUTTON CONTAINER. (page 2)*/
	prv_options_btn_container = lv_obj_create(prv_main_scr);
	lv_obj_set_size(prv_options_btn_container, 400, 480);
	lv_obj_set_style_bg_color(prv_options_btn_container, UI_COLOR_BLACK, LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(prv_options_btn_container, 0, LV_STATE_DEFAULT);
	lv_obj_set_layout(prv_options_btn_container, LV_LAYOUT_FLEX);
	lv_obj_set_flex_flow(prv_options_btn_container, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_flex_align(prv_options_btn_container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_SPACE_EVENLY);
	lv_obj_set_scroll_dir(prv_options_btn_container, LV_DIR_VER);

	/* HEADER LOGO. */
	lv_obj_t* logo_container = lv_obj_create(prv_gauge_select_btn_container);
	lv_obj_clear_flag(logo_container, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_set_scrollbar_mode(logo_container, LV_SCROLLBAR_MODE_OFF);
	lv_obj_set_size(logo_container, lv_pct(100), cangauge_logo_header.header.h);
	lv_obj_set_style_bg_opa(logo_container, 0, LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(logo_container, 0, LV_STATE_DEFAULT);
	lv_obj_t* logo = lv_image_create(logo_container);
	lv_image_set_src(logo, &cangauge_logo_header);
	lv_obj_center(logo);

	/* REFRESH BUTTON. */
	prv_refresh_btn = ui_helpers_create_btn_with_text(prv_gauge_select_btn_container, "Refresh", LV_FONT_DEFAULT);
	lv_obj_set_width(prv_refresh_btn, lv_pct(100));

	/* GAUGE SCREEN. */
	_gauge_scr = lv_obj_create(NULL);
	lv_obj_set_style_bg_color(_gauge_scr, UI_COLOR_BLACK, LV_PART_MAIN);

	/*Bind the controls and event function handlers.*/
	lv_obj_add_event(prv_main_scr, _scr_load_handler, LV_EVENT_SCREEN_LOAD_START, NULL);

	prv_is_init = true;



}

void ui_gauges_delete()
{
	prv_clear_all_checkboxes();
	lv_obj_delete_async(prv_main_scr);
	prv_is_init = false;

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
	lv_obj_add_event(checkbox, prv_gauge_select_checkbox_handler, LV_EVENT_CLICKED, prv_gauge_select_checkboxes);
	lv_obj_add_event(checkbox, prv_gauge_select_checkbox_handler, LV_EVENT_DOUBLE_CLICKED, NULL);
	lv_obj_add_event(checkbox, prv_gauge_select_checkbox_handler, LV_EVENT_LONG_PRESSED, prv_gauge_select_checkboxes);
	lv_obj_t* refresh_btn = lv_obj_get_child_by_type(prv_gauge_select_btn_container, 0, &lv_button_class);
	lv_obj_move_foreground(refresh_btn);
}

void ui_gauges_set_number_of_gauges(uint8_t num_gauges)
{
	prv_num_gauges = num_gauges;
}

void ui_gauges_create_gauge(const char* name, const char* units, int32_t min, int32_t max, uint8_t gauge_idx)
{
	_load_gauge(min, max, name, units, gauge_idx);
}

void ui_gauges_set_gauge_single_clicked_cb(void (*func)(lv_event_t* e))
{
	prv_gauge_single_clicked_cb = func;
}

void ui_gauges_set_gauge_long_pressed_cb(void (*func)(lv_event_t* e))
{
	prv_gauge_long_pressed_cb = func;
}

void ui_gauges_set_scr_load_cb(lv_event_cb_t func)
{
	_scr_load_cb = func;
}

void ui_load_gauge_screen()
{
	lv_screen_load(_gauge_scr);
}

void ui_delete_gauge_select_checkboxes()
{
	lv_obj_clean(prv_gauge_select_btn_container);
}

lv_obj_t* ui_gauges_get_options_container_obj()
{
	return prv_options_btn_container;
}

void ui_set_settings_btn_event_cb(lv_event_cb_t func)
{
	settings_btn_cb = func;
}

void ui_gauges_set_view_btn_cb(lv_event_cb_t func) 
{ 
	prv_view_gauges_event_cb = func;
}

void ui_add_clear_btn_event_cb(lv_event_cb_t func) 
{ 
	lv_obj_add_event_cb(prv_clear_btn, func, LV_EVENT_RELEASED, NULL); 
}


void ui_add_refresh_btn_event_cb(lv_event_cb_t func) 
{ 
	lv_obj_add_event_cb(prv_refresh_btn, func, LV_EVENT_RELEASED, NULL);
}
