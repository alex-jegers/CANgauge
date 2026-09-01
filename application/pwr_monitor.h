
#ifndef _PWR_MONITOR_H_
#define _PWR_MONITOR_H_

#ifdef __cplusplus
extern "C" {
#endif

/**********     INCLUDES        **********/
#include "cangauge.h"

/**********     TYPEDEFS      **********/

/**********     DEFINES      **********/

/**********     GLOBAL VARIABLE DECLRATIONS     **********/

/**********		GLOBAL FUNCTION DECLRATIONS		**********/
void pwr_monitor_run(uint8_t priority);
void pwr_monitor_suspend();
void pwr_monitor_resume();
void pwr_monitor_enter_low_pwr_mode();
bool pwr_monitor_add_low_pwr_mode_cb(void (*func)());
bool pwr_monitor_remove_low_pwr_mode_cb(void (*func()));


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif  //_PWR_MONITOR_H_
