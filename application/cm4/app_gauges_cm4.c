/*
 * app_gauges_cm4.c
 *
 *  Created on: Jun 9, 2024
 *      Author: awjpp
 */
#ifdef CORE_CM4

#include "app_gauges_cm4.h"
#include "drivers/stm32_canbus.h"
#include "application/app_shared_mem.h"
#include "drivers/stm32_hsem.h"

/**********     DEFINES    **********/
#define AF_RATIO_PARAM_ID			0x102A
#define BOOST_PARAM_ID				0x10EF
#define COOLANT_TEMP_PARAM_ID		0x10B8
#define OIL_LEVEL_PARAM_ID			0x1585

/**********     STATIC VARIABLES     **********/
static can_tx_buffer_entry_t tx_data_buffer;

/**********     STATIC FUNCTION PROTOTYPES     **********/
static void new_rx_int_handler();
static uint32_t get_last_rx_param_id();
static uint32_t get_tx_param_id();
static uint32_t get_last_rx_param_val();
static int32_t convert_raw_can_param_val(uint32_t param_id, uint32_t param_val);
static int32_t af_convert_value(uint32_t raw_can_val);
static int32_t boost_convert_value(uint32_t raw_can_value);
static int32_t coolant_temp_convert_value(uint32_t raw_can_value);
static int32_t oil_level_convert_value(uint32_t raw_can_value);

/**********     STATIC FUNCTION DEFINITIONS     **********/
static void new_rx_int_handler()
{
	can_rx_buffer_entry_t message;
	can_read_from_fifo0(HS_CAN, &message);

	uint32_t id = message.R0.bit.ID;
	uint8_t* data = &message.data;
	uint32_t rx_param_id, tx_param_id, param_val;
	int32_t converted_val;


	/*For debugging.*/
	shared_mem_set_can_rx_id(id);
	shared_mem_set_can_rx_data(data);

	/*Check if the received message matches the request one.*/
	rx_param_id = get_last_rx_param_id();
	tx_param_id = get_tx_param_id();

	if (rx_param_id == tx_param_id)
	{
		param_val = get_last_rx_param_val();
		converted_val = convert_raw_can_param_val(rx_param_id, param_val);
		shared_mem_set_converted_value(converted_val);
		hsem_lock(4,0);
		hsem_signal(4,0);
	}

}

static uint32_t get_last_rx_param_id()
{
	uint32_t param_id = (app_shared_mem->can_rx_data[3] << 8) | app_shared_mem->can_rx_data[4];
	return param_id;
}

static uint32_t get_tx_param_id()
{
	uint32_t param_id = (app_shared_mem->can_tx.data[3] << 8) | app_shared_mem->can_tx.data[4];
	return param_id;
}

static uint32_t get_last_rx_param_val()
{
	uint32_t param_length = app_shared_mem->can_rx_data[0];
	uint32_t val = 0;
	if (param_length == 0xCD)
	{
		val = app_shared_mem->can_rx_data[5];
		return val;
	}

	if (param_length == 0xCE)
	{
		val = (app_shared_mem->can_rx_data[5] << 8) | app_shared_mem->can_rx_data[6];
		return val;
	}

	return val;
}

static int32_t convert_raw_can_param_val(uint32_t param_id, uint32_t param_val)
{
	if (param_id == AF_RATIO_PARAM_ID)
	{
		return af_convert_value(param_val);
	}
	if(param_id == BOOST_PARAM_ID)
	{
		return boost_convert_value(param_val);
	}
	if (param_id == COOLANT_TEMP_PARAM_ID)
	{
		return coolant_temp_convert_value(param_val);
	}
	if (param_id == OIL_LEVEL_PARAM_ID)
	{
		return oil_level_convert_value(param_val);
	}
}

static int32_t af_convert_value(uint32_t raw_can_val)
{
	float lambda = (float)raw_can_val / 4000.0;
	float air_fuel_ratio = lambda * 14.7;
	air_fuel_ratio *= 10;
	return (int32_t)air_fuel_ratio;
}

static int32_t boost_convert_value(uint32_t raw_can_value)
{
	float pres_kpa = (float)raw_can_value / 25.62857;
	float pres_psi = pres_kpa * .0145038;
	pres_psi -= 14.2;
	pres_psi *= 10;
	return (int32_t)pres_psi;
}
static int32_t coolant_temp_convert_value(uint32_t raw_can_value)
{
	int32_t converted_value_celsius = (raw_can_value * 3) / 4;
	converted_value_celsius -= 48;
	float converted_value_fahrenheit = (converted_value_celsius * (9.0f/5.0f)) + 32.0f;
	converted_value_fahrenheit *= 10;
	return converted_value_fahrenheit;
}
static int32_t oil_level_convert_value(uint32_t raw_can_value)
{
	float percent_fill = ((float)raw_can_value / 255.0f) * 100.0f;
	percent_fill *= 10;
	return (int32_t)percent_fill;
}

/**********     GLOBAL FUNCTION DEFINITIONS     **********/
void app_gauges_run()
{
	/*Make sure CAN is initialized.*/
	can_init();
	can_init_clk();

	can_assign_rx_rf0n_cb(HS_CAN, new_rx_int_handler);
	can_enable_rx_rf0n_interrupt(HS_CAN);

	/*Set the baud rates.*/
	can_baud_rate_t hs_can_baud_rate = shared_mem_get_can_baud_rate(HS_CAN);
	can_baud_rate_t ls_can_baud_rate = shared_mem_get_can_baud_rate(LS_CAN);
	can_set_baud_rate(HS_CAN, hs_can_baud_rate);
	can_set_baud_rate(LS_CAN, ls_can_baud_rate);

	can_run(HS_CAN);

	/*Loop until CM7 says not to.*/
	while (!hsem_wait_bool(5,0))
	{
		/*Loop and do CAN processes until the TX data changes.*/
		while (!hsem_wait_bool(3,0))
		{
			if (hsem_wait_bool(5,0) == true)
			{
				break;
			}
			can_processes();
		}

		can_tx_buffer_entry_t tx_data_buffer = *shared_mem_get_can_tx();
		uint32_t tx_interval_ms = shared_mem_get_can_tx_interval();
		uint32_t rx_filter_id = shared_mem_get_ext_id_filter();

		can_deactivate_all_tx(HS_CAN);
		can_remove_all_ext_id_filters(HS_CAN);
		can_add_tx_buffer(HS_CAN, &tx_data_buffer, tx_interval_ms, true);
		can_add_ext_id_filter(HS_CAN, rx_filter_id, true);
		can_activate_tx(HS_CAN, &tx_data_buffer);

		hsem_clear_int(3);
	}
	hsem_clear_int(5);
}

#endif //CORE_CM4
