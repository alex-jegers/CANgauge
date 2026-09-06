
#ifndef _UI_GRAPH_H_
#define _UI_GRAPH_H_

#ifdef __cplusplus
extern "C" {
#endif

/**********     INCLUDES        **********/

/**********     TYPEDEFS         **********/

/**********     DEFINES      **********/

/**********     GLOBAL VARIABLE DECLRATIONS     **********/

/**********		GLOBAL FUNCTION DECLRATIONS		**********/
/**
 * name:
 *      desc:
 *      params:
 *      returns:
 */
void ui_graph_init();
void ui_graph_set_timebase(uint32_t max_ms, uint32_t increment_ms);

uint32_t ui_graph_get_number_of_list_items();
const char* ui_graph_get_file_list_item(uint32_t index);
const char* ui_graph_get_selected_file();
lv_obj_t* ui_graph_get_main_container();

void ui_graph_add_series_lbl(const char* name);
void ui_graph_add_series_data(float* data_arr, uint32_t arr_size_floats, char* name, lv_color_t color);
void ui_graph_set_delete_btn_cb(lv_event_cb_t func);
void ui_graph_set_file_list_event_cb(lv_event_cb_t func);

void ui_graph_clear_all_chart_data();
lv_obj_t* ui_graph_add_file_to_list(char* file_name);
void ui_graph_clear_file_list();
void ui_graph_delete_file_from_list(uint32_t index);




#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif  //_TEMPLATE_H_
