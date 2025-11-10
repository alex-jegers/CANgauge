/**********     INCLUDES        **********/
#include "ui_car_menu.h"
#include "ui_can_sniffer.h"
#include "ui_gauges.h"
/**********		DEFINES		**********/

/**********		EXTERNAL VARIABLE DEFINITIONS		**********/

/**********		STATIC VARIABLES		**********/
static bool ui_car_menu_screen_is_init = false;

/*Event callbacks.*/
void (*ui_car_gauges_btn_clicked_cb)();
void (*ui_car_can_sniffer_btn_clicked_cb)();

/*Panels and screens.*/
static lv_obj_t* ui_car_menu_scr;
static lv_obj_t* ui_car_menu_btn_panel;

/*Buttons.*/
static lv_obj_t* ui_car_gauges_btn;
static lv_obj_t* ui_car_can_sniffer_load_btn;

/**********		STATIC FUNCTION DECLRATIONS		**********/
static void ui_car_init_menu_screen();
static void ui_car_gauges_btn_clicked();
static void ui_car_can_sniffer_btn_clicked();

/**********		STATIC FUNCTION DEFINITIONS		**********/
static void ui_car_init_menu_screen()
{
	/*Init main background screen.*/
	ui_car_menu_scr = lv_obj_create(NULL);
	lv_obj_set_style_bg_color(ui_car_menu_scr, UI_COLOR_BLACK, LV_STATE_DEFAULT);
	ui_car_menu_screen_is_init = true;
	
	/*
	LV_IMAGE_DECLARE(cangauge_logo);
	lv_obj_t* cg_logo_image = lv_image_create(ui_car_menu_scr);
	lv_image_set_src(cg_logo_image, &cangauge_logo);
	lv_obj_align(cg_logo_image, LV_ALIGN_CENTER, 0, -100);
*/
	/*Add a button to launch the code reader.*/
	ui_car_gauges_btn = ui_helpers_create_btn_with_text(ui_car_menu_scr, "Gauges", &lv_font_montserrat_16);
	lv_obj_align(ui_car_gauges_btn, LV_ALIGN_CENTER, 0, 40);
	lv_obj_add_event_cb(ui_car_gauges_btn, ui_car_gauges_btn_clicked, LV_EVENT_PRESSED, NULL);

	/*Add a button to launch the CAN sniffer.*/
	ui_car_can_sniffer_load_btn = ui_helpers_create_btn_with_text(ui_car_menu_scr, "CAN Sniffer", &lv_font_montserrat_16);
	lv_obj_align(ui_car_can_sniffer_load_btn, LV_ALIGN_CENTER, 0, 100);
	lv_obj_add_event_cb(ui_car_can_sniffer_load_btn, ui_car_can_sniffer_btn_clicked, LV_EVENT_PRESSED, NULL);

}

static void ui_car_gauges_btn_clicked()
{
	if (ui_helpers_is_demo_mode() == false)
	{
		if (ui_car_gauges_btn_clicked_cb)
		{
			ui_car_gauges_btn_clicked_cb();
		}
	}
	ui_gauges_load();
}

static void ui_car_can_sniffer_btn_clicked()
{
	if (ui_car_can_sniffer_btn_clicked_cb)
	{
		ui_car_can_sniffer_btn_clicked_cb();
	}
	ui_can_sniffer_load();
}

/**********		GLOBAL FUNCTION DEFINITIONS		**********/
void ui_car_load_menu_screen()
{
	if (ui_car_menu_screen_is_init != true)
	{
		ui_car_init_menu_screen();
	}
	lv_screen_load(ui_car_menu_scr);
	ui_car_menu_screen_is_init = true;
}

void ui_car_set_dtc_reader_load_btn_clicked_cb(void* (func)())
{
	ui_car_gauges_btn_clicked_cb = func;
}

void ui_car_set_can_sniffer_btn_clicked_cb(void* (func)(lv_event_t* e))
{
	ui_car_can_sniffer_btn_clicked_cb = func;
}
