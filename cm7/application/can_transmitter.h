
#ifndef _APP_CAN_TRANSMIT_H_
#define _APP_CAN_TRANSMIT_H_

#ifdef __cplusplus
extern "C" {
#endif

/**********     INCLUDES        **********/
#include "system/system_cm7.h"
#include "drivers/drivers.h"

/**********     DEFINES      **********/
#define CAN_TRANSMIT_PERIOD_ONE_SHOT        0xFFFF

/**********     TYPEDEFS        **********/
typedef struct 
{
    bool active;
    uint32_t period_ms;     //use CAN_TRANSMIT_PERIOD_ONE_SHOT for a single, non-repeating msg.
    uint32_t last_time_sent;
    can_tx_buffer_entry_t* buf;
}can_transmit_handle_t;


/**********     GLOBAL VARIABLE DECLRATIONS     **********/

/**********		GLOBAL FUNCTION DECLRATIONS		**********/
void can_transmit_run(FDCAN_GlobalTypeDef* can, uint32_t min_time_between_msg_ms);
bool can_transmit_stop(uint32_t block_time_ms);

can_transmit_handle_t* can_transmit_create_msg();
bool can_transmit_set_period(can_transmit_handle_t* hndl, uint32_t period_ms);
bool can_transmit_set_msg_data(can_transmit_handle_t* hndl, can_tx_buffer_entry_t* buf);
bool can_transmit_set_active(can_transmit_handle_t* hndl);
bool can_transmit_set_inactive(can_transmit_handle_t* hndl);
bool can_transmit_delete_handle(can_transmit_handle_t* hndl);

TaskHandle_t* can_transmit_get_task_handle();


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif  //_APP_CAN_TRANSMIT_H_
