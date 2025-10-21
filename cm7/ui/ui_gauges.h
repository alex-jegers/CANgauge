
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
void ui_gauges_set_gauge_value(int32_t val);
void ui_gauges_set_gauge_select_btn_cb(void (*func)(lv_event_t* e));
void ui_gauges_set_back_btn_cb(void (*func)(lv_event_t* e));
void ui_gauges_set_gauge_cb(void (*func)(lv_event_t* e));



#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif  //_TEMPLATE_H_
