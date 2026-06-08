
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
/**
 * app_can_controller_run:
 * desc:
 * 
 */
BaseType_t app_can_controller_run();
bool app_can_controller_stop(uint32_t block_time_ms);
uint32_t can_controller_get_data(uint8_t pid, uint8_t first_byte, uint8_t num_params);
bool app_can_sniffer_running();
bool app_can_controller_is_init(uint32_t block_time_ms);
uint32_t app_can_controller_get_can_id();



#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif  //_APP_CAN_SNIFFER_H_
