/**********     INCLUDES        **********/
#include "lvgl.h"
#include "src/widgets/chart/lv_chart_private.h"
#include "ui_gauges_prv.h"
#include "ui_graph.h"
#include "ui_helpers/ui_helpers.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
/**********     TYPEDEFS         **********/

/**********		DEFINES		**********/

/**********		EXTERNAL VARIABLE DEFINITIONS		**********/

/**********		STATIC VARIABLES		**********/
static lv_obj_t* prv_main_container;
static lv_obj_t* prv_chart;
static lv_obj_t* prv_files_list;
static lv_obj_t* prv_delete_btn;
static lv_obj_t* prv_series_info_container;
static const char* prv_selected_file_name;		//The file name of the selected list item.
static int32_t prv_pri_axis_max = 0x80000000;	//Signed int, hold the the max value of the primary y axis.
static int32_t prv_pri_axis_min = 0x7FFFFFFF;	//Signed int, hold the the min value of the primary y axis.
static int32_t prv_sec_axis_max = 0x80000000;	//Signed int, hold the the max value of the secondary y axis.
static int32_t prv_sec_axis_min = 0x7FFFFFFF;	//Signed int, hold the the min value of the secondary y axis.
static uint32_t prv_max_time_ms = 0;			//Dont edit this outside of ui_graph_set_timebase. This is used internally to determine how many pts are in the data arrays for x axis scaling.
static uint32_t prv_timebase_ms = 0;			//Dont edit this outside of ui_graph_set_timebase. This is used internally to determine how many pts are in the data arrays for x axis scaling.
static uint32_t prv_x_axis_point_offset = 0;	//Keeps track of how far offset the data arrays are for adjustment of the x-axis since the LVGL implementation of change starting point is dumb asf.
static float prv_max_time_s = 0;				//This is made to match what is  being displayed vs the above.
static float prv_min_time_s = 0;				//This is made to match what is being displayed.
/**********		STATIC FUNCTION DECLRATIONS		**********/
static void prv_list_btn_pressed_handler(lv_event_t* e);
static void prv_series_switch_event_handler(lv_event_t* e);			//User data contained in the event is the lv_series_t.

/**
* @brief The callback for when one of the graph's axis labels are pressed. The 
* labels are actually text areas styled to look like a label. It opens up the 
* number pad and assigns the keypad to that text area. It also assigns the event
* handler for the numberpad being pressed.
* 
* @param e The event. It's used to determine which axis label was pressed.
* @attention The event's user data is a pointer to the int32_t that holds the value
* for that axes min/max. The value in the text area should be assigned to this integer.
*/
static void prv_axis_lbl_pressed_cb(lv_event_t* e);

/**
* @brief This is the event handler for the number pad being pressed when one of the 
* axis text areas are being edited. The only time
* it does anything is when the "ok" button is pressed. When that happens it removes
* this function as an event handler for the numberpad and closes the numberpad.
* 
* @param e The LVGL event. This is used to determine which key on the numberpad was
* pressed that triggered the event.
* @attention The event's user data is a pointer to the int32_t that holds the value
* for that axes min/max. The value in the text area should be assigned to this integer.
*/
static void prv_numberpad_pressed_y_axis_cb(lv_event_t* e);

/**
* @brief Updates the y-axis scale of the chart with the values stored in prv_xxx_y_axis_yyy variables.
* Also checks the values of these variables to make sure they're being used (i.e. not set to the 
* default values) and that the mins are greater the than maxes.
*/
static void prv_update_chart_axes();
static float prv_get_array_max_val_f(float* arr, uint32_t arr_len);
static float prv_get_array_min_val_f(float* arr, uint32_t arr_len);
static int32_t prv_get_array_max_val_i32(int32_t* arr, uint32_t arr_len);
static int32_t prv_get_array_min_val_i32(int32_t* arr, uint32_t arr_len);

/**
* @brief Determines the values of each y-axis label depending on what the min and max of the 
* series that have been assigned to them are. The max y-axis label will be equal to the max
* value of all the series that are assigned to that axis and the min label the same but the
* smallest value of the series assigned to that axis.
*/
static void prv_update_chart();
static void prv_set_series_y_axis(lv_chart_series_t* ser, lv_chart_axis_t y_axis);
static lv_chart_axis_t prv_get_series_y_axis(lv_chart_series_t* ser);

