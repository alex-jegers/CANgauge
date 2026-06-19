/**********     INCLUDES        **********/
#include "ui_settings.h"
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
static lv_obj_t* prv_settings_firmware_update_btn;		//Update firmware button.
static lv_obj_t* prv_settings_data_trsnf_btn;			//Transfer data button.
static lv_obj_t* prv_settings_back_btn;					//Back button.


/* Event Handlers */
static lv_event_cb_t prv_settings_back_btn_event_cb = NULL;




/**********		STATIC FUNCTION DECLRATIONS		**********/
static void prv_settings_btn_handler(lv_event_t* e);

static void prv_create_brightness_slider();
static void prv_settings_back_btn_event(lv_event_t* e);
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

static void prv_settings_back_btn_event(lv_event_t* e)
{
	ui_gauges_load();
	if (prv_settings_back_btn_event_cb != NULL)
	{
		prv_settings_back_btn_event_cb(e);
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
	prv_settings_screen = lv_obj_create(NULL);
	lv_obj_set_style_bg_color(prv_settings_screen, UI_COLOR_BLACK, LV_STATE_DEFAULT);

	/* Format the flex flow. */
	lv_obj_set_layout(prv_settings_screen, LV_LAYOUT_FLEX);
	lv_obj_set_flex_flow(prv_settings_screen, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_flex_align(prv_settings_screen, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_SPACE_EVENLY);
	lv_obj_set_style_pad_top(prv_settings_screen, 50, LV_PART_MAIN);
	lv_obj_set_style_pad_row(prv_settings_screen, 30, LV_STATE_DEFAULT);

	/* Create the settings label. */
	lv_obj_t* settings_lbl = lv_label_create(prv_settings_screen);
	lv_label_set_text(settings_lbl, "Settings");
	lv_obj_set_style_text_color(settings_lbl, UI_COLOR_WHITE, LV_PART_MAIN);
	lv_obj_set_style_text_font(settings_lbl, &lv_font_montserrat_28, LV_PART_MAIN);

	/* Create the brightness slider. */
	prv_create_brightness_slider();

	/* Create the demo mode checkbox. */
	lv_obj_t* demo_mode_checkbox = lv_checkbox_create(prv_settings_screen);
	lv_checkbox_set_text(demo_mode_checkbox, "Demo Mode");
	lv_obj_set_style_text_color(demo_mode_checkbox, UI_COLOR_WHITE, LV_STATE_DEFAULT);

	/* Make a firmware update button. */
	prv_settings_firmware_update_btn = ui_helpers_create_btn_with_text(prv_settings_screen, "Update Firmware", LV_FONT_DEFAULT);

	/* Data transfer button. */
	prv_settings_data_trsnf_btn = ui_helpers_create_btn_with_text(prv_settings_screen, "Transfer Data", LV_FONT_DEFAULT);

	/* Make a button to go back. */
	prv_settings_back_btn = ui_helpers_create_btn_with_text(prv_settings_screen, "Back", LV_FONT_DEFAULT);
	lv_obj_add_event_cb(prv_settings_back_btn, prv_settings_back_btn_event, LV_EVENT_RELEASED, NULL);

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


void ui_set_settings_back_btn_event_cb(lv_event_cb_t func)
{
	prv_settings_back_btn_event_cb = func;
}

void ui_add_settings_firmware_update_btn_event_cb(lv_event_cb_t func) 
{ 
	lv_obj_add_event_cb(prv_settings_firmware_update_btn, func, LV_EVENT_RELEASED, NULL); 
}

void ui_set_settings_data_trnsf_btn_event_cb(lv_event_cb_t func) 
{ 
	lv_obj_add_event_cb(prv_settings_data_trsnf_btn, func, LV_EVENT_RELEASED, NULL); 
}
