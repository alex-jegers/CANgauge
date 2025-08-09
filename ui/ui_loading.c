/*
 * ui_loading.c
 *
 *  Created on: May 25, 2024
 *      Author: awjpp
 */

#ifdef CORE_CM7

#include "ui_gauges.h"						//to be able to load the gauge screen.
#include "ui_loading.h"
#include "components/ui_vidagauge_logo.h"


/**********		STATIC VARIABLES		**********/
static void (*ui_loading_error_retry_btn_clicked_cb)(lv_event_t* e) = NULL;
static void (*ui_loading_error_demo_btn_clicked_cb)(lv_event_t* e) = NULL;
static bool loading_screen_initialized = false;
static bool loading_error_screen_initialized = false;

/**********     GLOBAL VARIABLE DEFINITIONS     **********/
lv_obj_t* ui_loading_screen = NULL;
lv_obj_t* ui_loading_error_screen;
lv_obj_t* ui_loading_error_screen_retry_btn;
lv_obj_t* ui_loading_error_screen_demo_btn;

/**********     STATIC FUNCTION DECLARATIONS     **********/
static void ui_loading_screen_spinner_anim_cb(void* indic, int32_t val);
static void ui_loading_error_retry_btn_clicked_handler(lv_event_t* e);
static void ui_loading_error_demo_btn_clicked_handler(lv_event_t* e);

/**********     STATIC FUNCTION DEFINITIONS     **********/
static void ui_loading_screen_spinner_anim_cb(void* indic, int32_t val)
{
	const uint32_t angle = 105;

	lv_arc_set_start_angle(indic, val);
	lv_arc_set_end_angle(indic, val + angle);
}

static void ui_loading_error_retry_btn_clicked_handler(lv_event_t* e)
{
	if (ui_loading_error_retry_btn_clicked_cb != NULL)
	{
		ui_loading_error_retry_btn_clicked_cb(e);
	}

#if WINDOWS == 1
		ui_load_loading_screen();
#endif //WINDOWS == 1
}

static void ui_loading_error_demo_btn_clicked_handler(lv_event_t* e)
{
	if (ui_loading_error_demo_btn_clicked_cb != NULL)
	{
		ui_loading_error_demo_btn_clicked_cb(e);
	}

#if WINDOWS == 1
	ui_init_gauge_screen();
	ui_load_gauge_screen();
#endif //WINDOWS == 1
}

/**********     GLOBAL FUNCTION DEFINITIONS     **********/
void ui_load_loading_screen()
{
	if (loading_screen_initialized == false)
	{
		ui_init_loading_screen();
		loading_screen_initialized = true;
	}
	lv_scr_load(ui_loading_screen);
}

void ui_init_loading_screen()
{
	/*Initialze the screen.*/
	ui_loading_screen = lv_obj_create(NULL);
	lv_obj_set_size(ui_loading_screen, UI_SIZE_X, UI_SIZE_Y);
	lv_obj_set_style_bg_color(ui_loading_screen, UI_COLOR_BLACK, LV_STATE_DEFAULT);
	
	/*Initialize the loading spinner.*/
	lv_obj_t* spinner = lv_arc_create(ui_loading_screen);
	lv_obj_set_size(spinner, 50, 50);
	lv_obj_set_style_arc_color(spinner, UI_COLOR_BLACK, LV_STATE_DEFAULT);
	lv_obj_set_style_arc_color(spinner, UI_COLOR_RED, LV_STATE_DEFAULT | LV_PART_INDICATOR);
	lv_obj_set_style_arc_width(spinner, 5, LV_STATE_DEFAULT | LV_PART_INDICATOR);
	lv_obj_set_style_bg_opa(spinner, 0, LV_STATE_DEFAULT | LV_PART_KNOB);
	lv_obj_center(spinner);
	lv_arc_set_end_angle(spinner, 180);
	lv_arc_set_start_angle(spinner, 90);
	lv_obj_clear_flag(spinner, LV_OBJ_FLAG_CLICKABLE);

	/*Initialze the loading spinner animation.*/
	static lv_anim_t a;
	lv_anim_init(&a);
	lv_anim_set_var(&a, spinner);
	lv_anim_set_exec_cb(&a, ui_loading_screen_spinner_anim_cb);
	lv_anim_set_values(&a, 0, 360);
	lv_anim_set_time(&a, 2000);
	lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
	lv_anim_start(&a);

	/*Create the logo.*/
	lv_obj_t* logo = lv_img_create(ui_loading_screen);
	lv_img_set_src(logo, &VIDAgauge_logo);
	lv_obj_align(logo, LV_ALIGN_CENTER, 0, -60);
}

void ui_delete_loading_screen()
{
	lv_obj_del(ui_loading_screen);
}

void ui_delete_loading_error_screen()
{
	lv_obj_del(ui_loading_error_screen);
}

void ui_init_loading_error_screen()
{
	/*Initialze the screen.*/
	ui_loading_error_screen = lv_obj_create(NULL);
	lv_obj_set_size(ui_loading_error_screen, 300, 300);
	lv_obj_set_style_bg_color(ui_loading_error_screen, UI_COLOR_BLACK, LV_STATE_DEFAULT);

	lv_obj_t* no_can_lbl = lv_label_create(ui_loading_error_screen);
	lv_obj_align(no_can_lbl, LV_ALIGN_CENTER, 0, -40);
	lv_label_set_text(no_can_lbl, "CANbus error");
	lv_obj_set_style_text_color(no_can_lbl, UI_COLOR_WHITE, LV_STATE_DEFAULT);

	ui_loading_error_screen_retry_btn = lv_btn_create(ui_loading_error_screen);
	lv_obj_t* retry_btn_lbl = lv_label_create(ui_loading_error_screen_retry_btn);
	lv_label_set_text(retry_btn_lbl, "Retry");
	lv_obj_align(ui_loading_error_screen_retry_btn, LV_ALIGN_CENTER, 0, 0);

	ui_loading_error_screen_demo_btn = lv_btn_create(ui_loading_error_screen);
	lv_obj_t* demo_btn_lbl = lv_label_create(ui_loading_error_screen_demo_btn);
	lv_label_set_text(demo_btn_lbl, "Enter Demo Mode");
	lv_obj_align(ui_loading_error_screen_demo_btn, LV_ALIGN_CENTER, 0, 40);

	ui_assign_button_style(ui_loading_error_screen_retry_btn);
	ui_assign_button_style(ui_loading_error_screen_demo_btn);

	lv_obj_add_event_cb(ui_loading_error_screen_demo_btn, ui_loading_error_demo_btn_clicked_handler, LV_EVENT_CLICKED, NULL);
	lv_obj_add_event_cb(ui_loading_error_screen_retry_btn, ui_loading_error_retry_btn_clicked_handler, LV_EVENT_CLICKED, NULL);
}

void ui_load_loading_error_screen()
{
	if (loading_error_screen_initialized == false)
	{
		ui_init_loading_error_screen();
		loading_error_screen_initialized = true;
	}
	lv_scr_load(ui_loading_error_screen);
}

void ui_assign_demo_btn_cb(void (*func)(lv_event_t*))
{
	ui_loading_error_demo_btn_clicked_cb = func;
}
void ui_assign_retry_btn_cb(void (*func)(lv_event_t*))
{
	ui_loading_error_retry_btn_clicked_cb = func;
}


#endif //CORE_CM7
