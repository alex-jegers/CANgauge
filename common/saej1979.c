/**********     INCLUDES        **********/
#include "saej1979.h"
#include "string.h"
#include "common/app_shared_mem.h"
/**********		DEFINES		**********/

/**********		EXTERNAL VARIABLE DEFINITIONS		**********/

/**********		STATIC FUNCTION DECLRATIONS		**********/
static int32_t convert_coolant_temp(uint32_t val) { return val - 40; };
<<<<<<< Updated upstream
static int32_t convert_fuel_pressure(uint32_t val) { return val * 3;};
=======
static int32_t convert_gauge_fuel_pressure(uint32_t val) { return val * 3;};
>>>>>>> Stashed changes
static int32_t convert_intake_air_pressure(uint32_t val) { return val; };
static int32_t convert_timing_advance(uint32_t val) { return (val / 2) - 64; };
static int32_t convert_intake_air_temp(uint32_t val) { return val - 40; };
static int32_t convert_maf_flow_rate(uint32_t val) { return val / 100; };
static int32_t convert_fuel_rail_pressure(uint32_t val);
static int32_t convert_air_fuel_ratio(uint32_t val);

/**********		STATIC VARIABLES		**********/
<<<<<<< Updated upstream
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
=======

static saej1979_current_data_t saej1979_monitor_status 			= {.name = "Monitor Status",     				.pid_code = 0x01, 	.conversion_func = NULL, 						.data_bytes = 4};
static saej1979_current_data_t saej1979_dtc_freeze_frame		= {.name = "DTC Freeze Frame",     				.pid_code = 0x02,  	.conversion_func = NULL, 						.data_bytes = 2};
static saej1979_current_data_t saej1979_fuel_system_status 		= {.name = "Fuel System Status",        		.pid_code = 0x03,   .conversion_func = NULL, 						.data_bytes = 0};
static saej1979_current_data_t saej1979_calculated_engine_load 	= {.name = "Calculated Engine Load",   			.pid_code = 0x04,   .conversion_func = NULL, 						.data_bytes = 0};
static saej1979_current_data_t saej1979_coolant_temp 			= {.name = "Coolant Temp",        				.pid_code = 0x05,   .conversion_func = convert_coolant_temp, 		.data_bytes = 1};
static saej1979_current_data_t saej1979_short_term_fuel_trim_1 	= {.name = "Short Term Fuel Trim 1",   			.pid_code = 0x06,   .conversion_func = NULL, 						.data_bytes = 0};
static saej1979_current_data_t saej1979_long_term_fuel_trim_1 	= {.name = "Long Term Fuel Trim 1",    			.pid_code = 0x07,   .conversion_func = NULL, 						.data_bytes = 0};
static saej1979_current_data_t saej1979_short_term_fuel_trim_2 	= {.name = "Short Term Fuel Trim 2",   			.pid_code = 0x08,   .conversion_func = NULL, 						.data_bytes = 0};
static saej1979_current_data_t saej1979_long_term_fuel_trim_2	= {.name = "Long Term Fuel Trim 2",    			.pid_code = 0x09,   .conversion_func = NULL, 						.data_bytes = 0};
static saej1979_current_data_t saej1979_gauge_fuel_pressure 	= {.name = "Gauge Fuel Pressure",       		.pid_code = 0x0A,   .conversion_func = convert_gauge_fuel_pressure, .data_bytes = 1};
static saej1979_current_data_t saej1979_intake_air_pressure 	= {.name = "Intake Air Temp",     				.pid_code = 0x0B,   .conversion_func = NULL, 						.data_bytes = 1};
static saej1979_current_data_t saej1979_engine_rpm 				= {.name = "Engine RPM",  	   					.pid_code = 0x0C,   .conversion_func = NULL, 						.data_bytes = 1};
static saej1979_current_data_t saej1979_vehicle_speed 			= {.name = "Vehicle Speed",     				.pid_code = 0x0D,   .conversion_func = NULL, 						.data_bytes = 1};
static saej1979_current_data_t saej1979_timing_advance 			= {.name = "Timing Advance",      				.pid_code = 0x0E,   .conversion_func = convert_timing_advance, 		.data_bytes = 1};
static saej1979_current_data_t saej1979_intake_air_temp 		= {.name = "Intake Air Temp",     				.pid_code = 0x0F,   .conversion_func = convert_intake_air_temp, 	.data_bytes = 1};
static saej1979_current_data_t saej1979_maf_flow_rate 			= {.name = "MAF Flow Rate",       				.pid_code = 0x10,   .conversion_func = convert_maf_flow_rate, 		.data_bytes = 2};
static saej1979_current_data_t saej1979_throttle_position 		= {.name = "Throttle Position",       			.pid_code = 0x11,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_secondary_air_status	= {.name = "Secondary Air Status",       		.pid_code = 0x12,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_oxy_sensors_present		= {.name = "Oxygen Sensors Present 2 Banks",    .pid_code = 0x13,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_oxy_sensor_1 			= {.name = "Oxygen Sensor 1",       			.pid_code = 0x14,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_oxy_sensor_2			= {.name = "Oxygen Sensor 2",       			.pid_code = 0x15,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_oxy_sensor_3 			= {.name = "Oxygen Sensor 3",       			.pid_code = 0x16,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_oxy_sensor_4 			= {.name = "Oxygen Sensor 4",       			.pid_code = 0x17,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_oxy_sensor_5 			= {.name = "Oxygen Sensor 5",       			.pid_code = 0x18,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_oxy_sensor_6 			= {.name = "Oxygen Sensor 6",       			.pid_code = 0x19,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_oxy_sensor_7 			= {.name = "Oxygen Sensor 7",       			.pid_code = 0x1A,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_oxy_sensor_8 			= {.name = "Oxygen Sensor 8",       			.pid_code = 0x1B,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_obd_standards 			= {.name = "OBD Standards",       				.pid_code = 0x1C,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_oxy_sensors_present_2	= {.name = "Oxygen Sensors Present 4 Banks",    .pid_code = 0x1D,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_aux_input_stats			= {.name = "Auxiliary Input Status",       		.pid_code = 0x1E,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_runtime					= {.name = "Runtime Since Engine Start",       	.pid_code = 0x1F,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_pids_supported_2		= {.name = "PIDs Supported 2",		       		.pid_code = 0x20,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_dist_travel_w_mil		= {.name = "Distance With MIL On",       		.pid_code = 0x21,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_fuel_rail_pressure 		= {.name = "Fuel Rail Pressure",  				.pid_code = 0x22,   .conversion_func = convert_fuel_rail_pressure, 	.data_bytes = 2};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x21,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x22,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x23,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x24,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x25,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x26,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x27,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x28,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x29,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x2A,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x2B,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x2C,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x2D,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x2E,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x2F,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x30,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x31,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x32,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x33,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_air_fuel_ratio 			= {.name = "Air/Fuel Ratio",      		.pid_code = 0x34,   .conversion_func = convert_air_fuel_ratio, 		.data_bytes = 2};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x35,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x36,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x37,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x38,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x39,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x3A,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x3B,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x3C,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x3D,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x3E,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x3F,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x40,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x41,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x42,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x43,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x44,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x45,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x46,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x47,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x48,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x49,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x4A,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x4B,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x4C,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x4D,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x4E,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x4F,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x51,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x52,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x53,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x54,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x55,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x56,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x57,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x58,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x59,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x5a,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x5b,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x5c,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x5d,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x5e,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x5f,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x61,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x62,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x63,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x64,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x65,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x66,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x67,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x68,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x69,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x6a,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x6b,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x6c,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x6d,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x6e,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x6f,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x71,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x72,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x73,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x74,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x75,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x76,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x77,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x78,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x79,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x7a,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x7b,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x7c,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x7d,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x7e,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x7f,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x81,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x82,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x83,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x84,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x85,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x86,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x87,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x88,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x89,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x8a,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x8b,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x8c,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x8d,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x8e,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x8f,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x90,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x91,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x92,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x93,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x94,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x95,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x96,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x97,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x98,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x99,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x9a,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x9b,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x9c,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x9d,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x9e,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0x9f,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0xa0,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0xa1,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0xa2,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0xa3,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0xa4,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0xa5,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0xa6,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0xa7,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0xa8,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0xa9,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0xaa,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0xab,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0xac,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0xad,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0xae,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0xaf,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0xb0,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0xb1,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0xb2,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0xb3,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0xb4,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0xb5,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0xb6,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0xb7,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0xb8,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0xb9,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0xba,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0xbb,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0xbc,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0xbd,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0xbe,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0xbf,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0xc0,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0xc1,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0xc2,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0xc3,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0xc4,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0xc5,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0xc6,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0xc7,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ 			= {.name = "",       		.pid_code = 0xc8,   .conversion_func = NULL, 		.data_bytes = 0};



saej1979_current_data_t* saej1979_current_data_arr[] = {
&saej1979_monitor_status 			,
&saej1979_dtc_freeze_frame			,
&saej1979_fuel_system_status 		,
&saej1979_calculated_engine_load 	,
&saej1979_coolant_temp,
&saej1979_gauge_fuel_pressure,
>>>>>>> Stashed changes
&saej1979_intake_air_temp,
&saej1979_timing_advance,
&saej1979_intake_air_temp,
&saej1979_maf_flow_rate,
&saej1979_fuel_rail_pressure,
&saej1979_air_fuel_ratio};

<<<<<<< Updated upstream
=======

>>>>>>> Stashed changes
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
<<<<<<< Updated upstream
=======

>>>>>>> Stashed changes
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
<<<<<<< Updated upstream
=======

>>>>>>> Stashed changes
