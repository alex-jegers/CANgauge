/*
 * ui_dtc_reader.h
 *
 *  Created on: April 10, 2025
 *      Author: Alex Jegers
 */


#ifndef UI_DTC_READER_H_
#define UI_DTC_READER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl/lvgl.h"

void ui_init_dtc_reader();
void ui_load_dtc_reader();
void ui_dtc_reader_assign_back_btn_clicked_handler(void (*func)(lv_event_t* e));
void ui_dtc_reader_assign_read_btn_clicked_handler(void (*func)(lv_event_t* e));
void ui_dtc_reader_assign_clear_btn_clicked_handler(void (*func)(lv_event_t* e));

#ifdef __cplusplus
}
#endif

#endif UI_DTC_READER_H_

