/**********     INCLUDES        **********/
#include "ui_settings.h"
#include "ui_gauges.h"
/**********     TYPEDEFS         **********/

/**********		DEFINES		**********/

/**********		EXTERNAL VARIABLE DEFINITIONS		**********/

/*************************************************/
/**********		STATIC VARIABLES		**********/
/*************************************************/
static bool prv_is_init = false;

/* UI Objects */
static lv_obj_t* prv_settings_screen;					//Parent object.
static lv_obj_t* prv_brightness_slider;					//Brightness slider.
static lv_obj_t* prv_pressure_units_dropdown;			
static lv_obj_t* prv_temperature_units_dropdown;
static lv_obj_t* prv_settings_firmware_update_btn;		//Update firmware button.
static lv_obj_t* prv_settings_data_trsnf_btn;			//Transfer data button.
static lv_obj_t* prv_restore_defaults_btn;				//Restore Defaults button.
static lv_obj_t* prv_data_logging_period_text_area;		//Where users enter the data logging duration.


/* Event Handlers */
static lv_event_cb_t prv_numberpad_closed_cb = NULL;


/**********		STATIC FUNCTION DECLRATIONS		**********/
static void prv_create_brightness_slider();
static void prv_text_area_clicked(lv_event_t* e);
static void prv_number_pad_pressed(lv_event_t* e);

