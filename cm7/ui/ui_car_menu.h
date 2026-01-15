
#ifndef _UI_CAR_MENU_H_
#define _UI_CAR_MENU_H_

#ifdef __cplusplus
extern "C" {
#endif

/**********     INCLUDES        **********/
#include "lvgl/lvgl.h"
#include "ui_helpers.h"

/**********     DEFINES      **********/

/**********     GLOBAL VARIABLE DECLRATIONS     **********/

/**********		GLOBAL FUNCTION DECLRATIONS		**********/
void ui_car_load_menu_screen();
void ui_car_set_gauges_load_btn_clicked_cb(lv_event_cb_t func);
void ui_car_set_can_sniffer_btn_clicked_cb(lv_event_cb_t func);
void ui_car_set_dtc_btn_clicked_cb(lv_event_cb_t func);
void ui_car_set_data_logger_btn_clicked_cb(lv_event_cb_t func);



#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif  //_UI_CAR_MENU_H_
