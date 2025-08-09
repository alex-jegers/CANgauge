/*
 * ui_dtc_reader.c
 *
 *  Created on: April 10, 2025
 *      Author: Alex Jegers
 */


#include "ui_dtc_reader.h"
#include "ui/ui_helpers.h"
#include "ui/ui_gauges.h"
#include "lvgl/lvgl.h"
#include "ui/ui_dialog_box.h"

 /**********		STATIC VARIABLES		**********/
static lv_obj_t* ui_dtc_reader_screen;
static lv_obj_t* dtc_reader_table;
static lv_obj_t* dtc_reader_read_btn;
static lv_obj_t* dtc_reader_clear_btn;
static lv_obj_t* dtc_reader_back_btn;
static lv_msgbox_t* dtc_reader_msgbox;
void (*dtc_reader_read_btn_clicked_cb)(lv_event_t* e) = NULL;
void (*dtc_reader_clear_btn_clicked_cb)(lv_event_t* e) = NULL;
void (*dtc_reader_back_btn_clicked_cb)(lv_event_t* e) = NULL;
static bool screen_initialized = false;

/**********         STATIC FUNCTION DECLARATIONS         **********/
static void dtc_reader_read_btn_clicked(lv_event_t* event_sender);
static void dtc_reader_clear_btn_clicked(lv_event_t* event_sender);
static void dtc_reader_back_btn_clicked(lv_event_t* event_sender);
static void dtc_reader_table_draw_event(lv_event_t* event_sender);
static void dtc_reader_demo_mode_close_diaglog_box(lv_timer_t* timer);

/**********         STATIC FUNCTION DEFINITIONS         **********/
static void dtc_reader_read_btn_clicked(lv_event_t* event_sender)
{
    if (ui_demo_mode)
    {
        ui_dialog_box_run("Reading BCM...");

        /*If were in demo mode start a timer to close the message box automatically.*/
        lv_timer_t* demo_timer = lv_timer_create(dtc_reader_demo_mode_close_diaglog_box, 3000, NULL);
        //lv_timer_ready(demo_timer);

        lv_table_set_cell_value(dtc_reader_table, 1, 0, "BCM");
        lv_table_set_cell_value(dtc_reader_table, 2, 0, "BCM");

        lv_table_set_cell_value(dtc_reader_table, 1, 1, "0022");
        lv_table_set_cell_value(dtc_reader_table, 2, 1, "0112");
    }

    if (dtc_reader_read_btn_clicked_cb != NULL)
    {
        dtc_reader_read_btn_clicked_cb(event_sender);
    }
}

static void dtc_reader_clear_btn_clicked(lv_event_t* event_sender)
{
    if (ui_demo_mode)
    {
        lv_table_set_cell_value(dtc_reader_table, 1, 0, "");
        lv_table_set_cell_value(dtc_reader_table, 2, 0, "");
        lv_table_set_cell_value(dtc_reader_table, 3, 0, "");
        lv_table_set_cell_value(dtc_reader_table, 4, 0, "");
        lv_table_set_cell_value(dtc_reader_table, 5, 0, "");

        lv_table_set_cell_value(dtc_reader_table, 1, 1, "");
        lv_table_set_cell_value(dtc_reader_table, 2, 1, "");
        lv_table_set_cell_value(dtc_reader_table, 3, 1, "");
        lv_table_set_cell_value(dtc_reader_table, 4, 1, "");
        lv_table_set_cell_value(dtc_reader_table, 5, 1, "");
    }

    if (dtc_reader_clear_btn_clicked_cb != NULL)
    {
        dtc_reader_clear_btn_clicked_cb(event_sender);
    }
}

static void dtc_reader_back_btn_clicked(lv_event_t* event_sender)
{
    if (ui_demo_mode)
    {
        ui_load_gauge_screen();
        return;
    }

    if (dtc_reader_back_btn_clicked_cb != NULL)
    {
        dtc_reader_back_btn_clicked_cb(event_sender);
    }
}