/**********		STATIC FUNCTION DEFINITIONS		**********/
static void prv_list_btn_pressed_handler(lv_event_t* e)
{
	lv_obj_t* btn = lv_event_get_target(e);
	lv_obj_t* lbl = lv_obj_get_child(btn, 0);
	lv_obj_set_state(btn, LV_STATE_USER_1, true);
	lv_obj_t* child_btn = lv_obj_get_child(prv_files_list, 0);
	uint32_t counter = 1;
	while (child_btn != NULL)
	{
		if (child_btn != btn)
		{
			lv_obj_remove_state(child_btn, LV_STATE_USER_1);
		}
		child_btn = lv_obj_get_child(prv_files_list, counter);
		counter++;
	}
	const char* txt = lv_label_get_text(lbl);
	prv_selected_file_name = txt;
}

static void prv_series_switch_event_handler(lv_event_t* e)
{
	lv_obj_t* sw = lv_event_get_target_obj(e);
	bool sw_state = lv_obj_has_state(sw, LV_STATE_CHECKED);
	lv_chart_series_t* ser = lv_event_get_user_data(e);

	if (sw_state == true)
	{
		prv_set_series_y_axis(ser, LV_CHART_AXIS_SECONDARY_Y);
	}
	else
	{
		prv_set_series_y_axis(ser, LV_CHART_AXIS_PRIMARY_Y);
	}
	prv_update_chart();
}

static void prv_axis_lbl_pressed_cb(lv_event_t* e)
{
	lv_obj_t* txt_area = lv_event_get_target_obj(e);	//Get the text area that was clicked.
	lv_obj_t* numpad = ui_helpers_load_number_pad();	//Show the number pad.
	void* user_data = lv_event_get_user_data(e);		//Pointer to the int32_t that holds the axis value.
	lv_keyboard_set_textarea(numpad, txt_area);			//Assign the number pad.
	lv_obj_add_event_cb(numpad, prv_numberpad_pressed_y_axis_cb, LV_EVENT_VALUE_CHANGED, user_data);		//Bind the event and pass along the int pointer.
}

static void prv_numberpad_pressed_y_axis_cb(lv_event_t* e)
{
	lv_obj_t* numpad = lv_event_get_target_obj(e);
	uint32_t key = lv_keyboard_get_selected_btn(numpad);
	const char* txt = lv_keyboard_get_btn_text(numpad, key);
	
	/* If any key other than the OK symbol were pressed return. */
	if (lv_streq(txt, LV_SYMBOL_OK) != true)
	{
		return;
	}

	void* e_user_data = lv_event_get_user_data(e);		//The user data tied to the event.

	//TODO: Optimize this if then block below.
	/* Check if the user data is pointing to one of the time labels (x axis). */
	if ((e_user_data == &prv_max_time_s) || (e_user_data == &prv_min_time_s))
	{	
		float* axis_val_f = (float*)e_user_data;
		const char* textarea_text = lv_textarea_get_text(lv_keyboard_get_textarea(numpad));		//Get the text from the text area.
		char* end_ptr;		//Used to convert string to float.
		float new_axis_val_f = strtof(textarea_text, &end_ptr);
		if (end_ptr != textarea_text)		//If end_ptr still points to the start of the textarea_text there was no text.
		{
			*axis_val_f = (new_axis_val_f);
		}
	}
	else
	{
		int32_t* axis_val_int = (int32_t*)e_user_data;		//Pointer to the axis value int32_t.
		const char* textarea_text = lv_textarea_get_text(lv_keyboard_get_textarea(numpad));		//Get the text from the text area.
		char* end_ptr;		//Used to convert string to float.
		float new_axis_val_f = strtof(textarea_text, &end_ptr);
		if (end_ptr != textarea_text)		//If end_ptr still points to the start of the textarea_text there was no text.
		{
			*axis_val_int = (int32_t)(new_axis_val_f * UI_GRAPH_Y_AXIS_MULTIPLIER);
		}
	}
	prv_update_chart_axes();
	lv_obj_remove_event_cb(numpad, prv_numberpad_pressed_y_axis_cb);
	ui_helpers_delete_number_pad();
}

