
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
void ui_gauges_set_gauge_value(float val, uint8_t idx);
void ui_gauges_create_gauge_btn(const char* name);			//TODO: Remove, obsolete.
void ui_gauges_create_gauge_checkbox(const char* name);
void ui_gauges_create_gauge(const char* name, const char* units, int32_t min, int32_t max, uint8_t gauge_idx);
void ui_gauges_set_number_of_gauges(uint8_t num_gauges);
void ui_load_gauge_screen();

/**
 * Callbacks for UI element events.
 */
void ui_gauges_set_gauge_select_btn_cb(void (*func)(lv_event_t* e));	//The buttons that select a gauge (to be replaced with checkboxes.
void ui_gauges_set_gauge_cb(void (*func)(lv_event_t* e));				//When the gauge screen itself is pressed.
void ui_gauges_set_scr_load_cb(lv_event_cb_t func);						//When the main screen is loaded.
void ui_set_brightness_slider_event_cb(lv_event_cb_t func);				//The brightness slider is interacted with.
void ui_set_settings_scr_load_event_cb(lv_event_cb_t func);				//The settings screen loads.
void ui_set_demo_mode_checkbox_event_cb(lv_event_cb_t func);			//The demo mode checkbox state is changed.
void ui_set_settings_btn_event_cb(lv_event_cb_t func);					//The button to load the settings screen is pressed.
void ui_set_settings_back_btn_event_cb(lv_event_cb_t func);				//The back button within the settings screen is pressed.
void ui_gauges_set_view_btn_cb(lv_event_cb_t func);
void ui_set_settings_firmware_update_btn_event_cb(lv_event_cb_t func);	//The button to start a firmware update.
void ui_set_settings_data_trnsf_btn_event_cb(lv_event_cb_t func);		//Used to connect the EEPROM file system over USB.

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif  //_TEMPLATE_H_
