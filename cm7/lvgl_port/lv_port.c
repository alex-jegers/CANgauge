/**********     INCLUDES        **********/
#include "lv_port.h"
#include "lvgl/lvgl.h"
#include "drivers/drivers.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "event_groups.h"

/**********		DEFINES		**********/
#define EVENT_BITS_TASK_STOPPED        (EventBits_t)0x01       //Bit is set when the task is deleted.

/**********		EXTERNAL VARIABLE DEFINITIONS		**********/

/**********		STATIC VARIABLES		**********/
static EventGroupHandle_t prv_event_group = NULL;
static SemaphoreHandle_t prv_lv_mutex = NULL;
static TaskHandle_t prv_task_handle = NULL;
static bool prv_run = false;
/**********		STATIC FUNCTION DECLRATIONS		**********/
static void prv_task_lvgl_timer_update();

/**********		STATIC FUNCTION DEFINITIONS		**********/
static void prv_task_lvgl_timer_update()
{
    /* Initialize the LTDC and the LCD itself. */
    lcd_init();
    while (prv_run)
	{
		if (xSemaphoreTake(prv_lv_mutex, portMAX_DELAY) == pdPASS)
		{
			uint32_t time_till_next = lv_task_handler();
			xSemaphoreGive(prv_lv_mutex);
			vTaskDelay(pdMS_TO_TICKS(time_till_next));
		}
	}
    xEventGroupSetBits(prv_event_group, EVENT_BITS_TASK_STOPPED);
    vTaskDelete(NULL);
}

/**********		GLOBAL FUNCTION DEFINITIONS		**********/
void lv_port_run()
{
    /* Create the mutex to guard LVGL. */
    prv_lv_mutex = xSemaphoreCreateMutex();

    /* Create an event group to signal when the task is deleted and clear the task stop bit. */
    prv_event_group = xEventGroupCreate();
    xEventGroupClearBits(prv_event_group, EVENT_BITS_TASK_STOPPED);
    
    /* Initializes the LVGL library. */
    lv_init();  

    prv_run = true;
    xTaskCreate(prv_task_lvgl_timer_update, "LVGL_TASK_HANDLER", 5000 / 4, NULL, 2, &prv_task_handle);
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

bool lv_port_stop(uint32_t block_time_ms)
{
    prv_run = false;

    /* If the event group is NULL, the task was never even created in the first place. */
    if (prv_event_group == NULL)
    {
    	return pdTRUE;
    }

    uint32_t rtn = xEventGroupWaitBits(prv_event_group, EVENT_BITS_TASK_STOPPED,    //Bits to wait for.
                                        pdFALSE,        //Dont clear the bits on exit.
                                        pdTRUE,         //wait for all the bits (it's only 1)
                                        block_time_ms); //Block time.
    if (rtn & EVENT_BITS_TASK_STOPPED != 0)
    {
        return pdTRUE;
    }
    else
    {
        return pdFALSE;
    }
}
