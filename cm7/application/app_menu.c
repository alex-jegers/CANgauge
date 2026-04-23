/**********     INCLUDES        **********/
#include "app_menu.h"
#include "lvgl/lvgl.h"

#include "drivers/drivers.h"

/**********		DEFINES		**********/

/**********		EXTERNAL VARIABLE DEFINITIONS		**********/

/**********		STATIC VARIABLES		**********/

/**********		STATIC FUNCTION DECLRATIONS		**********/
static void prv_can_sniffer_btn_hanlder(lv_event_t* e);
static void prv_gauges_btn_handler(lv_event_t* e);
static void prv_brightness_slider_handler(lv_event_t* e);
static void prv_menu_scr_load_handler(lv_event_t* e);

/**********		STATIC FUNCTION DEFINITIONS		**********/
static void prv_can_sniffer_btn_hanlder(lv_event_t* e)
{
	//xTaskCreate(app_can_sniffer_cm7, "CAN_SNIFFER", 500, NULL, 0, NULL);
}


static void prv_gauges_btn_handler(lv_event_t* e)
{
	app_gauges_run();
}



/**********		GLOBAL FUNCTION DEFINITIONS		**********/
void app_menu_run()
{
	ui_menu_load();
	ui_menu_set_can_sniffer_btn_clicked_cb(prv_can_sniffer_btn_hanlder);
	ui_menu_set_gauges_load_btn_clicked_cb(prv_gauges_btn_handler);

}
