/*
 * ui_loading.h
 *
 *  Created on: May 25, 2024
 *      Author: awjpp
 */

#ifndef UI_UI_LOADING_H_
#define UI_UI_LOADING_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl/lvgl.h"
#include "ui_helpers.h"
#include "ui_events.h"
#include "stdbool.h"

/**********     GLOBAL VARIABLE DECLRATIONS     **********/
extern lv_obj_t* ui_loading_screen;
extern lv_obj_t* ui_loading_error_screen;

/**********     GLOBAL FUNCTION DECLARATIONS     **********/
void ui_init_loading_screen();
void ui_load_loading_screen();
void ui_delete_loading_screen();

void ui_init_loading_error_screen();
void ui_load_loading_error_screen();
void ui_delete_loading_error_screen();

void ui_assign_demo_btn_cb(void (*func)(lv_event_t*));
void ui_assign_retry_btn_cb(void (*func)(lv_event_t*));

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* UI_UI_LOADING_H_ */
