
#ifndef _RUNTIME_STATS_H_
#define _RUNTIME_STATS_H_

#ifdef __cplusplus
extern "C" {
#endif

/**********     INCLUDES        **********/
#include "stm32h745xx.h"

/**********     TYPEDEFS         **********/

/**********     DEFINES      **********/

/**********     GLOBAL VARIABLE DECLRATIONS     **********/

/**********		GLOBAL FUNCTION DECLRATIONS		**********/
/**
 * name:
 *      desc:
 *      params:
 *      returns:
 */
void system_run_runtime_stats_task();
void system_config_runtime_stats();
uint32_t system_get_runtime_value();
void system_run_heap_stats_task();
char* system_get_heap_stats();




#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif  //_RUNTIME_STATS_H_
