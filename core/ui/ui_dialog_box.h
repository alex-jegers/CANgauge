
#ifndef _UI_DIALOG_BOX_H
#define _UI_DIALOG_BOX_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl/lvgl.h"

lv_obj_t** ui_dialog_box_run(const char* text);
void ui_dialog_box_change_text(const char* text);
void ui_dialog_box_close();




#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
