
#ifndef _UI_GRAPH_H_
#define _UI_GRAPH_H_

#ifdef __cplusplus
extern "C" {
#endif

/**********     INCLUDES        **********/
#include <stddef.h>

/**********     TYPEDEFS         **********/

/**********     DEFINES      **********/
/**
* @brief The value to multiply the data by before displaying it on the graph.
* This is done incase of data with small variance. For example, air/fuel ratio in the
* form of lambda hangs around a value of 1 +/- 0.1. If we don't amplify the floats
* before converting to ints for the grap these values would all be 1. By multiplying
* by 100 before now we have some better resolution on the graph.
*/
#define UI_GRAPH_Y_AXIS_MULTIPLIER			100.0

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

/**
* @brief Converts an array of floats to an array of integers and displays them on the graph. Adds a 
* color coordinated label with the name of the series to the container below the graph. The values
* in the float array are multiplied by a factor of UI_GRAPH_Y_AXIS_MULTIPLIER before they are converted
* to integers.
* @param data_arr The array of floats.
* @param arr_size_floats How many floats are in the array.
* @param name The name of the series, this will be added to the container below the graph.
* @param color The color of the series label and graph line.
*/
void ui_graph_add_series_data(float* data_arr, uint32_t arr_size_floats, char* name, lv_color_t color);
void ui_graph_set_delete_btn_cb(lv_event_cb_t func);
void ui_graph_set_file_list_event_cb(lv_event_cb_t func);

void ui_graph_clear_all_chart_data();
lv_obj_t* ui_graph_add_file_to_list(const char* file_name);
void ui_graph_clear_file_list();
void ui_graph_delete_file_from_list(uint32_t index);




#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif  //_TEMPLATE_H_
