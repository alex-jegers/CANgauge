/**********     INCLUDES        **********/
#include "lvgl.h"
#include "ui_gauges_prv.h"
#include "ui_graph.h"
#include "ui_helpers/ui_helpers.h"
/**********     TYPEDEFS         **********/

/**********		DEFINES		**********/

/**********		EXTERNAL VARIABLE DEFINITIONS		**********/

/**********		STATIC VARIABLES		**********/
static lv_obj_t* prv_main_container;
static lv_obj_t* prv_chart;
static lv_obj_t* prv_files_list;
/**********		STATIC FUNCTION DECLRATIONS		**********/

/**********		STATIC FUNCTION DEFINITIONS		**********/

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
	lv_obj_set_style_flex_main_place(prv_main_container, LV_FLEX_ALIGN_CENTER, 0);
	lv_obj_set_style_flex_cross_place(prv_main_container, LV_FLEX_ALIGN_START, 0);
	lv_obj_set_style_flex_track_place(prv_main_container, LV_FLEX_ALIGN_SPACE_EVENLY, 0);

	prv_chart = lv_chart_create(prv_main_container);
	lv_obj_set_size(prv_chart, lv_pct(100), 240);
	lv_chart_set_type(prv_chart, LV_CHART_TYPE_LINE);
	lv_obj_set_style_pad_bottom(prv_chart, 25, LV_STATE_DEFAULT);
	lv_obj_set_style_pad_right(prv_chart, 25, LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(prv_chart, 25, LV_STATE_DEFAULT);

	prv_files_list = lv_list_create(prv_main_container);
	lv_obj_set_size(prv_files_list, lv_pct(100), 200);
}

uint32_t ui_graph_get_number_of_list_items()
{
	return lv_obj_get_child_count(prv_files_list);
}

lv_obj_t* ui_graph_add_file_to_list(char* file_name)
{
	lv_obj_t* lbl = lv_list_add_button(prv_files_list, NULL, file_name);
	lv_obj_add_flag(lbl, LV_OBJ_FLAG_EVENT_BUBBLE);
	lv_obj_set_style_bg_color(lbl, UI_COLOR_LIGHT_RED, LV_STATE_FOCUSED);
	return lbl;
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

void ui_graph_delete_file_from_list(uint32_t index)
{
	lv_obj_t* btn = lv_obj_get_child(prv_files_list, index);
	lv_obj_delete(btn);
}
