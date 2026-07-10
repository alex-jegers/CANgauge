
#ifndef _ERROR_HANDLER_H_
#define _ERROR_HANDLER_H_

#ifdef __cplusplus
extern "C" {
#endif

/**********     INCLUDES        **********/
#include "system/system_cm7.h"
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
void error_handler_run();

/**
 * error_handler_log_from_isr:
 * 		returns: higher_pri_task_woken to be passed into portYIELD_FROM_ISR().
 */
BaseType_t error_handler_log_from_isr(char* log);


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif  //_ERROR_HANDLER_H_
