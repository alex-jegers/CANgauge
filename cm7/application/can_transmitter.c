/**********     INCLUDES        **********/
#include "can_transmitter.h"
#include "drivers/drivers.h"


/**********		DEFINES		**********/
#define EVENT_BITS_TASK_STOPPED     0x1 << 0

/**********		EXTERNAL VARIABLE DEFINITIONS		**********/

/**********		STATIC VARIABLES		**********/
static TaskHandle_t prv_task_handle = NULL;
static EventGroupHandle_t prv_event_group = NULL;
static bool prv_task_run = false;

static can_transmit_handle_t prv_handles[32];
static uint32_t prv_min_time_between_msg_ms = 0;

/**********		STATIC FUNCTION DECLRATIONS		**********/
static void prv_task_can_transmit();
static void prv_delete_handle(can_transmit_handle_t* hndl);

/**********		STATIC FUNCTION DEFINITIONS		**********/
static void prv_task_can_transmit()
{
    static uint32_t current_time_ms = 0;
    static uint32_t time_till_next = pdMS_TO_TICKS(500);
    while (prv_task_run)
    {
        current_time_ms = xTaskGetTickCount();

        for (uint8_t i = 0; i < 32; i++)
        {
            /* If the the handle isn't point to a buffer go to the next one. */
            if (prv_handles[i].buf == NULL)
            {
                continue;
            }
            /* If it's not active go to the next one. */
            if (prv_handles[i].active == false)
            {
                continue;
            }

            /* If the period is one shot, send it then delete it. Delay for the min time between messages. */
            if (prv_handles[i].period_ms == CAN_TRANSMIT_PERIOD_ONE_SHOT)
            {
                can_tx(FDCAN1, i);
                prv_delete_handle(&prv_handles[i]);
                time_till_next = prv_min_time_between_msg_ms;
                break;
            }

            uint32_t time_since_last = current_time_ms - prv_handles[i].last_time_sent;
            /* If the time since the last TX is greater than the period... */
            if (time_since_last >= prv_handles[i].period_ms)
            {
                /* Transmit the message. */
                can_tx(FDCAN1, i);

                /* Reset the last time sent. */
                prv_handles[i].last_time_sent = current_time_ms;
                
                /* Check if this messages period is greater than or equal to the time till next TX...*/
                if (prv_handles[i].period_ms <= time_till_next)
                {
                    /* If it is, make this the new time till next. */
                    time_till_next = prv_handles[i].period_ms;
                }
                continue;
            }
            else
            {
                /* If the buffer isn't due to be sent... */
                /* Calculate how much time is left till it is ready to be sent. */
                uint32_t next = prv_handles[i].period_ms - time_since_last;
                /* Check if this is less than the current amount of time to wait. */
                if (next <= time_till_next)
                {
                    time_till_next = next;
                }   
            }

        }

        if (can_get_last_error_code(FDCAN1) == CAN_ERROR_CODE_ACK_ERROR)
        {
            prv_task_run = false;
        }
        vTaskDelayUntil(&current_time_ms, time_till_next);
        time_till_next = pdMS_TO_TICKS(500);
    }
    can_stop(FDCAN1);
    can_deinit(FDCAN1);
    xEventGroupSetBits(prv_event_group, EVENT_BITS_TASK_STOPPED);
    vTaskDelete(NULL);
}

static void prv_delete_handle(can_transmit_handle_t* hndl)
{
    hndl->buf = NULL;
    hndl->active = false;
    hndl->last_time_sent = 0;
    hndl->period_ms = 0;
}

/**********		GLOBAL FUNCTION DEFINITIONS		**********/
void can_transmit_run(FDCAN_GlobalTypeDef* can, uint32_t min_time_between_msg_ms)
{
    prv_min_time_between_msg_ms = min_time_between_msg_ms;
    prv_event_group = xEventGroupCreate();
    xEventGroupClearBits(prv_event_group, EVENT_BITS_TASK_STOPPED);
    prv_task_run = true;
    xTaskCreate(prv_task_can_transmit, "CAN_TRANSMIT", 300, NULL, 3, &prv_task_handle);
}

bool can_transmit_stop(uint32_t block_time_ms)
{
    prv_task_run = false;

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

TaskHandle_t* can_transmit_get_task_handle()
{
    return &prv_task_handle;
}

can_transmit_handle_t* can_transmit_create_msg()
{
    for (uint32_t i = 0; i < 32; i++)
    {
        if (prv_handles[i].buf == NULL)
        {
            prv_handles[i].buf = can_get_tx_buffer(FDCAN1, i);
            return &prv_handles[i];
        }
    }
    return NULL;
}
bool can_transmit_set_period(can_transmit_handle_t* hndl, uint32_t period_ms)
{
    if (hndl->buf == NULL)
    {
        return false;
    }
    hndl->period_ms = period_ms;
}

bool can_transmit_set_msg_data(can_transmit_handle_t* hndl, can_tx_buffer_entry_t* buf)
{
    if (hndl->buf == NULL)
    {
        return false;
    }

    /* Copy it to message RAM. */
    *hndl->buf = *buf;

    /* Shift the ID over 18 bits if it's a std length ID. */
    if (hndl->buf->T0.bit.XTD == CAN_ID_STD)
    {
        hndl->buf->T0.bit.ID = hndl->buf->T0.bit.ID << 18;
    }
}

bool can_transmit_set_active(can_transmit_handle_t* hndl)
{
    if (hndl->buf == NULL)
    {
        return false;
    }
    hndl->active = true;
}

bool can_transmit_set_inactive(can_transmit_handle_t* hndl)
{
    if (hndl->buf == NULL)
    {
        return false;
    }
    hndl->active = false;
    return true;
}

bool can_transmit_delete_handle(can_transmit_handle_t* hndl)
{
    prv_delete_handle(hndl);
}

