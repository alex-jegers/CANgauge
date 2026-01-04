/**********     INCLUDES        **********/
#include "ui_car_menu.h"
#include "ui_can_sniffer.h"
#include "ui_gauges.h"
/**********		DEFINES		**********/
#define LOGO_Y_POS			-100
#define BTNS_Y_POS			100

/**********		EXTERNAL VARIABLE DEFINITIONS		**********/

/**********		STATIC VARIABLES		**********/
static bool prv_is_init = false;

/*Event callbacks.*/
lv_event_cb_t ui_car_gauges_btn_clicked_cb;
lv_event_cb_t ui_car_can_sniffer_btn_clicked_cb;
lv_event_cb_t ui_car_dtc_btn_clicked_cb;
lv_event_cb_t ui_car_data_logger_btn_clicked_cb;

/*Panels and screens.*/
static lv_obj_t* prv_main_scr;
static lv_obj_t* prv_other_scr;
static lv_obj_t* prv_main_btn_container;
static lv_obj_t* prv_other_btn_container;

/*Buttons.*/
static lv_obj_t* ui_car_gauges_btn;
static lv_obj_t* ui_car_dtc_btn;
static lv_obj_t* ui_car_data_logger_btn;
static lv_obj_t* ui_car_can_sniffer_load_btn;
static lv_obj_t* prv_other_btn;

/**********		STATIC FUNCTION DECLRATIONS		**********/
static void prv_init_menu_screen();
static void prv_init_other_screen();
static void ui_car_gauges_btn_clicked(lv_event_t* e );
static void ui_car_can_sniffer_btn_clicked(lv_event_t* e);
static void ui_car_dtc_btn_clicked(lv_event_t* e);
static void ui_car_data_logger_btn_clicked(lv_event_t* e);
static void prv_other_btn_clicked(lv_event_t* e);