static void prv_update_chart_axes()
{
	/* Make sure the axes minimum values are lower than the maximum values. If they're not just ignore it.*/
	if (prv_pri_axis_max >= prv_pri_axis_min)
	{
		lv_chart_set_axis_range(prv_chart, LV_CHART_AXIS_PRIMARY_Y, prv_pri_axis_min, prv_pri_axis_max);
	}
	if (prv_sec_axis_max >= prv_pri_axis_min)
	{
		lv_chart_set_axis_range(prv_chart, LV_CHART_AXIS_SECONDARY_Y, prv_sec_axis_min, prv_sec_axis_max);
	}
	if (prv_min_time_s < prv_max_time_s)
	{
		uint32_t number_of_pts_in_array = prv_max_time_ms / prv_timebase_ms;
		uint32_t starting_point = ((uint32_t)(prv_min_time_s * 1000)) / prv_timebase_ms;
		uint32_t ending_point = ((uint32_t)(prv_max_time_s * 1000)) / prv_timebase_ms;
		
		lv_chart_series_t* series = lv_chart_get_series_next(prv_chart, NULL);
		lv_chart_series_t* last = NULL;
		while (series != NULL)
		{
			int32_t* int_arr = lv_chart_get_series_y_array(prv_chart, series);		//Get the pointer to where ever in the data array were currently pointing to.
			int_arr -= prv_x_axis_point_offset;										//This brings us to the allocated pointer.
			lv_chart_set_series_ext_y_array(prv_chart, series, int_arr + starting_point);
			last = series;
			series = lv_chart_get_series_next(prv_chart, last);
		}
		prv_x_axis_point_offset = starting_point;
		if (ending_point <= number_of_pts_in_array)
		{
			uint32_t number_of_pts_to_display = ending_point - starting_point;
			lv_chart_set_point_count(prv_chart, number_of_pts_to_display);
		}
	}
	lv_chart_refresh(prv_chart);
}

static float prv_get_array_max_val_f(float* arr, uint32_t arr_len)
{
	float max = arr[0];
	for (uint32_t i = 0; i < arr_len; i++)
	{
		if (arr[i] > max)
		{
			max = arr[i];
		}
	}
	return max;
}

static float prv_get_array_min_val_f(float* arr, uint32_t arr_len)
{
	float min = arr[0];
	for (uint32_t i = 0; i < arr_len; i++)
	{
		if (arr[i] < min)
		{
			min = arr[i];
		}
	}
	return min;
}

static int32_t prv_get_array_max_val_i32(int32_t* arr, uint32_t arr_len)
{
	int32_t max = arr[0];
	for (uint32_t i = 0; i < arr_len; i++)
	{
		if (arr[i] > max)
		{
			max = arr[i];
		}
	}
	return max;
}

static int32_t prv_get_array_min_val_i32(int32_t* arr, uint32_t arr_len)
{
	float min = arr[0];
	for (uint32_t i = 0; i < arr_len; i++)
	{
		if (arr[i] < min)
		{
			min = arr[i];
		}
	}
	return min;
}

