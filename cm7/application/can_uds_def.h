
#ifndef _CAN_UDS_DEF_H_
#define _CAN_UDS_DEF_H_

#ifdef __cplusplus
extern "C" {
#endif
/**********     INCLUDES        **********/
#include "drivers/stm32_canbus.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
/**********     DEFINES      **********/

/**********     TYPEDEFS        **********/
typedef struct saej1979_current_data_t saej1979_current_data_t;
struct saej1979_current_data_t
{
    const char* name;                     //For debugging and easier readability.
    const uint8_t pid_code;
    float scale;
    float offset;
    uint8_t data_bytes;     //How many bytes of data are valid (usually 1 or 2).
    uint8_t first_byte;
    int32_t min;
    int32_t max;
    const char* units;
    bool available;
    saej1979_current_data_t** nested;      //Pointer to the nested data array. NULL if not a nested PID.
};

/**********     GLOBAL VARIABLE DECLRATIONS     **********/


/**********		GLOBAL FUNCTION DECLRATIONS		**********/
/**
 * Sets the CAN frame that should be transmitted to get the ECU to return
 * data as per SAE J1979 Service 01. Unused PIDs should be set to 0xCC.
 */
void can_uds_set_current_data_query(uint8_t pid1, uint8_t pid2, uint8_t pid3, uint8_t pid4);

/** 
 * Returns a value that can be sent to the UI. 
 * If it's not a valid SAE J1979 Service 01 response CAN frame returns 0.
*/
int32_t can_uds_process_raw_data(can_rx_buffer_entry_t* input);


saej1979_current_data_t* saej1979_get_current_data(uint8_t pid);
saej1979_current_data_t* saej1979_get_current_data_by_name(const char* name);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif  //_CAN_UDS_DEF_H_
