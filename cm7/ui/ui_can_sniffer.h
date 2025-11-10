
#ifndef _UI_CAN_SNIFFER_H_
#define _UI_CAN_SNIFFER_H_

#ifdef __cplusplus
extern "C" {
#endif

/**********     INCLUDES        **********/
#include <stdint.h>
#include <stdlib.h>
#include "lvgl/lvgl.h"
/**********     DEFINES      **********/

/**********     GLOBAL VARIABLE DECLRATIONS     **********/
extern lv_obj_t* ui_can_sniffer_can1_baud_lbl;
extern lv_obj_t* ui_can_sniffer_can2_baud_lbl;
extern lv_obj_t* ui_can_sniffer_table;

/**********		GLOBAL FUNCTION DECLRATIONS		**********/
void ui_can_sniffer_load();
void ui_can_sniffer_assign_can1_connect_btn_cb(void (*func)());
void ui_can_sniffer_assign_can1_disconnect_btn_cb(void (*func)(lv_event_t* e));
void ui_can_sniffer_assign_can2_connect_btn_cb(void (*func)());
void ui_can_sniffer_assign_back_to_main_btn_cb(void (*func)());
void ui_can_sniffer_assign_tx_ctrl_btn_matrix_cb(void (*func)(lv_event_t* e));
char* ui_can_sniffer_get_can1_dd_list_text();
char* ui_can_sniffer_get_can2_dd_list_text();
uint32_t ui_can_sniffer_get_tx_table_selected_row();
char* ui_can_sniffer_get_tx_table_cell_value(uint32_t row, uint32_t column);
uint32_t ui_can_sniffer_get_tx_table_number_of_rows();



#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif  //_UI_CAN_SNIFFER_H_
