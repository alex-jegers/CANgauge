/**********     INCLUDES        **********/
#include "ui_test.h"
#include "lvgl/lvgl.h"
#include "ui_helpers.h"

/**********     DEFINES         **********/

/**********		EXTERNAL VARIABLE DEFINITIONS		**********/

/**********		STATIC VARIABLES		**********/
static lv_obj_t* test_scr;
static lv_obj_t* test_btn;
static bool screen_initialized = false;

/**********		STATIC FUNCTION DECLRATIONS		**********/

/**********		STATIC FUNCTION DEFINITIONS		**********/

/**********		GLOBAL FUNCTION DEFINITIONS		**********/
void ui_init_test_screen()
{
    test_scr = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(test_scr, UI_COLOR_RED, LV_STATE_DEFAULT);

    test_btn = lv_btn_create(test_scr);
    lv_obj_align(test_btn, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_size(test_btn, 339, 339);
}

void ui_load_test_screen()
{
    if (screen_initialized == false)
    {
        ui_init_test_screen();
        screen_initialized = true;
    }
    lv_disp_load_scr(test_scr);
}




