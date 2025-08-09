/*
 * system_shared_mem.c
 *
 *  Created on: Jun 1, 2024
 *      Author: awjpp
 */

#include "app_shared_mem.h"
#include "drivers/stm32_canbus.h"
#include "string.h"					//for memcpy.


shared_mem_t* const app_shared_mem = (shared_mem_t*)0xD02A3000;

void system_init_shared_mem()
{
	shared_mem_t* p = app_shared_mem;

	p->hs_can_baud = CAN_BAUD_ERROR;
	p->ls_can_baud = CAN_BAUD_ERROR;

	app_shared_mem->set_gauge_val = NULL;
}

void shared_mem_call_set_gauge_val(uint32_t val)
{
	if (app_shared_mem->set_gauge_val != NULL)
	{
		app_shared_mem->set_gauge_val(val);
	}
}

void shared_mem_set_can_tx(const can_tx_buffer_entry_t* tx)
{
	app_shared_mem->can_tx = *tx;

}

void shared_mem_set_can_tx_interval(uint32_t tx_interval_ms)
{
	app_shared_mem->tx_interval_ms = tx_interval_ms;
}

void shared_mem_set_can_tx_data(uint8_t data[], uint8_t data_length)
{
	memcpy(&app_shared_mem->can_tx.data, data, data_length);
}

void shared_mem_set_can_ext_id_filter(uint32_t id)
{
	if (id > 0x1FFFFFFF)	//largest 29-bit ID
	{
		return;
	}
	app_shared_mem->ext_id_filter = id;
}

void shared_mem_set_can_rx_id(uint32_t id)
{
	app_shared_mem->can_rx_can_id = id;
}

void shared_mem_set_can_rx_data(uint8_t data[8])
{
	memcpy(app_shared_mem->can_rx_data, data, sizeof(uint8_t) * 8);
}

void shared_mem_set_gauge_val_ptr(void (*func)(uint32_t val))
{
	app_shared_mem->set_gauge_val = func;
}

void shared_mem_set_converted_value(int32_t converted_val)
{
	app_shared_mem->converted_value = converted_val;
}

can_baud_rate_t shared_mem_get_can_baud_rate(FDCAN_GlobalTypeDef* canbus)
{
	if (canbus == HS_CAN)
	{
		return app_shared_mem->hs_can_baud;
	}

	if (canbus == LS_CAN)
	{
		return app_shared_mem->ls_can_baud;
	}

	return CAN_BAUD_ERROR;
}

can_tx_buffer_entry_t* shared_mem_get_can_tx()
{
	return &app_shared_mem->can_tx;
}

uint32_t shared_mem_get_can_tx_interval()
{
	return app_shared_mem->tx_interval_ms;
}

void shared_mem_get_can_tx_data(uint8_t data_destination[])
{
	memcpy(data_destination, app_shared_mem->can_tx.data, app_shared_mem->can_tx.T1.bit.DLC);
}

uint32_t shared_mem_get_ext_id_filter()
{
	return app_shared_mem->ext_id_filter;
}

int32_t shared_mem_get_converted_value()
{
	return app_shared_mem->converted_value;
}