static void dtc_reader_table_draw_event(lv_event_t* event_sender)
{
    lv_obj_t* obj = lv_event_get_target(event_sender);
    lv_obj_draw_part_dsc_t* dsc = lv_event_get_draw_part_dsc(event_sender);

    /*If the background of the table is being drawn.*/
    if (dsc->part == LV_PART_MAIN)
    {
        dsc->rect_dsc->bg_color = UI_COLOR_BLACK;
        dsc->rect_dsc->border_color = UI_COLOR_BLACK;
    }

    /*If the cells are being drawn...*/
    if (dsc->part == LV_PART_ITEMS) {
        uint32_t row = dsc->id / lv_table_get_col_cnt(obj);
        uint32_t col = dsc->id - row * lv_table_get_col_cnt(obj);

        /*Make all the text white.*/
        dsc->label_dsc->color = UI_COLOR_WHITE;

        /*Make the texts in the first cell center aligned, bg black, and text white.*/
        if (row == 0) {
            dsc->label_dsc->align = LV_TEXT_ALIGN_CENTER;
            dsc->rect_dsc->bg_color = UI_COLOR_BLACK;
            dsc->rect_dsc->bg_opa = LV_OPA_COVER;
            dsc->rect_dsc->border_color = UI_COLOR_DARK_GRAY;
            dsc->rect_dsc->border_width = 2;
        }
        /*In the first column align the texts to the right*/
        else if (col == 0) {
            dsc->label_dsc->align = LV_TEXT_ALIGN_RIGHT;
        }

        /*MAke every 2nd row gray & every other row black.*/
        if ((row != 0 && row % 2) == 0) {
            dsc->rect_dsc->bg_color = UI_COLOR_BLACK;
            dsc->rect_dsc->border_color = UI_COLOR_GRAY;
            dsc->rect_dsc->border_width = 2;
        }
        else
        {
            dsc->rect_dsc->bg_color = UI_COLOR_DARK_GRAY;
            dsc->rect_dsc->border_color = UI_COLOR_DARK_GRAY;
            dsc->rect_dsc->border_width = 2;
        }
    }
}

static void dtc_reader_demo_mode_close_diaglog_box(lv_timer_t* timer)
{
    static uint8_t counter = 0;
    switch (counter)
    {
    case 0:
        ui_dialog_box_change_text("Reading CEM...");
        lv_table_set_cell_value(dtc_reader_table, 3, 0, "CEM");
        lv_table_set_cell_value(dtc_reader_table, 4, 0, "CEM");
        lv_table_set_cell_value(dtc_reader_table, 3, 1, "4F47");
        lv_table_set_cell_value(dtc_reader_table, 4, 1, "4F48");
        break;
    case 1:
        ui_dialog_box_change_text("Reading DEM...");
        lv_table_set_cell_value(dtc_reader_table, 5, 0, "DEM");
        lv_table_set_cell_value(dtc_reader_table, 5, 1, "0007");
        break;
    case 2:
        ui_dialog_box_close();
        lv_timer_del(timer);
        counter = 0;
        return;
    }
    counter++;
}