/**********		STATIC FUNCTION DEFINITIONS		**********/
static void prv_init_menu_screen()
{
	/*Init main background screen.*/
	prv_main_scr = lv_obj_create(NULL);
	lv_obj_set_style_bg_color(prv_main_scr, UI_COLOR_BLACK, LV_STATE_DEFAULT);
	lv_obj_set_scrollbar_mode(prv_main_scr, LV_SCROLLBAR_MODE_OFF);
	
	/* Add the CANgauge logo.*/
	LV_IMAGE_DECLARE(cangauge_logo);
	lv_obj_t* cg_logo_image = lv_image_create(prv_main_scr);
	lv_image_set_src(cg_logo_image, &cangauge_logo);
	lv_obj_align(cg_logo_image, LV_ALIGN_CENTER, 0, LOGO_Y_POS);

	/* Create a container for the buttons. */
	prv_main_btn_container = lv_obj_create(prv_main_scr);
	lv_obj_align(prv_main_btn_container, LV_ALIGN_CENTER, 0, BTNS_Y_POS);
	lv_obj_set_size(prv_main_btn_container, 300, 220);
	lv_obj_set_style_bg_color(prv_main_btn_container, UI_COLOR_BLACK, LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(prv_main_btn_container, 0, LV_STATE_DEFAULT);
	lv_obj_set_layout(prv_main_btn_container, LV_LAYOUT_FLEX);
	lv_obj_set_flex_flow(prv_main_btn_container, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_flex_align(prv_main_btn_container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_SPACE_EVENLY);


	/*Add a button to launch the code reader.*/
	ui_car_gauges_btn = ui_helpers_create_btn_with_text(prv_main_btn_container, "Gauges", UI_BTN_FONT);
	lv_obj_add_event_cb(ui_car_gauges_btn, ui_car_gauges_btn_clicked, LV_EVENT_PRESSED, NULL);

	/*Add a button for the DTC reader.*/
	ui_car_dtc_btn = ui_helpers_create_btn_with_text(prv_main_btn_container, "DTC Reader", UI_BTN_FONT);
	lv_obj_add_event_cb(ui_car_dtc_btn, ui_car_dtc_btn_clicked, LV_EVENT_PRESSED, NULL);


	/*Add a button for the data logger.*/
	ui_car_data_logger_btn = ui_helpers_create_btn_with_text(prv_main_btn_container, "Data Logger", UI_BTN_FONT);
	lv_obj_add_event_cb(ui_car_data_logger_btn, ui_car_data_logger_btn_clicked, LV_EVENT_PRESSED, NULL);

	/*Add a button to go to a settings/other options screen.*/
	prv_other_btn = ui_helpers_create_btn_with_text(prv_main_btn_container, "Other", UI_BTN_FONT);
}

static void prv_init_other_screen()
{
	/* Init the screen. */
	prv_other_scr = lv_obj_create(NULL);
	lv_obj_set_style_bg_color(prv_other_scr, UI_COLOR_BLACK, LV_STATE_DEFAULT);

	/* Create a container for the buttons. */
	prv_other_btn_container = lv_obj_create(prv_other_scr);
	lv_obj_align(prv_other_scr, LV_ALIGN_CENTER, 0, BTNS_Y_POS);
	lv_obj_set_size(prv_other_scr, 300, 220);
	lv_obj_set_style_bg_color(prv_other_scr, UI_COLOR_BLACK, LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(prv_other_scr, 0, LV_STATE_DEFAULT);
	lv_obj_set_layout(prv_other_scr, LV_LAYOUT_FLEX);
	lv_obj_set_flex_flow(prv_other_scr, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_flex_align(prv_other_scr, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_SPACE_EVENLY);

	/*Add a button to launch the CAN sniffer.*/
	ui_car_can_sniffer_load_btn = ui_helpers_create_btn_with_text(prv_other_btn_container, "CAN Sniffer", UI_BTN_FONT);
	lv_obj_add_event_cb(ui_car_can_sniffer_load_btn, ui_car_can_sniffer_btn_clicked, LV_EVENT_PRESSED, NULL);
}

static void ui_car_gauges_btn_clicked(lv_event_t* e)
{
	if (ui_car_gauges_btn_clicked_cb)
	{
		ui_car_gauges_btn_clicked_cb(e);
	}
	ui_gauges_load();
}

static void ui_car_can_sniffer_btn_clicked(lv_event_t* e)
{
	if (ui_car_can_sniffer_btn_clicked_cb)
	{
		ui_car_can_sniffer_btn_clicked_cb(e);
	}
	ui_can_sniffer_load();
}

static void ui_car_dtc_btn_clicked(lv_event_t* e)
{
	if (ui_car_dtc_btn_clicked_cb)
	{
		ui_car_dtc_btn_clicked_cb(e);
	}
	//TODO: load the DTC screen.
}

static void ui_car_data_logger_btn_clicked(lv_event_t* e)
{
	if (ui_car_data_logger_btn_clicked_cb)
	{
		ui_car_data_logger_btn_clicked_cb(e);
	}
	//TODO: load data logger screen.
}

static void prv_other_btn_clicked(lv_event_t* e)
{

}


/**********		GLOBAL FUNCTION DEFINITIONS		**********/
void ui_car_load_menu_screen()
{
	if (prv_is_init != true)
	{
		prv_init_menu_screen();
		prv_init_other_screen();
	}
	lv_screen_load(prv_main_scr);
	prv_is_init = true;
}

void ui_car_set_gauges_load_btn_clicked_cb(lv_event_cb_t func)
{
	ui_car_gauges_btn_clicked_cb = func;
}

void ui_car_set_can_sniffer_btn_clicked_cb(lv_event_cb_t func)
{
	ui_car_can_sniffer_btn_clicked_cb = func;
}

void ui_car_set_dtc_btn_clicked_cb(lv_event_cb_t func)
{
	ui_car_dtc_btn_clicked_cb = func;
}

void ui_car_set_data_logger_btn_clicked_cb(lv_event_cb_t func)
{
	ui_car_data_logger_btn_clicked_cb = func;
}


