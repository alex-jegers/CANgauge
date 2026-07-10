
#ifndef _APP_GAUGES_CM7_H_
#define _APP_GAUGES_CM7_H_

#ifdef __cplusplus
extern "C" {
#endif

/**********     INCLUDES        **********/
#include "system/system_cm7.h"
#include <string.h>

/**********     DEFINES      **********/

/**********     GLOBAL VARIABLE DECLRATIONS     **********/

/**********		GLOBAL FUNCTION DECLRATIONS		**********/
void app_gauges_run();
bool app_gauges_stop(uint32_t block_time_ms);



#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif  //_APP_GAUGES_CM7_H_
