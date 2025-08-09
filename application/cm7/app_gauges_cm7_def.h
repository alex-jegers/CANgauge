/*
 * app_gauges_cm7_data.h
 *
 *  Created on: Jun 9, 2024
 *      Author: awjpp
 */

#ifdef CORE_CM7

#ifndef CORE_APPLICATION_CM7_APP_GAUGES_CM7_DEF_H_
#define CORE_APPLICATION_CM7_APP_GAUGES_CM7_DEF_H_


/**********     INCLUDES     **********/
#include "ui/ui_gauges.h"
#include "drivers/stm32_canbus.h"

/**********     TYPEDEFS     **********/
typedef struct
{
	ui_gauge_t* gauge;
	const can_tx_buffer_entry_t* can_msg;
	const uint32_t can_return_id;
	uint32_t tx_interval_ms;
	float conversion_value;
}app_gauges_lut_t;

/**********     GLOBAL VARIABLE DECLARATIONS     **********/
extern app_gauges_lut_t app_gauge_lut_boost_pressure;
extern app_gauges_lut_t app_gauge_lut_coolant_temp;
extern app_gauges_lut_t app_gauge_lut_afr;
extern app_gauges_lut_t app_gauge_lut_oil_level;
extern app_gauges_lut_t* app_gauge_lut_list[];


/**********     GLOBAL FUNCTION DEFINITIONS    **********/
app_gauges_lut_t* app_gauges_get_lut_from_lv_obj(lv_obj_t* obj);

#endif /* CORE_APPLICATION_CM7_APP_GAUGES_CM7_DATA_H_ */

#endif //CORE_CM7
