
#ifndef _CAN_UDS_H_
#define _CAN_UDS_H_

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
uint32_t app_can_controller_get_query_can_id();	//Will return zero if there is no UDS on CAN found (i.e. no response was returned in request for PID 0x00). Otherwise returns the ID that is used to query the OBD.
uint32_t app_can_controller_get_response_can_id();	//Will return zero if there is no UDS on CAN found (i.e. no response was returned in request for PID 0x00). Otherwise returns the ID that was recieved from the ECU.
void can_uds_stop_query();					//Stops all the queries for current data being sent on the bus.
void can_uds_change_pressure_units(const char* units);
void can_uds_change_temperature_units(const char* units);


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif  //_CAN_UDS_H_
