/**********     INCLUDES        **********/
#include "saej1979.h"
#include "string.h"
#include "common/app_shared_mem.h"
/**********		DEFINES		**********/

/**********		EXTERNAL VARIABLE DEFINITIONS		**********/

/**********		STATIC FUNCTION DECLRATIONS		**********/
static int32_t convert_coolant_temp(uint32_t val) { return val - 40; };
static int32_t convert_fuel_pressure(uint32_t val) { return val * 3;};
static int32_t convert_intake_air_pressure(uint32_t val) { return val; };
static int32_t convert_timing_advance(uint32_t val) { return (val / 2) - 64; };
static int32_t convert_intake_air_temp(uint32_t val) { return val - 40; };
static int32_t convert_maf_flow_rate(uint32_t val) { return val / 100; };
static int32_t convert_fuel_rail_pressure(uint32_t val);
static int32_t convert_air_fuel_ratio(uint32_t val);

/**********		STATIC VARIABLES		**********/
static saej1979_current_data_t saej1979_coolant_temp =          {.name = "Coolant Temp",        .pid_code = 0x05,   .conversion_func = convert_coolant_temp, 		.data_bytes = 1};
static saej1979_current_data_t saej1979_fuel_pressure =         {.name = "Fuel Pressure",       .pid_code = 0x0A,   .conversion_func = convert_fuel_pressure, 		.data_bytes = 1};
static saej1979_current_data_t saej1979_intake_air_pressure =   {.name = "Intake Air Temp",     .pid_code = 0x0B,   .conversion_func = convert_intake_air_pressure, .data_bytes = 1};
static saej1979_current_data_t saej1979_timing_advance =        {.name = "Timing Advance",      .pid_code = 0x0E,   .conversion_func = convert_timing_advance, 		.data_bytes = 1};
static saej1979_current_data_t saej1979_intake_air_temp =       {.name = "Intake Air Temp",     .pid_code = 0x0F,   .conversion_func = convert_intake_air_temp, 	.data_bytes = 1};
static saej1979_current_data_t saej1979_maf_flow_rate =         {.name = "MAF Flow Rate",       .pid_code = 0x10,   .conversion_func = convert_maf_flow_rate, 		.data_bytes = 2};
static saej1979_current_data_t saej1979_fuel_rail_pressure =    {.name = "Fuel Rail Pressure",  .pid_code = 0x22,   .conversion_func = convert_fuel_rail_pressure, 	.data_bytes = 2};
static saej1979_current_data_t saej1979_air_fuel_ratio =        {.name = "Air/Fuel Ratio",      .pid_code = 0x34,   .conversion_func = convert_air_fuel_ratio, 		.data_bytes = 2};

saej1979_current_data_t* saej1979_current_data_arr[] = {&saej1979_coolant_temp,
&saej1979_fuel_pressure,
&saej1979_intake_air_temp,
&saej1979_timing_advance,
&saej1979_intake_air_temp,
&saej1979_maf_flow_rate,
&saej1979_fuel_rail_pressure,
&saej1979_air_fuel_ratio};

/**********		STATIC FUNCTION DEFINITIONS		**********/
static int32_t convert_fuel_rail_pressure(uint32_t val)
{
    float temp = (float)val * 0.079f;
    return (int32_t)temp;
}

static int32_t convert_air_fuel_ratio(uint32_t val)
{
    float temp = ((float)val * 2.0f / 65535.0f) * 14.7f;
    temp = temp * 10;
    return (int32_t)temp;
}
/**********		GLOBAL FUNCTION DEFINITIONS		**********/
void saej1979_current_data_set_getter(const char* name)
{
	/* Data byte 2 needs to be changed depending on the data being requested. */
	static can_tx_buffer_entry_t saej1979_getter_template =
	{
		.T0.bit.ID = 0x7DF << 18, .T0.bit.XTD = CAN_ID_STD, .T0.bit.RTR = CAN_RTR_DATA_FRAME,
		.T1.bit.DLC = 8, .T1.bit.EFC = 0, .T1.bit.BRS = 0, .T1.bit.FDF = 0,
		.data[0] = 0x02, .data[1] = 0x01, .data[2] = 0x00, .data[3] = 0xCC,
		.data[4] = 0xCC, .data[5] = 0xCC, .data[6] = 0xCC, .data[7] = 0xCC,
	};

	uint32_t num_params = sizeof(saej1979_current_data_arr) / sizeof(saej1979_current_data_t*);
    for (uint8_t i = 0; i < num_params; i++)
	{
		if (strcmp(name, saej1979_current_data_arr[i]->name) == 0)
		{
			saej1979_getter_template.data[2] = saej1979_current_data_arr[i]->pid_code;
			can_add_tx_buffer(FDCAN1, &saej1979_getter_template, 0);
			shared_set_can_tx_timing_data(30, 0);
			shared_set_can_tx_unique_ids(FDCAN1, 1);
			return;
		}
	}
}

int32_t saej1979_current_data_process_data(can_rx_buffer_entry_t* input)
{
	uint32_t id = input->R0.bit.ID;
	uint8_t pid = input->data[2];
	uint32_t data = 0;
	uint32_t num_params = sizeof(saej1979_current_data_arr) / sizeof(saej1979_current_data_t*);
    for (uint8_t i = 0; i < num_params; i++)
	{
		if (pid == saej1979_current_data_arr[i]->pid_code)
		{
			if (saej1979_current_data_arr[i]->data_bytes == 1)
			{
				data = input->data[3];
			}
			else if (saej1979_current_data_arr[i]->data_bytes == 2)
			{
				data = (input->data[3] << 8) | input->data[4];
			}
			data = saej1979_current_data_arr[i]->conversion_func(data);
			return data;
		}
	}	
	return 0;
}
