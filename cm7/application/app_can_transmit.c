/**********     INCLUDES        **********/
#include "app_can_transmit.h"
#include "drivers/drivers.h"


/**********		DEFINES		**********/

/**********		EXTERNAL VARIABLE DEFINITIONS		**********/

/**********		STATIC VARIABLES		**********/
static bool prv_task_run = false;
static TaskHandle_t prv_task_handle = NULL;
static uint16_t* prv_transmit_period_list = NULL;
static uint8_t prv_number_of_entries = 0;
static uint32_t prv_last_time_list[32];

/**********		STATIC FUNCTION DECLRATIONS		**********/
void prv_task_can_transmit();

/**********		STATIC FUNCTION DEFINITIONS		**********/
void prv_task_can_transmit()
{
    static uint32_t current_time_ms = 0;
    static uint32_t time_till_next = pdMS_TO_TICKS(500);
    while (prv_task_run)
    {
        current_time_ms = xTaskGetTickCount();

        for (uint8_t i = 0; i < prv_number_of_entries; i++)
        {
            if (prv_transmit_period_list[i] == 0)
            {
                continue;
            }

            if (prv_transmit_period_list[i] == 0xFFFF)
            {
                can_tx(FDCAN1, i);
                prv_transmit_period_list[i] = 0;
                continue;
            }

            uint32_t time_since_last = current_time_ms - prv_last_time_list[i];
            /* If the time since the last TX is greater than the period... */
            if (time_since_last > prv_transmit_period_list[i])
            {
                /* Transmit the message. */
                can_tx(FDCAN1, i);

                /* Reset the last time sent. */
                prv_last_time_list[i] = current_time_ms;
                
                /* Check if this messages period is greater than or equal to the time till next TX...*/
                if (prv_transmit_period_list[i] <= time_till_next)
                {
                    /* If it is, make this the new time till next. */
                    time_till_next = prv_transmit_period_list[i];
                }
                continue;
            }
            /* If the buffer isn't due to be sent... */
            /* Calculate how much time is left till it is ready to be sent. */
            uint32_t next = prv_transmit_period_list[i] - time_since_last;
            /* Check if this is less than the current amount of time to wait. */
            if (next <= time_till_next)
            {
                time_till_next = next;
            }
        }
        if (can_get_last_error_code(FDCAN1) == CAN_ERROR_CODE_ACK_ERROR)
        {
            prv_task_run = false;
        }
        vTaskDelay(time_till_next);
        time_till_next = pdMS_TO_TICKS(500);
    }
    can_stop(FDCAN1);
    can_deinit(FDCAN1);
    hsem_signal(31,0);
    vTaskDelete(NULL);
}

/**********		GLOBAL FUNCTION DEFINITIONS		**********/
void app_can_transmit_run(uint16_t* transmit_period_list, uint8_t length)
{
    prv_transmit_period_list = transmit_period_list;
    prv_number_of_entries = length;
    memset(prv_transmit_period_list, 0, length);

    /* If FDCAN1 is not already running. */
    if (hsem_lock(31,0) == true)
    {
        can_init(FDCAN1);
        can_set_baud_rate(FDCAN1, CAN_BAUD_500K);
        can_run(FDCAN1);
    }

    prv_task_run = true;
    xTaskCreate(prv_task_can_transmit, "CAN_TRANSMIT", 300, NULL, 3, &prv_task_handle);
}

void app_can_transmit_stop()
{
    prv_task_run = false;
}

TaskHandle_t* app_can_transmit_get_task_handle()
{
    return &prv_task_handle;
}
