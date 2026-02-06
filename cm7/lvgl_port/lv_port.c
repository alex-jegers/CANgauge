/**********     INCLUDES        **********/
#include "lv_port.h"
#include "lvgl/lvgl.h"
#include "drivers/drivers.h"
#include "FreeRTOS.h"
#include "semphr.h"

/**********		DEFINES		**********/

/**********		EXTERNAL VARIABLE DEFINITIONS		**********/

/**********		STATIC VARIABLES		**********/
static SemaphoreHandle_t prv_lv_mutex = NULL;
static TaskHandle_t prv_task_handle = NULL;
/**********		STATIC FUNCTION DECLRATIONS		**********/
static void prv_task_lvgl_timer_update();

/**********		STATIC FUNCTION DEFINITIONS		**********/
static void prv_task_lvgl_timer_update()
{
    lcd_init();
    while (1)
	{
		if (xSemaphoreTake(prv_lv_mutex, portMAX_DELAY) == pdPASS)
		{
			uint32_t time_till_next = lv_task_handler();
			xSemaphoreGive(prv_lv_mutex);
			vTaskDelay(pdMS_TO_TICKS(time_till_next));
		}
	}
}

/**********		GLOBAL FUNCTION DEFINITIONS		**********/
void lv_port_run()
{
    prv_lv_mutex = xSemaphoreCreateMutex();
    lv_init();
    xTaskCreate(prv_task_lvgl_timer_update, "LVGL_TASK_HANDLER", 1500, NULL, 2, &prv_task_handle);              
}

bool lv_port_take_lvgl_mutex(uint32_t block_time_ms)
{
    /* If the mutex was never created, always fail. */
    if (prv_lv_mutex == NULL)
    {
        return false;
    }
	
    if (xSemaphoreTake(prv_lv_mutex, pdMS_TO_TICKS(block_time_ms)) == pdPASS)
    {
        return true;
    }
    
    return false;
}

void lv_port_give_lvgl_mutex()
{
    xSemaphoreGive(prv_lv_mutex);
}