static void prv_update_chart()
{
	/* Get rid of the old labels. */
	lv_obj_clean(prv_chart);

	/* Find the min and max for both y-axes. */
	prv_pri_axis_max = 0x80000000;
	prv_pri_axis_min = 0x7FFFFFFF;
	prv_sec_axis_max = 0x80000000;
	prv_sec_axis_min = 0x7FFFFFFF;
	lv_chart_series_t* series = lv_chart_get_series_next(prv_chart, NULL);
	while (series != NULL)
	{
		lv_chart_axis_t axis = prv_get_series_y_axis(series);
		int32_t* data_arr = lv_chart_get_series_y_array(prv_chart, series);
		uint32_t num_pts = lv_chart_get_point_count(prv_chart);
		int32_t max = prv_get_array_max_val_i32(data_arr, num_pts);
		int32_t min = prv_get_array_min_val_i32(data_arr, num_pts);
		if (axis == LV_CHART_AXIS_PRIMARY_Y)
		{
			prv_pri_axis_max = (max > prv_pri_axis_max) ? max : prv_pri_axis_max;
			prv_pri_axis_min = (min < prv_pri_axis_min) ? min : prv_pri_axis_min;
		}
		else
		{
			prv_sec_axis_max = (max > prv_sec_axis_max) ? max : prv_sec_axis_max;
			prv_sec_axis_min = (min < prv_sec_axis_min) ? min : prv_sec_axis_min;
		}
		series = lv_chart_get_series_next(prv_chart, series);
	}
	lv_chart_set_axis_max_value(prv_chart, LV_CHART_AXIS_PRIMARY_Y, prv_pri_axis_max);
	lv_chart_set_axis_min_value(prv_chart, LV_CHART_AXIS_PRIMARY_Y, prv_pri_axis_min);
	lv_chart_set_axis_max_value(prv_chart, LV_CHART_AXIS_SECONDARY_Y, prv_sec_axis_max);
	lv_chart_set_axis_min_value(prv_chart, LV_CHART_AXIS_SECONDARY_Y, prv_sec_axis_min);

	/* 6 labels, primary max, primary min, secondary max, secondary min, time min, time max. */
	static lv_style_t textarea_style;
	lv_style_init(&textarea_style);
	lv_style_set_text_font(&textarea_style, &lv_font_montserrat_12);
	lv_style_set_text_color(&textarea_style, UI_COLOR_WHITE);
	lv_style_set_bg_opa(&textarea_style, 0);
	lv_style_set_border_opa(&textarea_style, 0);

	lv_obj_t* pri_max_lbl = lv_textarea_create(prv_chart);
	lv_obj_set_width(pri_max_lbl, 100);
	lv_textarea_set_one_line(pri_max_lbl, true);
	lv_obj_align(pri_max_lbl, LV_ALIGN_TOP_LEFT, -10, -35);
	float pri_max_float = (float)prv_pri_axis_max / UI_GRAPH_Y_AXIS_MULTIPLIER;
	lv_label_set_text_fmt(lv_textarea_get_label(pri_max_lbl), "%.1f", pri_max_float);
	lv_obj_set_style_text_align(lv_textarea_get_label(pri_max_lbl), LV_TEXT_ALIGN_LEFT, 0);
	lv_obj_set_style_border_color(pri_max_lbl, UI_COLOR_WHITE, LV_PART_CURSOR | LV_STATE_FOCUSED);
	lv_obj_add_style(pri_max_lbl, &textarea_style, 0);
	lv_obj_add_event_cb(pri_max_lbl, prv_axis_lbl_pressed_cb, LV_EVENT_SINGLE_CLICKED, &prv_pri_axis_max);
	if (prv_pri_axis_max == 0x80000000)
	{
		lv_obj_set_flag(pri_max_lbl, LV_OBJ_FLAG_HIDDEN, true);
	}

	lv_obj_t* pri_min_lbl = lv_textarea_create(prv_chart);
	lv_obj_set_width(pri_min_lbl, 100);
	lv_textarea_set_one_line(pri_min_lbl, true);
	lv_obj_align(pri_min_lbl, LV_ALIGN_BOTTOM_LEFT, -10, 35);
	float pri_min_float = (float)prv_pri_axis_min / UI_GRAPH_Y_AXIS_MULTIPLIER;
	lv_label_set_text_fmt(lv_textarea_get_label(pri_min_lbl), "%.1f", pri_min_float);
	lv_obj_set_style_text_align(lv_textarea_get_label(pri_min_lbl), LV_TEXT_ALIGN_LEFT, 0);
	lv_obj_set_style_border_color(pri_min_lbl, UI_COLOR_WHITE, LV_PART_CURSOR | LV_STATE_FOCUSED);
	lv_obj_add_style(pri_min_lbl, &textarea_style, 0);
	lv_obj_add_event_cb(pri_min_lbl, prv_axis_lbl_pressed_cb, LV_EVENT_SINGLE_CLICKED, &prv_pri_axis_min);
	if (prv_pri_axis_min == 0x7FFFFFFF)
	{
		lv_obj_set_flag(pri_min_lbl, LV_OBJ_FLAG_HIDDEN, true);
	}

	lv_obj_t* sec_max_lbl = lv_textarea_create(prv_chart);
	lv_obj_set_width(sec_max_lbl, 100);
	lv_textarea_set_one_line(sec_max_lbl, true);
	lv_obj_align(sec_max_lbl, LV_ALIGN_TOP_RIGHT, 10, -35);
	float sec_max_float = (float)prv_sec_axis_max / UI_GRAPH_Y_AXIS_MULTIPLIER;
	lv_label_set_text_fmt(lv_textarea_get_label(sec_max_lbl), "%.1f", sec_max_float);
	lv_obj_set_style_text_align(lv_textarea_get_label(sec_max_lbl), LV_TEXT_ALIGN_RIGHT, 0);
	lv_obj_set_style_border_color(sec_max_lbl, UI_COLOR_WHITE, LV_PART_CURSOR | LV_STATE_FOCUSED);
	lv_obj_add_style(sec_max_lbl, &textarea_style, 0);
	lv_obj_add_event_cb(sec_max_lbl, prv_axis_lbl_pressed_cb, LV_EVENT_SINGLE_CLICKED, &prv_sec_axis_max);
	lv_obj_set_style_text_color(sec_max_lbl, UI_COLOR_WHITE, 0);
	if (prv_sec_axis_max == 0x80000000)
	{
		lv_obj_set_flag(sec_max_lbl, LV_OBJ_FLAG_HIDDEN, true);
	}

	lv_obj_t* sec_min_lbl = lv_textarea_create(prv_chart);
	lv_obj_set_width(sec_min_lbl, 100);
	lv_textarea_set_one_line(sec_min_lbl, true);
	lv_obj_align(sec_min_lbl, LV_ALIGN_BOTTOM_RIGHT, 10, 35);
	float sec_min_float = (float)prv_sec_axis_min / UI_GRAPH_Y_AXIS_MULTIPLIER;
	lv_label_set_text_fmt(lv_textarea_get_label(sec_min_lbl), "%.1f", sec_min_float);
	lv_obj_set_style_text_align(lv_textarea_get_label(sec_min_lbl), LV_TEXT_ALIGN_RIGHT, 0);
	lv_obj_set_style_border_color(sec_min_lbl, UI_COLOR_WHITE, LV_PART_CURSOR | LV_STATE_FOCUSED);
	lv_obj_add_style(sec_min_lbl, &textarea_style, 0);
	lv_obj_add_event_cb(sec_min_lbl, prv_axis_lbl_pressed_cb, LV_EVENT_SINGLE_CLICKED, &prv_sec_axis_min);
	if (prv_sec_axis_min == 0x7FFFFFFF)
	{
		lv_obj_set_flag(sec_min_lbl, LV_OBJ_FLAG_HIDDEN, true);
	}

	lv_obj_t* time_zero_lbl = lv_textarea_create(prv_chart);
	lv_obj_set_width(time_zero_lbl, 100);
	lv_textarea_set_one_line(time_zero_lbl, true);
	lv_obj_align(time_zero_lbl, LV_ALIGN_BOTTOM_LEFT, -35, 15);
	lv_obj_set_style_text_align(lv_textarea_get_label(time_zero_lbl), LV_TEXT_ALIGN_LEFT, 0);
	lv_obj_set_style_border_color(time_zero_lbl, UI_COLOR_WHITE, LV_PART_CURSOR | LV_STATE_FOCUSED);
	lv_obj_add_style(time_zero_lbl, &textarea_style, 0);
	lv_label_set_text_fmt(lv_textarea_get_label(time_zero_lbl), "%.1fs", prv_min_time_s);
	lv_obj_add_event_cb(time_zero_lbl, prv_axis_lbl_pressed_cb, LV_EVENT_SINGLE_CLICKED, &prv_min_time_s);

	lv_obj_t* time_lbl = lv_textarea_create(prv_chart);
	lv_obj_set_width(time_lbl, 100);
	lv_textarea_set_one_line(time_lbl, true);
	lv_obj_align(time_lbl, LV_ALIGN_BOTTOM_RIGHT, 35, 15);
	lv_obj_set_style_text_align(lv_textarea_get_label(time_lbl), LV_TEXT_ALIGN_RIGHT, 0);
	lv_obj_set_style_border_color(time_lbl, UI_COLOR_WHITE, LV_PART_CURSOR | LV_STATE_FOCUSED);
	lv_obj_add_style(time_lbl, &textarea_style, 0);
	lv_label_set_text_fmt(lv_textarea_get_label(time_lbl), "%.1fs", prv_max_time_s); 
	lv_obj_add_event_cb(time_lbl, prv_axis_lbl_pressed_cb, LV_EVENT_SINGLE_CLICKED, &prv_max_time_s);

	lv_chart_refresh(prv_chart);
}

