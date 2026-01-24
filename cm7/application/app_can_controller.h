
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
 * 
 * params: 
 * data_storage: a pointer to a 2D array were the first index is the 
 * SAE J1979 PID and the second index is the data byte for said
 * PID where 0 = A, 1 = B, and so on.
 * 
 * desc:
 * processes incoming CAN data looking for UDSonOBD current powertrain
 * data packets and stores them in the location pointed to by data storage
 * upon receipt.
 */
void app_can_controller_run(uint8_t (*data_storage)[176][10]);
void app_can_controller_stop();
bool app_can_sniffer_running();



#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif  //_APP_CAN_SNIFFER_H_
