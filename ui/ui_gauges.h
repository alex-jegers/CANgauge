

#ifndef _SQUARELINE_PROJECT_UI_H
#define _SQUARELINE_PROJECT_UI_H

#ifdef __cplusplus
extern "C" {
#endif

/**********		DEFINES		**********/
#include "lvgl/lvgl.h"
#include "ui_helpers.h"
#include "ui_events.h"
#include "stdbool.h"

#define UI_NUMBER_OF_GAUGES		4

/**********     GLOBAL VARIABLE DECLRATIONS.     **********/
extern lv_obj_t* ui_main_screen;						//Parent to the ui_main_screen_panel.
extern lv_obj_t* ui_main_scr_panel;						//Parent to all the gauges and first menu screen.

extern lv_obj_t* ui_settings_btn_panel;
extern ui_gauge_t ui_coolant_temp;
extern ui_gauge_t ui_boost_pressure;
extern ui_gauge_t ui_oil_level;
extern ui_gauge_t ui_af_ratio;
extern ui_gauge_t* ui_gauge_list[];

/**********		GLOBAL FUNCTION DECLRATIONS.		**********/
void ui_init_gauge_screen();									//initialize all gauge ui elements.
void ui_load_gauge_screen();									//load the gauges ui.
void ui_delete_gauge_screen();									//deletes all the gauge ui elements.

void ui_coolant_temp_set_value(int32_t val);
void ui_boost_set_value(int32_t val);
void ui_oil_level_set_value(int32_t val);
void ui_af_set_value(int32_t val);


void ui_assign_gauge_clicked_cb(void (*func)(lv_event_t* e));	//assigns a function pointer to a function that will be called when a gauge is clicked.
void ui_assign_dtc_reader_clicked_cb(void (*func)(lv_event_t* e));

ui_gauge_t* ui_get_gauge_from_lv_obj(lv_obj_t* obj);			//TODO: Can this be static?




#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif

