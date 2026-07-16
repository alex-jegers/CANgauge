
#ifndef _APP_GAUGES_CM7_H_
#define _APP_GAUGES_CM7_H_

#ifdef __cplusplus
extern "C" {
#endif

/**********     INCLUDES        **********/
#include "cangauge.h"

/**********     DEFINES      **********/

/**********     GLOBAL VARIABLE DECLRATIONS     **********/

/**********		GLOBAL FUNCTION DECLRATIONS		**********/
void app_gauges_run();
bool app_gauges_stop(uint32_t block_time_ms);	//Waits for the task to shutdown, returns true if it successfully deleted itself, false for a timeout.
void app_gauges_hard_stop();		//Deletes the task without waiting.



#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif  //_APP_GAUGES_CM7_H_
