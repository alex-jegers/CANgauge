/**********     INCLUDES        **********/
#include "app_ui_test_cm7.h"

#include "ui/ui_test.h"
#include "lvgl/demos/benchmark/lv_demo_benchmark.h"

#include <stdbool.h>

#include "lvgl/lvgl.h"

/**********		DEFINES		**********/

/**********		EXTERNAL VARIABLE DEFINITIONS		**********/

/**********		STATIC VARIABLES		**********/
static bool app_run = false;
static uint32_t time_till_next = 0;
static uint32_t current_time = 0;

/**********		STATIC FUNCTION DECLRATIONS		**********/

/**********		STATIC FUNCTION DEFINITIONS		**********/

/**********		GLOBAL FUNCTION DEFINITIONS		**********/
void app_ui_test_run()
{
    app_run = true;
    lv_demo_benchmark();
    while (app_run == true)
    {
        time_till_next = lv_timer_handler();
        current_time = timer_get_time_ms();

        while ((timer_get_time_ms() - current_time) < time_till_next) {}
    }
    
}
