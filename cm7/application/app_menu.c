/**********     INCLUDES        **********/
#include "app_menu.h"
#include "lvgl/lvgl.h"
#include "ui/ui_car_menu.h"

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

static void prv_brightness_slider_handler(lv_event_t* e)
{
    lv_obj_t* obj = lv_event_get_target_obj(e);
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_VALUE_CHANGED)
    {
        uint8_t slider_val = lv_slider_get_value(obj);      //Returns a value between 0 and 100.
        uint32_t timer_val = (605 * slider_val) + 5000;		//Map the slider value of 0 to 100 to 5000 to 65535.
        timer_set_pwm_duty_cycle(TIM12, timer_val, 1);
    }

}

static void prv_menu_scr_load_handler(lv_event_t* e)
{
	lv_event_code_t code = lv_event_get_code(e);

	if (code == LV_EVENT_SCREEN_LOADED)
	{
			lv_obj_t** slider = lv_event_get_user_data(e);
			uint32_t timer_val = timer_get_pwm_duty_cycle(TIM12, 1);
			uint32_t slider_val = (timer_val - 5000) / 605;
			lv_slider_set_value(*slider, slider_val, LV_ANIM_OFF);
	}

}

/**********		GLOBAL FUNCTION DEFINITIONS		**********/
void app_menu_run()
{
	ui_menu_load();
	ui_menu_set_can_sniffer_btn_clicked_cb(prv_can_sniffer_btn_hanlder);
	ui_menu_set_gauges_load_btn_clicked_cb(prv_gauges_btn_handler);
    ui_menu_set_slider_event_cb(prv_brightness_slider_handler);
    ui_menu_set_settings_scr_load_event_cb(prv_menu_scr_load_handler);
}