/**********         GLOBAL FUNCTION DEFINITIONS         **********/
void ui_init_dtc_reader()
{
    ui_dtc_reader_screen = lv_obj_create(NULL);
    lv_obj_set_size(ui_dtc_reader_screen, 300, 300);
    lv_obj_set_style_bg_color(ui_dtc_reader_screen, UI_COLOR_BLACK, LV_STATE_DEFAULT);

    /*Create the table.*/
    dtc_reader_table = lv_table_create(ui_dtc_reader_screen);
    lv_table_set_cell_value(dtc_reader_table, 0, 0, "Module");
    lv_table_set_cell_value(dtc_reader_table, 1, 0, "");
    lv_table_set_cell_value(dtc_reader_table, 2, 0, "");
    lv_table_set_cell_value(dtc_reader_table, 3, 0, "");
    lv_table_set_cell_value(dtc_reader_table, 4, 0, "");
    lv_table_set_cell_value(dtc_reader_table, 5, 0, "");


    lv_table_set_cell_value(dtc_reader_table, 0, 1, "Code");
    lv_table_set_cell_value(dtc_reader_table, 1, 1, "");
    lv_table_set_cell_value(dtc_reader_table, 2, 1, "");
    lv_table_set_cell_value(dtc_reader_table, 3, 1, "");
    lv_table_set_cell_value(dtc_reader_table, 4, 1, "");
    lv_table_set_cell_value(dtc_reader_table, 5, 1, "");

    lv_obj_set_style_bg_color(dtc_reader_table, UI_COLOR_BLACK, LV_STATE_ANY);

    /*Add an event callback to to apply some custom drawing*/
    lv_obj_add_event_cb(dtc_reader_table, dtc_reader_table_draw_event, LV_EVENT_DRAW_PART_BEGIN, NULL);

    /*Set a smaller height to the table. It'll make it scrollable*/
    lv_obj_set_height(dtc_reader_table, 200);
    lv_obj_center(dtc_reader_table);

    /*Create a button to read codes.*/
    dtc_reader_read_btn = lv_btn_create(ui_dtc_reader_screen);
    lv_obj_t* dtc_reader_read_btn_lbl = lv_label_create(dtc_reader_read_btn);
    lv_label_set_text(dtc_reader_read_btn_lbl, "Read");
    lv_obj_align(dtc_reader_read_btn_lbl, LV_ALIGN_CENTER, 0, 0);
    lv_obj_align(dtc_reader_read_btn, LV_ALIGN_CENTER, -100, 125);
    lv_obj_add_flag(dtc_reader_read_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_color(dtc_reader_read_btn, UI_COLOR_RED, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(dtc_reader_read_btn, UI_COLOR_GRAY, LV_STATE_PRESSED | LV_STATE_CHECKED | LV_STATE_FOCUSED);
    lv_obj_set_style_shadow_width(dtc_reader_read_btn, 0, LV_STATE_DEFAULT);
    lv_obj_add_event_cb(dtc_reader_read_btn, dtc_reader_read_btn_clicked, LV_EVENT_PRESSED, NULL); 

    /*Create a button to clear codes.*/
    dtc_reader_clear_btn = lv_btn_create(ui_dtc_reader_screen);
    lv_obj_t* dtc_reader_clear_btn_lbl = lv_label_create(dtc_reader_clear_btn);
    lv_label_set_text(dtc_reader_clear_btn_lbl, "Clear");
    lv_obj_align(dtc_reader_clear_btn_lbl, LV_ALIGN_CENTER, 0, 0);
    lv_obj_align(dtc_reader_clear_btn, LV_ALIGN_CENTER, 0, 125);
    lv_obj_add_flag(dtc_reader_clear_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_color(dtc_reader_clear_btn, UI_COLOR_RED, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(dtc_reader_clear_btn, UI_COLOR_GRAY, LV_STATE_PRESSED | LV_STATE_CHECKED | LV_STATE_FOCUSED);
    lv_obj_set_style_shadow_width(dtc_reader_clear_btn, 0, LV_STATE_DEFAULT);
    lv_obj_add_event_cb(dtc_reader_clear_btn, dtc_reader_clear_btn_clicked, LV_EVENT_PRESSED, NULL);

    /*Create a button to return to gauges.*/
    dtc_reader_back_btn = lv_btn_create(ui_dtc_reader_screen);
    lv_obj_t* dtc_reader_back_btn_lbl = lv_label_create(dtc_reader_back_btn);
    lv_label_set_text(dtc_reader_back_btn_lbl, "Back");
    lv_obj_align(dtc_reader_back_btn_lbl, LV_ALIGN_CENTER, 0, 0);
    lv_obj_align(dtc_reader_back_btn, LV_ALIGN_CENTER, 100, 125);
    lv_obj_add_flag(dtc_reader_back_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_color(dtc_reader_back_btn, UI_COLOR_RED, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(dtc_reader_back_btn, UI_COLOR_GRAY, LV_STATE_PRESSED | LV_STATE_CHECKED | LV_STATE_FOCUSED);
    lv_obj_set_style_shadow_width(dtc_reader_back_btn, 0, LV_STATE_DEFAULT);
    lv_obj_add_event_cb(dtc_reader_back_btn, dtc_reader_back_btn_clicked, LV_EVENT_PRESSED, NULL);
}

void ui_load_dtc_reader()
{
	if (screen_initialized == false)
	{
		ui_init_dtc_reader();
		screen_initialized = true;
	}
    lv_disp_load_scr(ui_dtc_reader_screen);
}

void ui_dtc_reader_assign_back_btn_clicked_handler(void (*func)(lv_event_t* e))
{
	dtc_reader_back_btn_clicked_cb = func;
}

void ui_dtc_reader_assign_read_btn_clicked_handler(void (*func)(lv_event_t* e))
{

}

void ui_dtc_reader_assign_clear_btn_clicked_handler(void (*func)(lv_event_t* e))
{

}