static void prv_set_series_y_axis(lv_chart_series_t* ser, lv_chart_axis_t y_axis)
{
	ser->y_axis_sec = y_axis & LV_CHART_AXIS_SECONDARY_Y ? 1 : 0;
}

static lv_chart_axis_t prv_get_series_y_axis(lv_chart_series_t* ser)
{
	if (ser->y_axis_sec == 1)
	{
		return LV_CHART_AXIS_SECONDARY_Y;
	}
	else
	{
		return LV_CHART_AXIS_PRIMARY_Y;
	}
}

/**********		GLOBAL FUNCTION DEFINITIONS		**********/
void ui_graph_init()
{
	prv_main_container = ui_gauges_get_graph_container_obj();
	lv_obj_set_size(prv_main_container, 460, 480);
	lv_obj_set_style_bg_color(prv_main_container, UI_COLOR_BLACK, LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(prv_main_container, 0, 0);
	lv_obj_set_style_pad_top(prv_main_container, 120, 0);
	lv_obj_set_style_pad_bottom(prv_main_container, 120, 0);
	lv_obj_set_style_pad_left(prv_main_container, 10, 0);
	lv_obj_set_style_pad_right(prv_main_container, 10, 0);
	lv_obj_set_flex_flow(prv_main_container, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_style_flex_main_place(prv_main_container, LV_FLEX_ALIGN_START, 0);
	lv_obj_set_style_flex_cross_place(prv_main_container, LV_FLEX_ALIGN_CENTER, 0);
	lv_obj_set_style_flex_track_place(prv_main_container, LV_FLEX_ALIGN_SPACE_EVENLY, 0);

	/* Create the chart. */
	prv_chart = lv_chart_create(prv_main_container);
	lv_obj_set_size(prv_chart, lv_pct(100), 240);
	lv_obj_set_style_bg_color(prv_chart, UI_COLOR_BLACK, 0);
	lv_obj_set_style_border_color(prv_chart, UI_COLOR_GRAY, 0);
	lv_obj_set_style_line_color(prv_chart, UI_COLOR_GRAY, 0);
	lv_obj_set_style_size(prv_chart, 0, 0, LV_PART_INDICATOR);
	lv_chart_set_type(prv_chart, LV_CHART_TYPE_LINE);
	lv_obj_set_style_pad_top(prv_chart, 25, LV_STATE_DEFAULT);
	lv_obj_set_style_pad_bottom(prv_chart, 25, LV_STATE_DEFAULT);
	lv_obj_set_style_pad_right(prv_chart, 25, LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(prv_chart, 25, LV_STATE_DEFAULT);
	lv_obj_remove_flag(prv_chart, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_set_scrollbar_mode(prv_chart, LV_SCROLLBAR_MODE_OFF);
	lv_chart_set_update_mode(prv_chart, LV_CHART_UPDATE_MODE_SHIFT);

	/* Create a container to hold info about the data series. */
	prv_series_info_container = lv_obj_create(prv_main_container);
	lv_obj_set_size(prv_series_info_container, lv_pct(100), LV_SIZE_CONTENT);
	lv_obj_set_style_bg_color(prv_series_info_container, UI_COLOR_BLACK, 0);
	lv_obj_set_style_border_width(prv_series_info_container, 0, 0);
	lv_obj_set_style_pad_bottom(prv_series_info_container, 0, 0);
	lv_obj_set_style_pad_top(prv_series_info_container, 0, 0);
	lv_obj_set_style_pad_left(prv_series_info_container, 20, 0);
	lv_obj_set_style_pad_right(prv_series_info_container, 20, 0);
	lv_obj_set_flex_flow(prv_series_info_container, LV_FLEX_FLOW_ROW_WRAP);
	lv_obj_set_style_flex_main_place(prv_series_info_container, LV_FLEX_ALIGN_SPACE_BETWEEN, 0);
	lv_obj_set_style_flex_cross_place(prv_series_info_container, LV_FLEX_ALIGN_CENTER, 0);
	lv_obj_set_style_flex_track_place(prv_series_info_container, LV_FLEX_ALIGN_CENTER, 0);


	/* Create the file list. */
	lv_obj_t* file_list_lbl = lv_label_create(prv_main_container);
	lv_label_set_text_static(file_list_lbl, "Data log files will appear in the list below. Tap to display on the chart. Once displayed, the y-axis can be changed with the toggle button.");
	lv_obj_set_width(file_list_lbl, 400);
	lv_obj_set_style_text_color(file_list_lbl, UI_COLOR_WHITE, 0);
	lv_obj_set_style_text_align(file_list_lbl, LV_TEXT_ALIGN_CENTER, 0);
	lv_obj_set_style_text_font(file_list_lbl, &lv_font_montserrat_14, 0);
	lv_label_set_long_mode(file_list_lbl, LV_LABEL_LONG_MODE_WRAP);
	prv_files_list = lv_list_create(prv_main_container);
	lv_obj_set_flag(prv_files_list, LV_OBJ_FLAG_SCROLLABLE, true);
	lv_obj_set_size(prv_files_list, lv_pct(100), 200);
	lv_obj_set_style_bg_color(prv_files_list, UI_COLOR_BLACK, 0);
	lv_obj_set_style_border_color(prv_files_list, UI_COLOR_GRAY, 0);


	/* Create the delete button. */
	prv_delete_btn = ui_helpers_create_btn_with_text(prv_main_container, "Delete", LV_FONT_DEFAULT);
}

void ui_graph_set_timebase(uint32_t max_ms, uint32_t increment_ms)
{
	prv_max_time_ms = max_ms;
	prv_timebase_ms = increment_ms;
	prv_max_time_s = ((float)max_ms / 1000.0);
}

uint32_t ui_graph_get_number_of_list_items()
{
	uint32_t rtn_val = lv_obj_get_child_count(prv_files_list);
	return rtn_val;
}

void ui_graph_clear_all_chart_data()
{
	lv_chart_series_t* series = lv_chart_get_series_next(prv_chart, NULL);
	while (series != NULL)
	{
		int32_t* int_arr = lv_chart_get_series_y_array(prv_chart, series);
		int_arr -= prv_x_axis_point_offset;
		free(int_arr);
		lv_chart_remove_series(prv_chart, series);
		series = lv_chart_get_series_next(prv_chart, NULL);

	}
	prv_pri_axis_max = 0x80000000;
	prv_pri_axis_min = 0x7FFFFFFF;
	prv_sec_axis_max = 0x80000000;
	prv_sec_axis_min = 0x7FFFFFFF;
	prv_max_time_s = 0;
	lv_obj_clean(prv_chart);
	lv_obj_clean(prv_series_info_container);
}

lv_obj_t* ui_graph_add_file_to_list(const char* file_name)
{
	lv_obj_t* btn = lv_list_add_button(prv_files_list, NULL, file_name);
	lv_obj_add_flag(btn, LV_OBJ_FLAG_EVENT_BUBBLE);
	lv_obj_set_style_bg_color(btn, UI_COLOR_DARK_RED, LV_STATE_PRESSED);
	lv_obj_set_style_bg_color(btn, UI_COLOR_DARK_RED, LV_STATE_USER_1);		//User State 1 is set when a btn is clicked and cleared when a different button is clicked.
	lv_obj_set_style_bg_color(btn, UI_COLOR_BLACK, 0);	
	lv_obj_set_style_text_color(btn, UI_COLOR_WHITE, 0);
	lv_obj_set_style_transform_width(btn, 100, LV_STATE_USER_1);
	//lv_obj_set_style_width(btn, 480, LV_STATE_USER_1);
	lv_obj_set_style_border_color(btn, UI_COLOR_DARK_GRAY, 0);
	lv_obj_set_style_border_width(btn, 2, 0);
	lv_obj_add_event_cb(btn, prv_list_btn_pressed_handler, LV_EVENT_SHORT_CLICKED, NULL);
	return btn;
}

void ui_graph_clear_file_list()
{
	lv_obj_clean(prv_files_list);
}

const char* ui_graph_get_file_list_item(uint32_t index)
{
	lv_obj_t* btn = lv_obj_get_child(prv_files_list, index);
	if (btn == NULL) { return NULL; }
	lv_obj_t * lbl = lv_obj_get_child(btn, 0);
	const char * text = lv_label_get_text(lbl);
	return text;
}

const char* ui_graph_get_selected_file()
{
	return prv_selected_file_name;
}

lv_obj_t* ui_graph_get_main_container()
{
	return prv_main_container;
}

void ui_graph_delete_file_from_list(uint32_t index)
{
	lv_obj_t* btn = lv_obj_get_child(prv_files_list, index);
	if (btn != NULL)
	{
		lv_obj_delete(btn);
	}

}

void ui_graph_add_series_data(float* data_arr, uint32_t arr_size_floats, char* name, lv_color_t color)
{
	/* Everything gets multiplied by 10 in case there are values like A/F ratio that are between like 0-2. */
	int32_t* int_arr = (int32_t*)malloc(arr_size_floats * sizeof(float));	//This will be freed in ui_graph_clear_all_data.
	if (int_arr == NULL)
	{
		return;
	}
	for(uint32_t i = 0; i < arr_size_floats; i++)
	{
		int_arr[i] = (int32_t)roundf(UI_GRAPH_Y_AXIS_MULTIPLIER * data_arr[i]);
	}

	lv_chart_set_point_count(prv_chart, arr_size_floats);
	lv_chart_series_t* lv_series = lv_chart_add_series(prv_chart, color, LV_CHART_AXIS_PRIMARY_Y);
	//lv_chart_set_series_values(prv_chart, lv_series, int_arr, (size_t)arr_size_floats);
	lv_chart_set_series_ext_y_array(prv_chart, lv_series, int_arr);
	lv_obj_t* lbl = lv_label_create(prv_series_info_container);
	
	/* Check if name ends with a newline and trim it off if it does. */
	name[ strcspn(name, "\n") ] = 0;			//https://stackoverflow.com/questions/2693776/removing-trailing-newline-character-from-fgets-input
	lv_label_set_text(lbl, name);
	lv_obj_set_style_text_color(lbl, color, 0);
	lv_obj_set_width(lbl, lv_pct(65));
	lv_label_set_long_mode(lbl, LV_LABEL_LONG_WRAP);
	lv_obj_t* cb = lv_checkbox_create(prv_series_info_container);
	lv_checkbox_set_text_static(cb, "2nd axis?");
	lv_obj_set_style_text_font(cb, &lv_font_montserrat_12, 0);
	lv_obj_set_style_pad_all(cb, 7, LV_PART_INDICATOR);
	lv_obj_set_style_text_color(cb, UI_COLOR_WHITE,0);
	lv_obj_set_height(cb, 35);
	lv_obj_set_width(cb, lv_pct(30));
	lv_obj_add_event_cb(cb, prv_series_switch_event_handler, LV_EVENT_VALUE_CHANGED, lv_series);
	lv_obj_set_style_bg_color(cb, UI_COLOR_RED, LV_STATE_CHECKED | LV_PART_INDICATOR);
	lv_obj_set_style_border_color(cb, UI_COLOR_RED, LV_PART_INDICATOR);

	prv_update_chart();
}

void ui_graph_set_delete_btn_cb(lv_event_cb_t func)
{
	if (func == NULL) { return; }
	lv_obj_add_event_cb(prv_delete_btn, func, LV_EVENT_SHORT_CLICKED, NULL);
}

void ui_graph_set_file_list_event_cb(lv_event_cb_t func)
{
	lv_obj_add_event_cb(prv_files_list, func, LV_EVENT_SHORT_CLICKED, NULL);
}
