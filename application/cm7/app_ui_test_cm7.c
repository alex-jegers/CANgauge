/**********     INCLUDES        **********/
#include "app_ui_test_cm7.h"
#include "ui/ui_test.h"
#include <stdbool.h>
#include "lvgl/lvgl.h"

/**********		DEFINES		**********/

/**********		EXTERNAL VARIABLE DEFINITIONS		**********/

/**********		STATIC VARIABLES		**********/
bool app_run = false;

/**********		STATIC FUNCTION DECLRATIONS		**********/

/**********		STATIC FUNCTION DEFINITIONS		**********/

/**********		GLOBAL FUNCTION DEFINITIONS		**********/
void app_ui_test_run()
{
    app_run = true;
    ui_load_test_screen();
    while (app_run == true)
    {
        lv_timer_handler();
    }
    
}