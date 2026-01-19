
#ifndef _APP_CAN_SNIFFER_H_
#define _APP_CAN_SNIFFER_H_

#ifdef __cplusplus
extern "C" {
#endif

/**********     INCLUDES        **********/
#include "drivers/stm32_canbus.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
/**********     DEFINES      **********/

/**********     GLOBAL VARIABLE DECLRATIONS     **********/


/**********		GLOBAL FUNCTION DECLRATIONS		**********/
void app_can_controller_run(uint8_t (*data_storage)[176][10]);
void app_can_sniffer_stop();
bool app_can_sniffer_running();



#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif  //_APP_CAN_SNIFFER_H_
