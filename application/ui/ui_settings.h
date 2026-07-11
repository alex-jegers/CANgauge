
#ifndef _UI_SETTINGS_H_
#define _UI_SETTINGS_H_

#include "lvgl.h"
#include "ui_helpers/ui_helpers.h"

#ifdef __cplusplus
extern "C" {
#endif

/**********     INCLUDES        **********/

/**********     TYPEDEFS         **********/

/**********     DEFINES      **********/

/**********     GLOBAL VARIABLE DECLRATIONS     **********/

/**********		GLOBAL FUNCTION DECLRATIONS		**********/
/**
 * name:
 *      desc:
 *      params:
 *      returns:
 */
void ui_settings_load();
void ui_settings_init();
void ui_settings_delete();
void ui_set_brightness_slider_event_cb(lv_event_cb_t func);				//The brightness slider is interacted with.
void ui_set_settings_scr_load_event_cb(lv_event_cb_t func);				//The settings screen loads.
void ui_set_settings_back_btn_event_cb(lv_event_cb_t func);				//The back button within the settings screen is pressed.
void ui_add_settings_firmware_update_btn_event_cb(lv_event_cb_t func);	//The button to start a firmware update.
void ui_set_settings_data_trnsf_btn_event_cb(lv_event_cb_t func);		//Used to connect the EEPROM file system over USB.
bool ui_settings_set_pressure_units_dropdown(char* units_str);	//Units_str must be PSI, kPa, or bar. Anything else will return false.
bool ui_settings_set_temperature_units_dropdown(char* units_str);	//Units_str must be C or F. Anything else will return false.
void ui_settings_get_pressure_units_dropdown(char* buf);
void ui_settings_get_temperature_units_dropdown(char* buf);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif  //_UI_SETTINGS_H_
