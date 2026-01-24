
#ifndef _UI_GAUGES_H_
#define _UI_GAUGES_H_

#include "lvgl/lvgl.h"
#include "ui_helpers.h"

#ifdef __cplusplus
extern "C" {
#endif

/**********     INCLUDES        **********/

/**********     DEFINES      **********/

/**********     GLOBAL VARIABLE DECLRATIONS     **********/

/**********		GLOBAL FUNCTION DECLRATIONS		**********/
void ui_gauges_load();
/**
* ui_gauge_set_gauge_value
* params:
* val:
*/
void ui_gauges_set_gauge_value(float val);
void ui_gauges_create_gauge_btn(const char* name);
void ui_gauges_create_gauge(const char* name, const char* units, uint32_t min, uint32_t max);

void ui_gauges_set_gauge_select_btn_cb(void (*func)(lv_event_t* e));
void ui_gauges_set_back_btn_cb(void (*func)(lv_event_t* e));
void ui_gauges_set_gauge_cb(void (*func)(lv_event_t* e));
void ui_gauges_set_scr_load_cb(lv_event_cb_t func);



#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif  //_TEMPLATE_H_