/**********		STATIC FUNCTION DEFINITIONS		**********/
static void prv_create_brightness_slider()
{
	/* Container to hold label and slider. */
	lv_obj_t* container = lv_obj_create(prv_settings_screen);
	lv_obj_set_size(container, 400, 120);
	lv_obj_set_style_bg_color(container, UI_COLOR_GRAY, LV_PART_MAIN);
	lv_obj_set_style_border_color(container, UI_COLOR_DARK_GRAY, LV_PART_MAIN);
	lv_obj_clear_flag(container, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_set_scrollbar_mode(container, LV_SCROLLBAR_MODE_OFF);

	/* Create and style the slider object. */
	prv_brightness_slider = lv_slider_create(container);
	lv_obj_align(prv_brightness_slider, LV_ALIGN_CENTER, 0, 15);
	lv_obj_set_width(prv_brightness_slider, 300);
	lv_obj_set_style_bg_color(prv_brightness_slider, UI_COLOR_RED, LV_PART_MAIN);
	lv_obj_set_style_bg_color(prv_brightness_slider, UI_COLOR_RED, LV_PART_KNOB);
	lv_obj_set_style_bg_color(prv_brightness_slider, UI_COLOR_RED, LV_PART_INDICATOR);

	/* Create and style the label. */
	lv_obj_t* lbl = lv_label_create(container);
	lv_obj_align(lbl, LV_ALIGN_CENTER, 0, -30);
	lv_obj_set_style_text_color(lbl, UI_COLOR_WHITE, LV_STATE_DEFAULT);
	lv_label_set_text(lbl, "Brightness");
}

static void prv_text_area_clicked(lv_event_t* e)
{
	lv_obj_t* txt_area = lv_event_get_target_obj(e);
	lv_obj_t* number_pad = ui_helpers_load_number_pad();
	lv_keyboard_set_textarea(number_pad, txt_area);
	lv_obj_add_event_cb(number_pad, prv_number_pad_pressed, LV_EVENT_VALUE_CHANGED, NULL);
}

static void prv_number_pad_pressed(lv_event_t* e)
{
	lv_obj_t* number_pad = lv_event_get_target_obj(e);
	uint32_t key = lv_keyboard_get_selected_btn(number_pad);
	const char* txt = lv_keyboard_get_btn_text(number_pad, key);
	if (lv_streq(txt, LV_SYMBOL_OK) == true)
	{
		const char* textarea_text = lv_textarea_get_text(prv_data_logging_period_text_area);
		uint32_t datalogger_period_val = strtoul(textarea_text, NULL, 10);
		if (datalogger_period_val < 15)
		{
			lv_textarea_set_text(prv_data_logging_period_text_area, "15");
		}
		if (datalogger_period_val < 60)
		{
			lv_obj_set_style_border_color(prv_data_logging_period_text_area, UI_COLOR_RED, LV_STATE_DEFAULT);
		}
		else
		{
			lv_obj_set_style_border_color(prv_data_logging_period_text_area, UI_COLOR_GRAY, LV_STATE_DEFAULT);
		}
		ui_helpers_delete_number_pad();
		if (prv_numberpad_closed_cb != NULL)
		{
			prv_numberpad_closed_cb(e);
		}

	}
}

/**********		GLOBAL FUNCTION DEFINITIONS		**********/
void ui_settings_load()
{
	lv_scr_load(prv_settings_screen);
}

void ui_settings_init()
{
	if (prv_is_init == true)
	{
		return;
	}
	/* Init the screen. */
	prv_settings_screen = ui_gauges_get_options_container_obj();

	/* Create the settings label. */
	lv_obj_t* settings_lbl = lv_label_create(prv_settings_screen);
	lv_label_set_text(settings_lbl, "Settings");
	lv_obj_set_style_text_color(settings_lbl, UI_COLOR_WHITE, LV_PART_MAIN);
	lv_obj_set_style_text_font(settings_lbl, &lv_font_montserrat_28, LV_PART_MAIN);

	/* Create the brightness slider. */
	prv_create_brightness_slider();

	/* Pressure and temperature units dropdown box and label. */
	lv_obj_t* units_container = lv_obj_create(prv_settings_screen);
	lv_obj_set_size(units_container, 480, 150);
	lv_obj_set_style_bg_opa(units_container, 0, LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(units_container, 0, LV_STATE_DEFAULT);
	lv_obj_set_layout(units_container, LV_LAYOUT_FLEX);
	lv_obj_set_flex_flow(units_container, LV_FLEX_FLOW_ROW_WRAP);
	lv_obj_set_flex_align(units_container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_SPACE_EVENLY);
	lv_obj_set_style_pad_row(units_container, 50, LV_STATE_DEFAULT);
	lv_obj_clear_flag(units_container, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_set_scrollbar_mode(units_container, LV_SCROLLBAR_MODE_OFF);

	lv_obj_t* pressure_units_lbl = lv_label_create(units_container);
	lv_label_set_text_static(pressure_units_lbl, "Pressure Units: ");
	lv_obj_set_style_text_color(pressure_units_lbl, UI_COLOR_WHITE, LV_STATE_DEFAULT);

	prv_pressure_units_dropdown = lv_dropdown_create(units_container);
	lv_dropdown_set_options_static(prv_pressure_units_dropdown, "kPa\nPSI\nbar");
	lv_obj_set_width(prv_pressure_units_dropdown, lv_pct(25));

	lv_obj_t* temp_units_lbl = lv_label_create(units_container);
	lv_label_set_text_static(temp_units_lbl, "Temperature Units: ");
	lv_obj_set_style_text_color(temp_units_lbl, UI_COLOR_WHITE, LV_STATE_DEFAULT);
	prv_temperature_units_dropdown = lv_dropdown_create(units_container);
	lv_dropdown_set_options_static(prv_temperature_units_dropdown, "C\nF");
	lv_obj_set_width(prv_temperature_units_dropdown, lv_pct(25));

	/* Text area for data logging rate. */
	lv_obj_t* text_area_container = lv_obj_create(prv_settings_screen);
	lv_obj_clear_flag(text_area_container, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_set_scrollbar_mode(text_area_container, LV_SCROLLBAR_MODE_OFF);
	lv_obj_set_style_bg_color(text_area_container, UI_COLOR_BLACK, LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(text_area_container, 0, LV_STATE_DEFAULT);
	lv_obj_set_size(text_area_container, lv_pct(100), 105);
	lv_obj_set_style_pad_top(text_area_container, 0, LV_STATE_DEFAULT);
	lv_obj_set_style_pad_bottom(text_area_container, 0, LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(text_area_container, 0, LV_STATE_DEFAULT);
	lv_obj_set_style_pad_right(text_area_container, 0, LV_STATE_DEFAULT);
	prv_data_logging_period_text_area = lv_textarea_create(text_area_container);
	lv_textarea_set_one_line(prv_data_logging_period_text_area, true);
	lv_obj_set_width(prv_data_logging_period_text_area, lv_pct(50));
	lv_obj_align(prv_data_logging_period_text_area, LV_ALIGN_TOP_LEFT, 0, 0);
	lv_obj_set_style_border_width(prv_data_logging_period_text_area, 2, LV_STATE_DEFAULT);
	lv_obj_t* txt_area_lbl = lv_label_create(text_area_container);
	lv_obj_set_style_text_color(txt_area_lbl, UI_COLOR_WHITE, LV_STATE_DEFAULT);
	lv_obj_align(txt_area_lbl, LV_ALIGN_TOP_RIGHT, 0, 0);
	lv_label_set_text(txt_area_lbl, "Data logging period (ms)");
	lv_label_set_long_mode(txt_area_lbl, LV_LABEL_LONG_MODE_WRAP);
	lv_obj_set_size(txt_area_lbl, lv_pct(40), lv_pct(80));
	lv_obj_set_style_text_align(txt_area_lbl, LV_TEXT_ALIGN_LEFT, LV_STATE_DEFAULT);
	lv_obj_add_event_cb(prv_data_logging_period_text_area, prv_text_area_clicked, LV_EVENT_SHORT_CLICKED, NULL);


	/* Make a firmware update button. */
	prv_settings_firmware_update_btn = ui_helpers_create_btn_with_text(prv_settings_screen, "Update Firmware", LV_FONT_DEFAULT);

	/* Data transfer button. */
	prv_settings_data_trsnf_btn = ui_helpers_create_btn_with_text(prv_settings_screen, "Transfer Data", LV_FONT_DEFAULT);

	/* Restore defaults button. */
	prv_restore_defaults_btn = ui_helpers_create_btn_with_text(prv_settings_screen, "Restore Defaults", LV_FONT_DEFAULT);

	prv_is_init = true;
}

void ui_settings_delete()
{
	lv_obj_delete_async(prv_settings_screen);
	prv_is_init = false;
}

void ui_set_brightness_slider_event_cb(lv_event_cb_t func)
{
	lv_obj_add_event_cb(prv_brightness_slider, func, LV_EVENT_VALUE_CHANGED, NULL);
}

void ui_set_settings_scr_load_event_cb(lv_event_cb_t func)
{
	lv_obj_add_event_cb(prv_settings_screen, func, LV_EVENT_SCREEN_LOADED, &prv_brightness_slider);
}


void ui_set_save_settings_cb(lv_event_cb_t func)
{
	lv_obj_add_event_cb(prv_brightness_slider, func, LV_EVENT_RELEASED, NULL);
	lv_obj_add_event_cb(prv_temperature_units_dropdown, func, LV_EVENT_VALUE_CHANGED, NULL);
	lv_obj_add_event_cb(prv_pressure_units_dropdown, func, LV_EVENT_VALUE_CHANGED, NULL);
}

void ui_add_settings_firmware_update_btn_event_cb(lv_event_cb_t func) 
{ 
	lv_obj_add_event_cb(prv_settings_firmware_update_btn, func, LV_EVENT_RELEASED, NULL); 
}

void ui_set_settings_data_trnsf_btn_event_cb(lv_event_cb_t func) 
{ 
	lv_obj_add_event_cb(prv_settings_data_trsnf_btn, func, LV_EVENT_RELEASED, NULL); 
}

void ui_settings_set_restore_defaults_btn_event_cb(lv_event_cb_t func)
{
	lv_obj_add_event_cb(prv_restore_defaults_btn, func, LV_EVENT_RELEASED, NULL);
}

void ui_set_numberpad_closed_cb(lv_event_cb_t func)
{
	prv_numberpad_closed_cb = func;
}

bool ui_settings_set_pressure_units_dropdown(char* units_str)
{
	int8_t option_index = lv_dropdown_get_option_index(prv_pressure_units_dropdown, units_str);
	if (option_index == -1)
	{
		return false;
	}
	lv_dropdown_set_selected(prv_pressure_units_dropdown, option_index);
	return true;
}

bool ui_settings_set_temperature_units_dropdown(char* units_str)
{
	int8_t option_index = lv_dropdown_get_option_index(prv_temperature_units_dropdown, units_str);
	if (option_index == -1)
	{
		return false;
	}
	lv_dropdown_set_selected(prv_temperature_units_dropdown, option_index);
	return true;
}

void ui_settings_set_brightness_slider_value(uint32_t val)
{
	lv_slider_set_value(prv_brightness_slider, val, LV_ANIM_OFF);
}

void ui_settings_get_pressure_units_dropdown(char* buf)
{
	lv_dropdown_get_selected_str(prv_pressure_units_dropdown, buf, 0);

}

void ui_settings_get_temperature_units_dropdown(char* buf)
{
	lv_dropdown_get_selected_str(prv_temperature_units_dropdown, buf, 0);
}

uint32_t ui_settings_get_brightness_slider_value()
{
	uint32_t rtn_val = lv_slider_get_value(prv_brightness_slider);
	return rtn_val;
}

uint32_t ui_settings_get_data_logger_rate()
{
	const char* textarea_text = lv_textarea_get_text(prv_data_logging_period_text_area);
	uint32_t datalogger_period_val = strtoul(textarea_text, NULL, 10);
	return datalogger_period_val;
}

void ui_settings_set_data_logger_rate(uint32_t rate)
{
	char rate_str[10];
	sprintf(rate_str, "%ul", rate);
	lv_textarea_set_text(prv_data_logging_period_text_area, (const char*)rate_str);

}
