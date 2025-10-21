
#ifndef _APP_CAN_SNIFFER_H_
#define _APP_CAN_SNIFFER_H_

#ifdef __cplusplus
extern "C" {
#endif

/**********     INCLUDES        **********/
#include "drivers/stm32_canbus.h"

#include "FreeRTOS.h"
#include "task.h"
/**********     DEFINES      **********/

/**********     GLOBAL VARIABLE DECLRATIONS     **********/
extern TaskHandle_t app_can_sniffer_task_handle;

/**********		GLOBAL FUNCTION DECLRATIONS		**********/
void app_can_sniffer_run();



#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif  //_APP_CAN_SNIFFER_H_
