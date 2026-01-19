
#ifndef _APP_CAN_TRANSMIT_H_
#define _APP_CAN_TRANSMIT_H_

#ifdef __cplusplus
extern "C" {
#endif

/**********     INCLUDES        **********/
#include "stm32h745xx.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "list.h"
#include "semphr.h"

/**********     DEFINES      **********/

/**********     GLOBAL VARIABLE DECLRATIONS     **********/

/**********		GLOBAL FUNCTION DECLRATIONS		**********/
void app_can_transmit_run(uint16_t* transmit_period_list, uint8_t length);
TaskHandle_t* app_can_transmit_get_task_handle();


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif  //_APP_CAN_TRANSMIT_H_