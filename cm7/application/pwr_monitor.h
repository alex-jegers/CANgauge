
#ifndef _PWR_MONITOR_H_
#define _PWR_MONITOR_H_

#ifdef __cplusplus
extern "C" {
#endif

/**********     INCLUDES        **********/
#include "system/system_cm7.h"


/**********     DEFINES      **********/

/**********     GLOBAL VARIABLE DECLRATIONS     **********/

/**********		GLOBAL FUNCTION DECLRATIONS		**********/
void pwr_monitor_run(uint8_t priority);



#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif  //_PWR_MONITOR_H_
