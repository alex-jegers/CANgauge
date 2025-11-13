/**********     INCLUDES        **********/
#include "app_gauges_cm7.h"
#include "ui/ui_gauges.h"
#include "drivers/stm32_canbus.h"
#include "lvgl.h"
#include "drivers/stm32_hsem.h"
#include "common/app_shared_mem.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/**********		DEFINES		**********/

/**********		EXTERNAL VARIABLE DEFINITIONS		**********/

/**********		STATIC VARIABLES		**********/
bool _run = false;
static TaskHandle_t _gauges_task_handle;

/* Data byte 2 needs to be changed depending on the data being requested. */
static can_tx_buffer_entry_t saej1979_getter_template = 
{
	.T0.bit.ID = 0x7DF << 18, .T0.bit.XTD = CAN_ID_STD, .T0.bit.RTR = CAN_RTR_DATA_FRAME, 
	.T1.bit.DLC = 8, .T1.bit.EFC = 0, .T1.bit.BRS = 0, .T1.bit.FDF = 0,
	.data[0] = 0x02, .data[1] = 0x01, .data[2] = 0x00, .data[3] = 0xCC,
	.data[4] = 0xCC, .data[5] = 0xCC, .data[6] = 0xCC, .data[7] = 0xCC,
};

/* The PIDs that are to be plugged into data byte 2 of the template above. */
uint8_t saej1979_pid_coolant_temp = 0x05;
uint8_t saej1979_pid_fuel_pressure = 0x0A;
uint8_t saej1979_pid_intake_air_pressure = 0x0B;
uint8_t saej1979_pid_timing_advance = 0x0E;
uint8_t saej1979_pid_intake_air_temp = 0x0F;
uint8_t saej1979_pid_maf_flow_rate = 0x10;
uint8_t saej1979_pid_fuel_rail_pressure = 0x22;
uint8_t saej1979_pid_air_fuel_ratio = 0x34;


/**********		STATIC FUNCTION DECLRATIONS		**********/
static void _task_gauges();							//The FreeRTOS task.
static void _gauge_btn_event_cb(lv_event_t* e);					//Handler for the gauge selection buttons events.
static void _gauge_event_cb(lv_event_t* e);			//Handler for the gauge itself events.
static void _gauge_scr_load_cb(lv_event_t* e);					//Handler for the gauge screen loading.

/**********		STATIC FUNCTION DEFINITIONS		**********/
static void _task_gauges()
{
	_run = true;

	ui_gauges_set_gauge_select_btn_cb(_gauge_btn_event_cb);
	ui_gauges_set_gauge_cb(_gauge_event_cb);

	/* Manually set the baud rate. */
	shared_set_can_baud_override(FDCAN1, CAN_BAUD_500K);

	/* Start the CAN controller on CM4. */
	hsem_lock(HSEM_APP_CAN_CONTROLLER_START, HSEM_ID_APP_CAN_CONTROLLER_START);
	hsem_signal(HSEM_APP_CAN_CONTROLLER_START, HSEM_ID_APP_CAN_CONTROLLER_START);

	/* Wait for CAN controller to be up and running. */
	//TODO: Make this actually check if CAN has been init-ed not just a delay.
	vTaskDelay(500);

	/* Add a std ID filter. */
	can_std_id_filter_t _saej1979_filter;
	_saej1979_filter.S0.bit.SFEC = CAN_SFEC_STORE_FIFO1;
	_saej1979_filter.S0.bit.SFT = CAN_SFT_RANGE;
	_saej1979_filter.S0.bit.SFID1 = 0x7E8;
	_saej1979_filter.S0.bit.SFID2 = 0x7EF;
	can_set_std_id_filter(FDCAN1, 0, &_saej1979_filter);

	while (_run)
	{
		/* Check if there is data in FIFO1. */
		if (can_check_for_rx_fifo1(FDCAN1) == false)
		{
			/*If there's not, wait for a bit and check again. */
			vTaskDelay(30);
		}

		/* Check the PID code against what it should be. */
		can_rx_buffer_entry_t* new_data = NULL;
		shared_get_can_rx1_buffer_entry(FDCAN1, 0, new_data);


		/* If it's the correct PID code call ui_gauge_set_value with the data. */
	}
}

static void _gauge_btn_event_cb(lv_event_t* e)
{
	/* Determine which button was clicked. */
	lv_obj_t* btn = lv_event_get_target_obj(e);
	lv_obj_t* lbl = lv_obj_get_child(btn, 0);
	const char* lbl_text = lv_label_get_text(lbl);

	if (strcmp(lbl_text, "Coolant Temp") == 0)
	{
		saej1979_getter_template.data[2] = saej1979_pid_coolant_temp;
	}
	else if (strcmp(lbl_text, "Fuel Pressure") == 0)
	{
		saej1979_getter_template.data[2] = saej1979_pid_fuel_pressure;
	}
	else if (strcmp(lbl_text, "Intake Air Pressure") == 0)
	{
		saej1979_getter_template.data[2] = saej1979_pid_intake_air_pressure;
	}
	else if (strcmp(lbl_text, "Timing Advance") == 0)
	{
		saej1979_getter_template.data[2] = saej1979_pid_timing_advance;
	}
	else if (strcmp(lbl_text, "Intake Air Temp") == 0)
	{
		saej1979_getter_template.data[2] = saej1979_pid_intake_air_temp;
	}
	else if (strcmp(lbl_text, "MAF Flow Rate") == 0)
	{
		saej1979_getter_template.data[2] = saej1979_pid_maf_flow_rate;
	}
	else if (strcmp(lbl_text, "Fuel Rail Pressure") == 0)
	{
		saej1979_getter_template.data[2] = saej1979_pid_fuel_rail_pressure;
	}
	else if (strcmp(lbl_text, "Air/Fuel Ratio") == 0)
	{
		saej1979_getter_template.data[2] = saej1979_pid_air_fuel_ratio;
	}

	/* Put its CAN TX message into the buffers. */
	can_add_tx_buffer(FDCAN1, &saej1979_getter_template, 0);
	
	/* Set the unique CAN TX IDs to 1. And set the interval at which to transmit at. */
	shared_set_can_tx_timing_data(30, 0);
	shared_set_can_tx_unique_ids(FDCAN1, 1);
}

static void _gauge_event_cb(lv_event_t* e)
{
	/* Tell CM4 to stop transmitting data. */
	shared_set_can_tx_unique_ids(FDCAN1, 0);
}

static void _gauge_scr_load_cb()
{

}

/**********		GLOBAL FUNCTION DEFINITIONS		**********/
void app_gauges_run()
{
	xTaskCreate(_task_gauges, "APP_GAUGES", 800, NULL, 3, _gauges_task_handle);
}

void app_gauges_stop()
{

}
