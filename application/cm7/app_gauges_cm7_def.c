/*
 * app_gauges_cm7_def.c
 *
 *  Created on: Jun 9, 2024
 *      Author: awjpp
 */
#ifdef CORE_CM7

#include "app_gauges_cm7_def.h"

/**********     DEFINES     **********/
#define VOLVO_GET_DATA_BOOST_PRESSURE			0xCD, 0x7A, 0xA6, 0x10, 0xEF, 0x01, 0x00, 0x00
#define VOLVO_GET_DATA_COOLANT_TEMP				0xCD, 0x7A, 0xA6, 0x10, 0xB8, 0x01, 0x00, 0x00
#define VOLVO_GET_DATA_OIL_LEVEL				0xCD, 0x7A, 0xA6, 0x15, 0x85, 0x01, 0x00, 0x00
#define VOLVO_GET_DATA_AF_RATIO					0xCD, 0x7A, 0xA6, 0x10, 0x2A, 0x01, 0x00, 0x00

#define BOOST_CONVERSION_VALUE					((.0145038 / 25.6285) * 10)
#define AIR_FUEL_CONVERSION_VALUE				((14.7 / 4000) * 10)
#define COOLANT_TEMP_CONVERSION_VALUE			(((3/4) - 48) * 10)


/**********    STATIC VARIABLE DEFINITIONS     **********/
static can_tx_buffer_entry_t can_msg_boost_pressure =
{
	.T0.bit.ID = 0x000FFFFE,
	.T0.bit.XTD = XTD_ID,
	.T0.bit.RTR = DATA_FRAME,
	.T1.bit.DLC = 8,
	.T1.bit.FDF = CLASSIC_CAN,
	.data = {VOLVO_GET_DATA_BOOST_PRESSURE},
};

static can_ext_id_filter_t can_filter_boost_pressure =
{
	.F0.bit.EFID1 = 0x01200021,
	.F0.bit.EFEC = FDCAN_XIDFE_EFEC_STF0M_Val,
};

static can_tx_buffer_entry_t can_msg_coolant_temp =
{
	.T0.bit.ID = 0x000FFFFE,
	.T0.bit.XTD = XTD_ID,
	.T0.bit.RTR = DATA_FRAME,
	.T1.bit.DLC = 8,
	.T1.bit.FDF = CLASSIC_CAN,
	.data = {VOLVO_GET_DATA_COOLANT_TEMP},
};

static can_tx_buffer_entry_t can_msg_oil_level =
{
	.T0.bit.ID = 0x000FFFFE,
	.T0.bit.XTD = XTD_ID,
	.T0.bit.RTR = DATA_FRAME,
	.T1.bit.DLC = 8,
	.T1.bit.FDF = CLASSIC_CAN,
	.data = {VOLVO_GET_DATA_OIL_LEVEL},
};

static can_tx_buffer_entry_t can_msg_af_ratio =
{
	.T0.bit.ID = 0x000FFFFE,
	.T0.bit.XTD = XTD_ID,
	.T0.bit.RTR = DATA_FRAME,
	.T1.bit.DLC = 8,
	.T1.bit.FDF = CLASSIC_CAN,
	.data = {VOLVO_GET_DATA_AF_RATIO},
};


/**********          GLOBAL VARIABLE DEFINITIONS     **********/
app_gauges_lut_t app_gauge_lut_boost_pressure =
{
	.gauge = &ui_boost_pressure,
	.can_msg = &can_msg_boost_pressure,
	.tx_interval_ms = 100,
	.can_return_id = 0x01200021,
	.conversion_value = BOOST_CONVERSION_VALUE,
};

app_gauges_lut_t app_gauge_lut_coolant_temp =
{
	.gauge = &ui_coolant_temp,
	.can_msg = &can_msg_coolant_temp,
	.tx_interval_ms = 1000,
	.can_return_id = 0x01200021,
	.conversion_value = COOLANT_TEMP_CONVERSION_VALUE,
};

app_gauges_lut_t app_gauge_lut_afr =
{
	.gauge = &ui_af_ratio,
	.can_msg = &can_msg_af_ratio,
	.tx_interval_ms = 100,
	.can_return_id = 0x01200021,
	.conversion_value = AIR_FUEL_CONVERSION_VALUE,
};

app_gauges_lut_t app_gauge_lut_oil_level =
{
	.gauge = &ui_oil_level,
	.can_msg = &can_msg_oil_level,
	.tx_interval_ms = 1000,
	.can_return_id = 0x01200021,
};

app_gauges_lut_t* app_gauge_lut_list[] = { &app_gauge_lut_boost_pressure,
												&app_gauge_lut_coolant_temp,
												&app_gauge_lut_afr,
												&app_gauge_lut_oil_level };

/**********     GLOBAL FUNCTION DEFINITIONS     **********/
app_gauges_lut_t* app_gauges_get_lut_from_lv_obj(lv_obj_t* obj)
{
	ui_gauge_t* gauge_t = ui_get_gauge_from_lv_obj(obj);
	for (uint32_t i = 0; i < UI_NUMBER_OF_GAUGES; i++)
	{
		if (app_gauge_lut_list[i]->gauge == gauge_t)
		{
			return app_gauge_lut_list[i];
		}
	}

	return NULL;
}

#endif //CORE_CM7
