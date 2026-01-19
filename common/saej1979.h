
#ifndef _SAEJ1979_H_
#define _SAEJ1979_H_

#ifdef __cplusplus
extern "C" {
#endif

/**********     INCLUDES        **********/
#include "drivers/stm32_canbus.h"

/**********     DEFINES      **********/

/**********     GLOBAL VARIABLE DECLRATIONS     **********/
typedef struct
{
    char* name;                     //For debugging and easier readability.
    const uint8_t pid_code;
    float scale;
    float offset;
    uint8_t data_bytes;     //How many bytes of data are valid (usually 1 or 2).
    uint8_t first_byte;
    int32_t min;
    int32_t max;
    const char* units;
    bool gauge;
    int32_t (*processing_func)(uint32_t data);
}saej1979_current_data_t;


/**********		GLOBAL FUNCTION DECLRATIONS		**********/
/**
 * Sets the CAN frame that should be transmitted to get the ECU to return
 * data as per SAE J1979 Service 01.
 */
void saej1979_current_data_set_getter(uint8_t pid);

/** 
 * Returns a value that can be sent to the UI. 
 * If it's not a valid SAE J1979 Service 01 response CAN frame returns 0.
*/
int32_t saej1979_current_data_process_data(can_rx_buffer_entry_t* input);


saej1979_current_data_t* saej1979_get_current_data(uint8_t pid);
saej1979_current_data_t* saej1979_get_current_data_by_name(const char* name);


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif  //_SAEJ1979_H_
