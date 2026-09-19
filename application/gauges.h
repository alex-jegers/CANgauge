/**
 * @file gauges.h
 * @brief This file is responsible for everything pertaining to the display and
 * monitoring of data. 
 * 
 * 
 * @date 2026-09-12
 * 
 */
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
void app_gauges_stop();	


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif  //_APP_GAUGES_CM7_H_
