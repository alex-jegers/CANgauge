
#ifndef _LV_PORT_H_
#define _LV_PORT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/**********     DEFINES      **********/

/**********		TYPEDEFS		**********/

/**********		INCLUDES		**********/

/**********     GLOBAL VARIABLE DECLRATIONS     **********/

/**********		GLOBAL FUNCTION DECLRATIONS		**********/
/**
 * lv_port_run():
 * 
 * desc: initializes LVGL, creates the LVGL mutex, and a FreeRTOS 
 * task to run the LVGL task handler function.
 */
void lv_port_run();

/**
 * lv_port_take_lvgl_mutex:
 * 
 * desc: attempts to take the LVGL mutex (must be called before using
 * any LVGL related function).
 * 
 * params:
 * block_time_ms: the amount of time in milliseconds for the task to 
 * block while waiting for the mutex.
 * 
 * return: 
 * true: the mutex was successfully taken.
 * false: the mutex could not be obtained
 */
bool lv_port_take_lvgl_mutex(uint32_t block_time_ms);

/**
 * lv_port_give_lvgl_mutex:
 * 
 * desc: give the LVGL mutex, must be called upon return from any
 * LVGL related function.
 */
void lv_port_give_lvgl_mutex();


bool lv_port_stop(uint32_t block_time_ms);



#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif  //_LV_PORT_H_
