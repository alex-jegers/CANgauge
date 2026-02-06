
#ifndef _APP_BATTERY_MONITOR_H_
#define _APP_BATTERY_MONITOR_H_

#ifdef __cplusplus
extern "C" {
#endif

/**********     INCLUDES        **********/
#include "stm32h745xx.h"

#include "common/drivers/stm32_adc.h"

#include "FreeRTOS.h"
#include <task.h>
#include <queue.h>
#include <list.h>
#include <semphr.h>


/**********     DEFINES      **********/

/**********     GLOBAL VARIABLE DECLRATIONS     **********/

/**********		GLOBAL FUNCTION DECLRATIONS		**********/
void app_battery_monitor_run(uint8_t priority);



#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif  //_APP_BATTERY_MONITOR_H_
