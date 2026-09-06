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
static int32_t prv_pri_axis_max = 0x80000000;
static int32_t prv_pri_axis_min = 0x7FFFFFFF;
static int32_t prv_sec_axis_max = 0x80000000;
static int32_t prv_sec_axis_min = 0x7FFFFFFF;
/**********		STATIC FUNCTION DECLRATIONS		**********/
static void prv_list_btn_pressed_handler(lv_event_t* e);
static void prv_series_switch_event_handler(lv_event_t* e);			//User data contained in the event is the lv_series_t.
static float prv_get_array_max_val_f(float* arr, uint32_t arr_len);
static float prv_get_array_min_val_f(float* arr, uint32_t arr_len);
static int32_t prv_get_array_max_val_i32(int32_t* arr, uint32_t arr_len);
static int32_t prv_get_array_min_val_i32(int32_t* arr, uint32_t arr_len);
static void prv_update_chart();
static void prv_set_series_y_axis(lv_chart_series_t* ser, lv_chart_axis_t y_axis);
static lv_chart_axis_t prv_get_series_y_axis(lv_chart_series_t* ser);

/**********		STATIC FUNCTION DEFINITIONS		**********/
static void prv_list_btn_pressed_handler(lv_event_t* e)
{
	lv_obj_t* btn = lv_event_get_target(e);
	lv_obj_t* lbl = lv_obj_get_child(btn, 0);
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

	/* 4 labels, primary max, primary min, secondary max, secondary min. */

	lv_obj_t* pri_max_lbl = lv_label_create(prv_chart);
	lv_obj_set_style_text_font(pri_max_lbl, &lv_font_montserrat_12, 0);
	lv_obj_align(pri_max_lbl, LV_ALIGN_TOP_LEFT, 0, -10);
	lv_obj_set_style_text_align(pri_max_lbl, LV_TEXT_ALIGN_LEFT, 0);
	float pri_max_float = (float)prv_pri_axis_max / 10.0;
	lv_label_set_text_fmt(pri_max_lbl, "%.1f", pri_max_float);
	lv_obj_set_style_text_color(pri_max_lbl, UI_COLOR_WHITE, 0);
	if (prv_pri_axis_max == 0x80000000)
	{
		lv_obj_set_flag(pri_max_lbl, LV_OBJ_FLAG_HIDDEN, true);
	}

	lv_obj_t* pri_min_lbl = lv_label_create(prv_chart);
	lv_obj_set_style_text_font(pri_min_lbl, &lv_font_montserrat_12, 0);
	lv_obj_align(pri_min_lbl, LV_ALIGN_BOTTOM_LEFT, 0, 0);
	lv_obj_set_style_text_align(pri_max_lbl, LV_TEXT_ALIGN_LEFT, 0);
	float pri_min_float = (float)prv_pri_axis_min / 10.0;
	lv_label_set_text_fmt(pri_min_lbl, "%.1f", pri_min_float);
	lv_obj_set_style_text_color(pri_min_lbl, UI_COLOR_WHITE, 0);
	if (prv_pri_axis_min == 0x7FFFFFFF)
	{
		lv_obj_set_flag(pri_min_lbl, LV_OBJ_FLAG_HIDDEN, true);
	}

	lv_obj_t* sec_max_lbl = lv_label_create(prv_chart);
	lv_obj_set_style_text_font(sec_max_lbl, &lv_font_montserrat_12, 0);
	lv_obj_align(sec_max_lbl, LV_ALIGN_TOP_RIGHT, 0, -10);
	lv_obj_set_style_text_align(sec_max_lbl, LV_TEXT_ALIGN_RIGHT, 0);
	float sec_max_float = (float)prv_sec_axis_max / 10.0;
	lv_label_set_text_fmt(sec_max_lbl, "%.1f", sec_max_float);
	lv_obj_set_style_text_color(sec_max_lbl, UI_COLOR_WHITE, 0);
	if (prv_sec_axis_max == 0x80000000)
	{
		lv_obj_set_flag(sec_max_lbl, LV_OBJ_FLAG_HIDDEN, true);
	}

	lv_obj_t* sec_min_lbl = lv_label_create(prv_chart);
	lv_obj_set_style_text_font(sec_min_lbl, &lv_font_montserrat_12, 0);
	lv_obj_align(sec_min_lbl, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
	lv_obj_set_style_text_align(sec_min_lbl, LV_TEXT_ALIGN_RIGHT, 0);
	float sec_min_float = (float)prv_sec_axis_min / 10.0;
	lv_label_set_text_fmt(sec_min_lbl, "%.1f", sec_min_float);
	lv_obj_set_style_text_color(sec_min_lbl, UI_COLOR_WHITE, 0);
	if (prv_sec_axis_min == 0x7FFFFFFF)
	{
		lv_obj_set_flag(sec_min_lbl, LV_OBJ_FLAG_HIDDEN, true);
	}

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
	lv_obj_set_style_pad_bottom(prv_chart, 25, LV_STATE_DEFAULT);
	lv_obj_set_style_pad_right(prv_chart, 25, LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(prv_chart, 25, LV_STATE_DEFAULT);

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
	lv_obj_set_style_flex_main_place(prv_series_info_container, LV_FLEX_ALIGN_START, 0);
	lv_obj_set_style_flex_cross_place(prv_series_info_container, LV_FLEX_ALIGN_CENTER, 0);
	lv_obj_set_style_flex_track_place(prv_series_info_container, LV_FLEX_ALIGN_SPACE_BETWEEN, 0);

	/* Create the file list. */
	prv_files_list = lv_list_create(prv_main_container);
	lv_obj_set_flag(prv_files_list, LV_OBJ_FLAG_SCROLLABLE, true);
	lv_obj_set_size(prv_files_list, lv_pct(100), 200);

	/* Create the delete button. */
	prv_delete_btn = ui_helpers_create_btn_with_text(prv_main_container, "Delete", LV_FONT_DEFAULT);
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
		lv_chart_remove_series(prv_chart, series);
		series = lv_chart_get_series_next(prv_chart, NULL);
	}
	prv_pri_axis_max = 0x80000000;
	prv_pri_axis_min = 0x7FFFFFFF;
	prv_sec_axis_max = 0x80000000;
	prv_sec_axis_min = 0x7FFFFFFF;
	lv_obj_clean(prv_chart);
	lv_obj_clean(prv_series_info_container);
}

lv_obj_t* ui_graph_add_file_to_list(char* file_name)
{
	lv_obj_t* btn = lv_list_add_button(prv_files_list, NULL, file_name);
	lv_obj_add_flag(btn, LV_OBJ_FLAG_EVENT_BUBBLE);
	lv_obj_set_style_bg_color(btn, UI_COLOR_LIGHT_RED, LV_STATE_FOCUSED);
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
	lv_obj_delete(btn);
}

void ui_graph_add_series_lbl(const char* name)
{
	lv_obj_t* lbl = lv_label_create(prv_series_info_container);
	lv_label_set_text(lbl, name);
}

void ui_graph_add_series_data(float* data_arr, uint32_t arr_size_floats, char* name, lv_color_t color)
{
	/* Everything gets multiplied by 10 in case there are values like A/F ratio that are between like 0-2. */
	int32_t* int_arr = (int32_t*)malloc(arr_size_floats * sizeof(float));

	for(uint32_t i = 0; i < arr_size_floats; i++)
	{
		int_arr[i] = (int32_t)roundf(10 * data_arr[i]);
	}

	lv_chart_set_point_count(prv_chart, arr_size_floats);
	lv_chart_series_t* lv_series = lv_chart_add_series(prv_chart, color, LV_CHART_AXIS_PRIMARY_Y);
	lv_chart_set_series_values(prv_chart, lv_series, int_arr, (size_t)arr_size_floats);

	lv_obj_t* lbl = lv_label_create(prv_series_info_container);
	/* Check if name ends with a newline and trim it off if it does. */
	name[ strcspn(name, "\n") ] = 0;			//https://stackoverflow.com/questions/2693776/removing-trailing-newline-character-from-fgets-input
	lv_label_set_text(lbl, name);
	lv_obj_set_style_text_color(lbl, color, 0);
	lv_obj_set_width(lbl, lv_pct(50));
	lv_label_set_long_mode(lbl, LV_LABEL_LONG_WRAP);
	lv_obj_t* sw = lv_switch_create(prv_series_info_container);
	lv_obj_set_height(sw, 40);
	lv_obj_add_event_cb(sw, prv_series_switch_event_handler, LV_EVENT_VALUE_CHANGED, lv_series);

	prv_update_chart();
	/*
	prv_pri_axis_max = (prv_pri_axis_max < max) ? max : prv_pri_axis_max;
	prv_pri_axis_min = (prv_pri_axis_min > min) ? min : prv_pri_axis_min;
	lv_chart_set_axis_max_value(prv_chart, LV_CHART_AXIS_PRIMARY_Y, prv_pri_axis_max);
	lv_chart_set_axis_min_value(prv_chart, LV_CHART_AXIS_PRIMARY_Y, prv_pri_axis_min);
	*/
	free(int_arr);
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
