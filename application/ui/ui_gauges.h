
#ifndef _UI_GAUGES_H_
#define _UI_GAUGES_H_

#include "lvgl.h"
#include "ui_helpers/ui_helpers.h"

#ifdef __cplusplus
extern "C" {
#endif

/**********     INCLUDES        **********/

/**********     DEFINES      **********/
LV_IMAGE_DECLARE(cangauge_logo_header);

/**********     GLOBAL VARIABLE DECLRATIONS     **********/

/**********		GLOBAL FUNCTION DECLRATIONS		**********/
void ui_gauges_load();
void ui_gauges_init();
void ui_gauges_delete();		//Delete all the ui elements.
/**
* ui_gauge_set_gauge_value
* params:
* val:
*/
void ui_gauges_set_gauge_value(float val, uint8_t idx);
void ui_gauges_create_gauge_btn(const char* name);			//TODO: Remove, obsolete.
void ui_gauges_create_gauge_checkbox(const char* name);
void ui_gauges_create_gauge(const char* name, const char* units, int32_t min, int32_t max, uint8_t gauge_idx);
void ui_gauges_set_number_of_gauges(uint8_t num_gauges);
void ui_load_gauge_screen();
void ui_delete_gauge_select_checkboxes();
lv_obj_t* ui_gauges_get_options_container_obj();
void ui_gauges_show_recording();				//Creates a small label on the top layer with blinking light that says "Rec." for when the device is data logging.
void ui_gauges_hide_recording();				//

/**
 * Callbacks for UI element events.
 */
void ui_gauges_set_gauge_single_clicked_cb(void (*func)(lv_event_t* e));				//When the gauge screen itself is pressed.
void ui_gauges_set_gauge_long_pressed_cb(void (*func)(lv_event_t* e));
void ui_gauges_set_scr_load_cb(lv_event_cb_t func);						//When the main screen is loaded.
void ui_gauges_set_view_btn_cb(lv_event_cb_t func);
void ui_set_settings_btn_event_cb(lv_event_cb_t func);
void ui_add_refresh_btn_event_cb(lv_event_cb_t func);					//Refresh the CAN connection.
void ui_add_clear_btn_event_cb(lv_event_cb_t func);						//Clear the checked checkboxes.

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif  //_TEMPLATE_H_
