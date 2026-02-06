/**********     INCLUDES        **********/
#include <cangauge_common.h>
#include "saej1979.h"
#include "string.h"
/**********		DEFINES		**********/

/**********		EXTERNAL VARIABLE DEFINITIONS		**********/

/**********		STATIC FUNCTION DECLRATIONS		**********/
static int32_t convert_coolant_temp(uint32_t val) { return val - 40; };
static int32_t convert_fuel_pressure(uint32_t val) { return val * 3;};
static int32_t convert_gauge_fuel_pressure(uint32_t val) { return val * 3;};
static int32_t convert_intake_air_pressure(uint32_t val) { return val; };
static int32_t convert_timing_advance(uint32_t val) { return (val / 2) - 64; };
static int32_t convert_intake_air_temp(uint32_t val) { return val - 40; };
static int32_t convert_maf_flow_rate(uint32_t val) { return val / 100; };
static int32_t convert_fuel_rail_pressure(uint32_t val);
static int32_t convert_air_fuel_ratio(uint32_t val);

/**********		STATIC VARIABLES		**********/
const char celsius[] = "C";
const char kpa[] = "kPa";
const char lambda[] = "Equiv. Ratio";
const char rpm[] = "RPM";
const char degrees[] = "Degrees";
const char percent[] = "%";
const char psi[] = "PSI";
const char volts[] = "Volts";

static saej1979_current_data_t saej1979_available_pids_1 									= {.name = "Available PIDs 1",     				.pid_code = 0x00, 	.scale = 0, 		.offset = 0, 	.data_bytes = 4,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_monitor_status 										= {.name = "Monitor Status",     				.pid_code = 0x01, 	.scale = 0, 		.offset = 0, 	.data_bytes = 4,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_dtc_freeze_frame									= {.name = "DTC Freeze Frame",     				.pid_code = 0x02,  	.scale = 0, 		.offset = 0, 	.data_bytes = 2,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_fuel_system_status 									= {.name = "Fuel System Status",        		.pid_code = 0x03,   .scale = 0, 		.offset = 0, 	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_calculated_engine_load 								= {.name = "Calculated Engine Load",   			.pid_code = 0x04,   .scale = 0, 		.offset = 0, 	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_coolant_temp 										= {.name = "Engine Coolant Temp",  				.pid_code = 0x05,   .scale = 1, 		.offset = -40, 	.data_bytes = 1,	.first_byte = 0,	.min = -40,	.max = 216,		.units = &celsius,	.available = true,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_short_term_fuel_trim_1 								= {.name = "Short Term Fuel Trim 1",   			.pid_code = 0x06,   .scale = 0.78125, 	.offset = -100, .data_bytes = 1,	.first_byte = 0,	.min = -100,.max = 100,		.units = &percent,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_long_term_fuel_trim_1 								= {.name = "Long Term Fuel Trim 1",    			.pid_code = 0x07,   .scale = 0.78125, 	.offset = -100, .data_bytes = 1,	.first_byte = 0,	.min = -100,.max = 100,		.units = &percent,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_short_term_fuel_trim_2 								= {.name = "Short Term Fuel Trim 2",   			.pid_code = 0x08,   .scale = 0.78125, 	.offset = -100, .data_bytes = 1,	.first_byte = 0,	.min = -100,.max = 100,		.units = &percent,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_long_term_fuel_trim_2								= {.name = "Long Term Fuel Trim 2",    			.pid_code = 0x09,   .scale = 0.78125, 	.offset = -100, .data_bytes = 1,	.first_byte = 0,	.min = -100,.max = 100,		.units = &percent,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_gauge_fuel_pressure 								= {.name = "Fuel Pressure (gauge)",       		.pid_code = 0x0A,   .scale = 3, 		.offset = 0, 	.data_bytes = 1,	.first_byte = 0,	.min = 0,	.max = 768,		.units = &kpa,		.available = true,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_intake_air_pressure 								= {.name = "Abs Intake Manifold Presssure",		.pid_code = 0x0B,   .scale = 1, 		.offset = 0, 	.data_bytes = 1,	.first_byte = 0,	.min = 0,	.max = 256,		.units = &kpa,		.available = true,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_engine_rpm 											= {.name = "Engine RPM",  	   					.pid_code = 0x0C,   .scale = 0.25, 		.offset = 0, 	.data_bytes = 2,	.first_byte = 0,	.min = 0,	.max = 10000,	.units = &rpm,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_vehicle_speed 										= {.name = "Vehicle Speed",     				.pid_code = 0x0D,   .scale = 0, 		.offset = 0, 	.data_bytes = 1,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_timing_advance 										= {.name = "Timing Advance",      				.pid_code = 0x0E,   .scale = .5, 		.offset = -64, 	.data_bytes = 1,	.first_byte = 0,	.min = -64,	.max = 64,		.units = &degrees,	.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_intake_air_temp 									= {.name = "Intake Air Temp",     				.pid_code = 0x0F,   .scale = 0, 		.offset = 0, 	.data_bytes = 1,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_maf_flow_rate 										= {.name = "MAF Flow Rate",       				.pid_code = 0x10,   .scale = 0, 		.offset = 0, 	.data_bytes = 2,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_throttle_position 									= {.name = "Throttle Position",       			.pid_code = 0x11,   .scale = 0, 		.offset = 0, 	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_secondary_air_status								= {.name = "Secondary Air Status",       		.pid_code = 0x12,   .scale = 0, 		.offset = 0, 	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_o2_sensors_present									= {.name = "Oxygen Sensors Present 2 Banks",    .pid_code = 0x13,   .scale = 0, 		.offset = 0, 	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_o2_sensor_b1_s1 									= {.name = "Short Term Fuel Trim B1 S1",       	.pid_code = 0x14,   .scale = .78125,	.offset = -100,	.data_bytes = 1,	.first_byte = 1,	.min = -100,.max = 100,		.units = &percent,	.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_o2_sensor_b1_s2										= {.name = "Short Term Fuel Trim B1 S2",       	.pid_code = 0x15,   .scale = .78125,	.offset = -100,	.data_bytes = 1,	.first_byte = 1,	.min = -100,.max = 100,		.units = &percent,	.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_o2_sensor_b1_s3 									= {.name = "Short Term Fuel Trim B1 S3",       	.pid_code = 0x16,   .scale = .78125,	.offset = -100,	.data_bytes = 1,	.first_byte = 1,	.min = -100,.max = 100,		.units = &percent,	.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_o2_sensor_b1_s4 									= {.name = "Short Term Fuel Trim B1 S4",       	.pid_code = 0x17,   .scale = .78125,	.offset = -100,	.data_bytes = 1,	.first_byte = 1,	.min = -100,.max = 100,		.units = &percent,	.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_o2_sensor_b2_s1 									= {.name = "Short Term Fuel Trim B2 S1",       	.pid_code = 0x18,   .scale = .78125,	.offset = -100,	.data_bytes = 1,	.first_byte = 1,	.min = -100,.max = 100,		.units = &percent,	.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_o2_sensor_b2_s2 									= {.name = "Short Term Fuel Trim B2 S2",       	.pid_code = 0x19,   .scale = .78125,	.offset = -100,	.data_bytes = 1,	.first_byte = 1,	.min = -100,.max = 100,		.units = &percent,	.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_o2_sensor_b2_s3 									= {.name = "Short Term Fuel Trim B2 S3",       	.pid_code = 0x1A,   .scale = .78125,	.offset = -100,	.data_bytes = 1,	.first_byte = 1,	.min = -100,.max = 100,		.units = &percent,	.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_o2_sensor_b2_s4 									= {.name = "Short Term Fuel Trim B2 S4",       	.pid_code = 0x1B,   .scale = .78125,	.offset = -100,	.data_bytes = 1,	.first_byte = 1,	.min = -100,.max = 100,		.units = &percent,	.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_obd_standards 										= {.name = "OBD Standards",       				.pid_code = 0x1C,   .scale = 0, 		.offset = 0, 	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_oxy_sensors_present_2								= {.name = "Oxygen Sensors Present 4 Banks",    .pid_code = 0x1D,   .scale = 0, 		.offset = 0, 	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_aux_input_stats										= {.name = "Auxiliary Input Status",       		.pid_code = 0x1E,   .scale = 0, 		.offset = 0, 	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_runtime												= {.name = "Runtime Since Engine Start",       	.pid_code = 0x1F,   .scale = 0, 		.offset = 0, 	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_pids_supported_2									= {.name = "PIDs Supported 2",		       		.pid_code = 0x20,   .scale = 0, 		.offset = 0, 	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_dist_travel_w_mil									= {.name = "Distance With MIL On",       		.pid_code = 0x21,   .scale = 0, 		.offset = 0, 	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_fuel_pres_to_manifold_vacuum 						= {.name = "Fuel Rail Pres. To Manifold",  		.pid_code = 0x22,   .scale = 0, 		.offset = 0, 	.data_bytes = 2,	.first_byte = 0,	.min = 0,	.max = 0,		.units = &kpa,		.available = false,		.processing_func = NULL	}; //Fuel rail pressure relative to manifold vacuum
static saej1979_current_data_t saej1979_fuel_rail_pressure 									= {.name = "Fuel Rail Pressure",   				.pid_code = 0x23,   .scale = 1.450377, 	.offset = 0, 	.data_bytes = 2,	.first_byte = 0,	.min = 0,	.max = 100000,	.units = &psi,		.available = false,		.processing_func = NULL	}; //Absolute fuel rail pressure
static saej1979_current_data_t saej1979_wide_o2_ratio_volt_b1_s1							= {.name = "O2 Ratio B1 S1",       				.pid_code = 0x24,   .scale = .0000305,	.offset = 0, 	.data_bytes = 2,	.first_byte = 0,	.min = 0,	.max = 2,		.units = &lambda,	.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_wide_o2_ratio_volt_b1_s2							= {.name = "O2 Ratio B1 S2",					.pid_code = 0x25,   .scale = .0000305,	.offset = 0, 	.data_bytes = 2,	.first_byte = 0,	.min = 0,	.max = 2,		.units = &lambda,	.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_wide_o2_ratio_volt_b1_s3 							= {.name = "O2 Ratio B1 S3",					.pid_code = 0x26,   .scale = .0000305,	.offset = 0, 	.data_bytes = 2,	.first_byte = 0,	.min = 0,	.max = 2,		.units = &lambda,	.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_wide_o2_ratio_volt_b1_s4 							= {.name = "O2 Ratio B1 S4",					.pid_code = 0x27,   .scale = .0000305,	.offset = 0, 	.data_bytes = 2,	.first_byte = 0,	.min = 0,	.max = 2,		.units = &lambda,	.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_wide_o2_ratio_volt_b2_s1 							= {.name = "O2 Ratio B2 S1",					.pid_code = 0x28,   .scale = .0000305,	.offset = 0, 	.data_bytes = 2,	.first_byte = 0,	.min = 0,	.max = 2,		.units = &lambda,	.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_wide_o2_ratio_volt_b2_s2 							= {.name = "O2 Ratio B2 S2",					.pid_code = 0x29,   .scale = .0000305,	.offset = 0, 	.data_bytes = 2,	.first_byte = 0,	.min = 0,	.max = 2,		.units = &lambda,	.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_wide_o2_ratio_volt_b2_s3 							= {.name = "O2 Ratio B2 S3",					.pid_code = 0x2A,   .scale = .0000305,	.offset = 0, 	.data_bytes = 2,	.first_byte = 0,	.min = 0,	.max = 2,		.units = &lambda,	.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_wide_o2_ratio_volt_b2_s4 							= {.name = "O2 Ratio B2 S4",					.pid_code = 0x2B,   .scale = .0000305,	.offset = 0, 	.data_bytes = 2,	.first_byte = 0,	.min = 0,	.max = 2,		.units = &lambda,	.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_commanded_egr 										= {.name = "",       							.pid_code = 0x2C,   .scale = 0, 		.offset = 0, 	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_egr_error 											= {.name = "",       							.pid_code = 0x2D,   .scale = 0, 		.offset = 0, 	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_commanded_evap_purge 								= {.name = "",       							.pid_code = 0x2E,   .scale = 0, 		.offset = 0, 	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_fuel_level_input 									= {.name = "",       							.pid_code = 0x2F,   .scale = 0, 		.offset = 0, 	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_num_warmups_since_dtc_cleared 						= {.name = "",       							.pid_code = 0x30,   .scale = 0, 		.offset = 0, 	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_dist_traveled_since_dtc_cleared 					= {.name = "",       							.pid_code = 0x31,   .scale = 0, 		.offset = 0, 	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_evap_system_vapor_pressure 							= {.name = "",       							.pid_code = 0x32,   .scale = 0, 		.offset = 0, 	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_barometric_pressure 								= {.name = "Barometric Pressure", 				.pid_code = 0x33,   .scale = 1, 		.offset = 0, 	.data_bytes = 1,	.first_byte = 0,	.min = 0,	.max = 255,		.units = &kpa,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_wide_o2_ratio_curr_b1_s1 							= {.name = "O2 Ratio B1 S1",					.pid_code = 0x34,   .scale = .0000305,	.offset = 0, 	.data_bytes = 2,	.first_byte = 0,	.min = 0,	.max = 2,		.units = &lambda,	.available = true,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_wide_o2_ratio_curr_b1_s2 							= {.name = "O2 Ratio B1 S2",					.pid_code = 0x35,   .scale = .0000305,	.offset = 0, 	.data_bytes = 2,	.first_byte = 0,	.min = 0,	.max = 2,		.units = &lambda,	.available = true,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_wide_o2_ratio_curr_b1_s3 							= {.name = "O2 Ratio B1 S3",					.pid_code = 0x36,   .scale = .0000305,	.offset = 0, 	.data_bytes = 2,	.first_byte = 0,	.min = 0,	.max = 2,		.units = &lambda,	.available = true,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_wide_o2_ratio_curr_b1_s4 							= {.name = "O2 Ratio B1 S4",					.pid_code = 0x37,   .scale = .0000305,	.offset = 0, 	.data_bytes = 2,	.first_byte = 0,	.min = 0,	.max = 2,		.units = &lambda,	.available = true,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_wide_o2_ratio_curr_b2_s1 							= {.name = "O2 Ratio B2 S1",					.pid_code = 0x38,   .scale = .0000305,	.offset = 0, 	.data_bytes = 2,	.first_byte = 0,	.min = 0,	.max = 2,		.units = &lambda,	.available = true,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_wide_o2_ratio_curr_b2_s2 							= {.name = "O2 Ratio B2 S2",					.pid_code = 0x39,   .scale = .0000305,	.offset = 0, 	.data_bytes = 2,	.first_byte = 0,	.min = 0,	.max = 2,		.units = &lambda,	.available = true,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_wide_o2_ratio_curr_b2_s3 							= {.name = "O2 Ratio B2 S3",					.pid_code = 0x3A,   .scale = .0000305,	.offset = 0, 	.data_bytes = 2,	.first_byte = 0,	.min = 0,	.max = 2,		.units = &lambda,	.available = true,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_wide_o2_ratio_curr_b2_s4 							= {.name = "O2 Ratio B2 S4",					.pid_code = 0x3B,   .scale = .0000305,	.offset = 0, 	.data_bytes = 2,	.first_byte = 0,	.min = 0,	.max = 2,		.units = &lambda,	.available = true,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_catalyst_temp_b1_s1 								= {.name = "Catalyst Temp B1 S1",				.pid_code = 0x3C,   .scale = 0.1,		.offset = -40,	.data_bytes = 2,	.first_byte = 0,	.min = 0,	.max = 6513.5,	.units = &celsius,	.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_catalyst_temp_b1_s2 								= {.name = "Catalyst Temp B1 S2",				.pid_code = 0x3D,   .scale = 0.1,		.offset = -40,	.data_bytes = 2,	.first_byte = 0,	.min = 0,	.max = 6513.5,	.units = &celsius,	.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_catalyst_temp_b2_s1 								= {.name = "Catalyst Temp B2 S1",				.pid_code = 0x3E,   .scale = 0.1,		.offset = -40,	.data_bytes = 2,	.first_byte = 0,	.min = 0,	.max = 6513.5,	.units = &celsius,	.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_catalyst_temp_b2_s2 								= {.name = "Catalyst Temp B2 S2",				.pid_code = 0x3F,   .scale = 0.1,		.offset = -40,	.data_bytes = 2,	.first_byte = 0,	.min = 0,	.max = 6513.5,	.units = &celsius,	.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_available_pids_2 									= {.name = "",       							.pid_code = 0x40,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_monitor_status_2									= {.name = "",       							.pid_code = 0x41,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	}; //Dupe
static saej1979_current_data_t saej1979_control_module_voltage 								= {.name = "Control Module Voltage",			.pid_code = 0x42,   .scale = 0.001,		.offset = 0,	.data_bytes = 2,	.first_byte = 0,	.min = 0,	.max = 20,		.units = &volts,	.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_absolute_load_value 								= {.name = "",       							.pid_code = 0x43,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_commanded_air_fuel_ratio 							= {.name = "Air/Fuel Cmd Eqiv Ratio", 			.pid_code = 0x44,   .scale = .0000305,	.offset = 0,	.data_bytes = 2,	.first_byte = 0,	.min = 0,	.max = 2,		.units = &lambda,	.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_relative_throttle_position 							= {.name = "",       							.pid_code = 0x45,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_ambient_air_temperature 							= {.name = "Ambient Air Temp", 					.pid_code = 0x46,   .scale = 1,			.offset = -40,	.data_bytes = 1,	.first_byte = 0,	.min = -40,	.max = 215,		.units = &celsius,	.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_absolute_throttle_pos_b 							= {.name = "",       							.pid_code = 0x47,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_absolute_throttle_pos_c 							= {.name = "",       							.pid_code = 0x48,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_accelerator_pedal_pos_d 							= {.name = "",       							.pid_code = 0x49,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_accelerator_pedal_pos_e 							= {.name = "",       							.pid_code = 0x4A,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_accelerator_pedal_pos_f 							= {.name = "",       							.pid_code = 0x4B,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_commanded_throttle_actuator_control 				= {.name = "",       							.pid_code = 0x4C,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_engine_runtime_w_mil 								= {.name = "",       							.pid_code = 0x4D,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_engine_runtime_since_dtc_cleared 					= {.name = "",       							.pid_code = 0x4E,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_external_test_equip_config_1 						= {.name = "",       							.pid_code = 0x4F,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_external_test_equip_config_2 						= {.name = "",       							.pid_code = 0x50,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_type_of_fuel_used 									= {.name = "",       							.pid_code = 0x51,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_alcohol_fuel_percentage 							= {.name = "",       							.pid_code = 0x52,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_evap_sys_absolute_pressure 							= {.name = "",       							.pid_code = 0x53,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_evap_sys_pressure 									= {.name = "",       							.pid_code = 0x54,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_short_term_fuel_trim_b1 							= {.name = "",       							.pid_code = 0x55,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_long_term_fuel_trim_b1 								= {.name = "",       							.pid_code = 0x56,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_short_term_fuel_trim_b2 							= {.name = "",       							.pid_code = 0x57,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_long_term_fuel_trim_b2 								= {.name = "",       							.pid_code = 0x58,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_fuel_rail_pressure_absolute 						= {.name = "",       							.pid_code = 0x59,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_relative_accelerator_pedal_position 				= {.name = "",       							.pid_code = 0x5a,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_hybrid_battery_pack_charge 							= {.name = "",       							.pid_code = 0x5b,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_engine_oil_temp 									= {.name = "",       							.pid_code = 0x5c,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_fuel_injection_timing 								= {.name = "",       							.pid_code = 0x5d,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_engine_fuel_rate 									= {.name = "",       							.pid_code = 0x5e,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_emission_requirements 								= {.name = "",       							.pid_code = 0x5f,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_available_pids_3									= {.name = "",       							.pid_code = 0x60,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_driver_commanded_engine_tq_percentage 				= {.name = "",       							.pid_code = 0x61,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_actual_engine_tq_percentage 						= {.name = "",       							.pid_code = 0x62,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_engine_ref_tq 										= {.name = "",       							.pid_code = 0x63,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_engine_percent_tq_data 								= {.name = "",       							.pid_code = 0x64,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_aux_io_status 										= {.name = "",       							.pid_code = 0x65,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_maf_sensor 											= {.name = "",       							.pid_code = 0x66,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_engine_coolant_temp 								= {.name = "",       							.pid_code = 0x67,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_intake_air_temp_2 									= {.name = "",       							.pid_code = 0x68,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	}; //Dupe
static saej1979_current_data_t saej1979_egr 												= {.name = "",       							.pid_code = 0x69,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_diesel_air_intake 									= {.name = "",       							.pid_code = 0x6a,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_exhaust_gas_recirc_temp 							= {.name = "",       							.pid_code = 0x6b,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_throttle_ctrl_and_pos 								= {.name = "",       							.pid_code = 0x6c,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_fuel_pres_ctrl_sys 									= {.name = "",       							.pid_code = 0x6d,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_injection_pres_ctrl_sys 							= {.name = "",       							.pid_code = 0x6e,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_turbo_inlet_pressure 								= {.name = "",       							.pid_code = 0x6f,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_boost_pressure_control 								= {.name = "Boost Pressure",					.pid_code = 0x70,   .scale = .0045324,	.offset = 0,	.data_bytes = 2,	.first_byte = 7,	.min = 0,	.max = 30,		.units = &psi,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_variable_geometry_turbo_control 					= {.name = "",       							.pid_code = 0x71,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_wastegate_control 									= {.name = "",       							.pid_code = 0x72,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_exhaust_pressure 									= {.name = "",       							.pid_code = 0x73,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_turbo_rpm 											= {.name = "",       							.pid_code = 0x74,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_turbo_temp_a 										= {.name = "",       							.pid_code = 0x75,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_turbo_temp_b 										= {.name = "",       							.pid_code = 0x76,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_charge_air_cooler_temp 								= {.name = "",       							.pid_code = 0x77,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_exhaust_gas_temp_b1 								= {.name = "",       							.pid_code = 0x78,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_exhaust_gas_temp_b2 								= {.name = "",       							.pid_code = 0x79,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_particulate_filter_b1 								= {.name = "",       							.pid_code = 0x7a,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_particulate_filter_b2 								= {.name = "",       							.pid_code = 0x7b,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_particulate_filter_temp 							= {.name = "",       							.pid_code = 0x7c,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_nox_nte_control_status								= {.name = "",       							.pid_code = 0x7d,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_pm_nte_control_status 								= {.name = "",       							.pid_code = 0x7e,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_engine_runtime 										= {.name = "",       							.pid_code = 0x7f,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_available_pids_4 									= {.name = "",       							.pid_code = 0x80,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_engine_runtime_aecd_1_5 							= {.name = "",       							.pid_code = 0x81,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_engine_runtime_aecd_6_10 							= {.name = "",       							.pid_code = 0x82,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_nox_sensor 											= {.name = "",       							.pid_code = 0x83,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_manifold_surface_temp 								= {.name = "",       							.pid_code = 0x84,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_nox_control_system 									= {.name = "",       							.pid_code = 0x85,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_particulate_matter_sensor 							= {.name = "",       							.pid_code = 0x86,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_intake_manifold_abs_pres 							= {.name = "",       							.pid_code = 0x87,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_scr_inducement_sys 									= {.name = "",       							.pid_code = 0x88,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_engine_runtime_aecd_11_15 							= {.name = "",       							.pid_code = 0x89,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_engine_runtime_aecd_16_20 							= {.name = "",       							.pid_code = 0x8a,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_aftertreatment_status 								= {.name = "",       							.pid_code = 0x8b,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_o2_sensor 											= {.name = "",       							.pid_code = 0x8c,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_abs_throttle_pos_g									= {.name = "",       							.pid_code = 0x8d,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_engine_friction_percent_tq 							= {.name = "",       							.pid_code = 0x8e,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_particulate_matter 									= {.name = "",       							.pid_code = 0x8f,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_obd_sys_info 										= {.name = "",       							.pid_code = 0x90,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_obd_ecu_sys_info 									= {.name = "",       							.pid_code = 0x91,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_fuel_sys_status 									= {.name = "",       							.pid_code = 0x92,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_obd_counters 										= {.name = "",       							.pid_code = 0x93,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_nox_control 										= {.name = "",       							.pid_code = 0x94,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_scr_catalyst 										= {.name = "",       							.pid_code = 0x95,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_hydrocarbon_doser 									= {.name = "",       							.pid_code = 0x96,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_nox_mass_emission_rate 								= {.name = "",       							.pid_code = 0x97,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_exhaust_gas_temp_b1_2 								= {.name = "",       							.pid_code = 0x98,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};	//Dupe
static saej1979_current_data_t saej1979_exhaust_gas_temp_b2_2								= {.name = "",       							.pid_code = 0x99,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	}; //Dupe
static saej1979_current_data_t saej1979_hybrid_ev_sys_data 									= {.name = "",       							.pid_code = 0x9a,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_def_sensor_output 									= {.name = "",       							.pid_code = 0x9b,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_o2_sensor_2 										= {.name = "",       							.pid_code = 0x9c,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_fuel_rate 											= {.name = "",       							.pid_code = 0x9d,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_engine_exhaust_flow_rate 							= {.name = "",       							.pid_code = 0x9e,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_fuel_system_percentage_use 							= {.name = "",       							.pid_code = 0x9f,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_available_pids_5 									= {.name = "",       							.pid_code = 0xa0,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_nox_sensor_corrected 								= {.name = "",       							.pid_code = 0xa1,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_cylinder_fuel_rate 									= {.name = "",       							.pid_code = 0xa2,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_evap_system_vapor_pressure_2 						= {.name = "",       							.pid_code = 0xa3,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	}; //Dupe
static saej1979_current_data_t saej1979_transmission_actual_gear 							= {.name = "",       							.pid_code = 0xa4,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_def_dosing 											= {.name = "",       							.pid_code = 0xa5,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_odometer 											= {.name = "",       							.pid_code = 0xa6,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_nox_sensor_2 										= {.name = "",       							.pid_code = 0xa7,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_nox_sensor_corrected_2 								= {.name = "",       							.pid_code = 0xa8,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_motorcycle_io_status 								= {.name = "",       							.pid_code = 0xa9,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_speed_limiter_set 									= {.name = "",       							.pid_code = 0xaa,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_alternate_fuel_vehicle_data 						= {.name = "",       							.pid_code = 0xab,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_max_def_dosing 										= {.name = "",       							.pid_code = 0xac,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_crankcase_ventilation_data 							= {.name = "",       							.pid_code = 0xad,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_evap_system_purge_sensor 							= {.name = "",       							.pid_code = 0xae,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_commanded_fresh_air_flow 							= {.name = "",       							.pid_code = 0xaf,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_egr_mass_flow 										= {.name = "",       							.pid_code = 0xb0,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_compression_ignition_fuel_sys 						= {.name = "",       							.pid_code = 0xb1,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};	//TODO: this one
static saej1979_current_data_t saej1979_traction_battery_pack_health 						= {.name = "",       							.pid_code = 0xb2,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_hvesp_actual_charge_rate 							= {.name = "",       							.pid_code = 0xb3,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_hvesp_temp 											= {.name = "",       							.pid_code = 0xb4,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_hvesp_current 										= {.name = "",       							.pid_code = 0xb5,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_hvesp_voltage 										= {.name = "",       							.pid_code = 0xb6,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_hybrid_ev_battery_temp_data 						= {.name = "",       							.pid_code = 0xb7,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_hybrid_ev_battery_hv_sys_time_since_balance 		= {.name = "",       							.pid_code = 0xb8,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_hybrid_ev_battery_min_max_cell_voltage 				= {.name = "",       							.pid_code = 0xb9,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_hybrid_ev_battery_continuous_power 					= {.name = "",       							.pid_code = 0xba,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_energy_into_hv_storage 								= {.name = "",       							.pid_code = 0xbb,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_energy_from_hv_storage 								= {.name = "",       							.pid_code = 0xbc,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_hvess_energy_throughput 							= {.name = "",       							.pid_code = 0xbd,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_hvess_state_of_health  								= {.name = "",       							.pid_code = 0xbe,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_hvess_min_state_of_charge 							= {.name = "",       							.pid_code = 0xbf,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_available_pids_6 									= {.name = "",       							.pid_code = 0xc0,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_hvess_max_state_of_charge 							= {.name = "",       							.pid_code = 0xc1,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_hvess_discharge_energy_capacity 					= {.name = "",       							.pid_code = 0xc2,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_fuel_level_input_2 									= {.name = "",       							.pid_code = 0xc3,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};	//Dupe
static saej1979_current_data_t saej1979_exhaust_particulate_ctrl_sys 						= {.name = "",       							.pid_code = 0xc4,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_low_pres_fuel_sys 									= {.name = "",       							.pid_code = 0xc5,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_particulate_ctrl 									= {.name = "",       							.pid_code = 0xc6,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_dist_since_reflash 									= {.name = "",       							.pid_code = 0xc7,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_ncd_and_pcd_warning_status 							= {.name = "",       							.pid_code = 0xc8,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_o2_sensor_3 										= {.name = "",       							.pid_code = 0xc9,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_reserved	 										= {.name = "",       							.pid_code = 0xca,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_exhuast_pressure 									= {.name = "",       							.pid_code = 0xcb,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_electric_motor_rpm 									= {.name = "",       							.pid_code = 0xcc,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_electric_motor_tq 									= {.name = "",       							.pid_code = 0xcd,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_evap_sys_pressure_2 								= {.name = "",       							.pid_code = 0xce,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_catalyst_temp 										= {.name = "",       							.pid_code = 0xcf,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};


/* Expanded data. */
static saej1979_current_data_t saej1979_boost_pressure_ctrl_support							= {.name = "Boost Pressure Ctrl Support",		.pid_code = 0x70,   .scale = 0,			.offset = 0,	.data_bytes = 0,	.first_byte = 0,	.min = 0,	.max = 0,		.units = NULL,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_commanded_boost_pressure_a 							= {.name = "Cmd Boost Pressure A",   			.pid_code = 0x70,   .scale = .03125,	.offset = 0,	.data_bytes = 2,	.first_byte = 1,	.min = 0,	.max = 2048,	.units = &kpa,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_boost_pressure_sensor_a								= {.name = "Boost Pressure Sensor A",			.pid_code = 0x70,   .scale = .03125,	.offset = 0,	.data_bytes = 2,	.first_byte = 3,	.min = 0,	.max = 2048,	.units = &kpa,		.available = false,		.processing_func = NULL	};
static saej1979_current_data_t saej1979_commanded_boost_pressure_b 							= {.name = "Cmd Boost Pressure B",   			.pid_code = 0x70,   .scale = .03125,	.offset = 0,	.data_bytes = 2,	.first_byte = 5,	.min = 0,	.max = 2048,	.units = &kpa,		.available = false,		.processing_func = NULL	};	//TODO: this one
static saej1979_current_data_t saej1979_boost_pressure_sensor_b		 						= {.name = "Boost Pressure Sensor B",			.pid_code = 0x70,   .scale = .03125,	.offset = 0,	.data_bytes = 2,	.first_byte = 7,	.min = 0,	.max = 2048,	.units = &kpa,		.available = false,		.processing_func = NULL	};


saej1979_current_data_t* saej1979_current_data_arr[] = {
&saej1979_available_pids_1 									,
&saej1979_monitor_status 									,	
&saej1979_dtc_freeze_frame									,
&saej1979_fuel_system_status 								,	
&saej1979_calculated_engine_load 							,	
&saej1979_coolant_temp 										,
&saej1979_short_term_fuel_trim_1 							,	
&saej1979_long_term_fuel_trim_1 								,
&saej1979_short_term_fuel_trim_2 							,	
&saej1979_long_term_fuel_trim_2								,
&saej1979_gauge_fuel_pressure 								,
&saej1979_intake_air_pressure 								,
&saej1979_engine_rpm 										,	
&saej1979_vehicle_speed 										,
&saej1979_timing_advance 									,	
&saej1979_intake_air_temp 									,
&saej1979_maf_flow_rate 										,
&saej1979_throttle_position 									,
&saej1979_secondary_air_status								,
&saej1979_o2_sensors_present									,
&saej1979_o2_sensor_b1_s1 									,
&saej1979_o2_sensor_b1_s2									,	
&saej1979_o2_sensor_b1_s3 									,
&saej1979_o2_sensor_b1_s4 									,
&saej1979_o2_sensor_b2_s1 									,
&saej1979_o2_sensor_b2_s2 									,
&saej1979_o2_sensor_b2_s3 									,
&saej1979_o2_sensor_b2_s4 									,
&saej1979_obd_standards 										,
&saej1979_oxy_sensors_present_2								,
&saej1979_aux_input_stats									,	
&saej1979_runtime											,	
&saej1979_pids_supported_2									,
&saej1979_dist_travel_w_mil									,
&saej1979_fuel_pres_to_manifold_vacuum 						,
&saej1979_fuel_rail_pressure 								,	
&saej1979_wide_o2_ratio_volt_b1_s1							,
&saej1979_wide_o2_ratio_volt_b1_s2							,
&saej1979_wide_o2_ratio_volt_b1_s3 							,
&saej1979_wide_o2_ratio_volt_b1_s4 							,
&saej1979_wide_o2_ratio_volt_b2_s1 							,
&saej1979_wide_o2_ratio_volt_b2_s2 							,
&saej1979_wide_o2_ratio_volt_b2_s3 							,
&saej1979_wide_o2_ratio_volt_b2_s4 							,
&saej1979_commanded_egr 										,
&saej1979_egr_error 											,
&saej1979_commanded_evap_purge 								,
&saej1979_fuel_level_input 									,
&saej1979_num_warmups_since_dtc_cleared 						,
&saej1979_dist_traveled_since_dtc_cleared 					,
&saej1979_evap_system_vapor_pressure 						,	
&saej1979_barometric_pressure 								,
&saej1979_wide_o2_ratio_curr_b1_s1 							,
&saej1979_wide_o2_ratio_curr_b1_s2 							,
&saej1979_wide_o2_ratio_curr_b1_s3 							,
&saej1979_wide_o2_ratio_curr_b1_s4 							,
&saej1979_wide_o2_ratio_curr_b2_s1 							,
&saej1979_wide_o2_ratio_curr_b2_s2 							,
&saej1979_wide_o2_ratio_curr_b2_s3 							,
&saej1979_wide_o2_ratio_curr_b2_s4 							,
&saej1979_catalyst_temp_b1_s1 								,
&saej1979_catalyst_temp_b1_s2 								,
&saej1979_catalyst_temp_b2_s1 								,
&saej1979_catalyst_temp_b2_s2 								,
&saej1979_available_pids_2 									,
&saej1979_monitor_status_2									,
&saej1979_control_module_voltage 							,	
&saej1979_absolute_load_value 								,
&saej1979_commanded_air_fuel_ratio 							,
&saej1979_relative_throttle_position 						,	
&saej1979_ambient_air_temperature 							,
&saej1979_absolute_throttle_pos_b 							,
&saej1979_absolute_throttle_pos_c 							,
&saej1979_accelerator_pedal_pos_d 							,
&saej1979_accelerator_pedal_pos_e 							,
&saej1979_accelerator_pedal_pos_f 							,
&saej1979_commanded_throttle_actuator_control 				,
&saej1979_engine_runtime_w_mil 								,
&saej1979_engine_runtime_since_dtc_cleared 					,
&saej1979_external_test_equip_config_1 						,
&saej1979_external_test_equip_config_2 						,
&saej1979_type_of_fuel_used 									,
&saej1979_alcohol_fuel_percentage 							,
&saej1979_evap_sys_absolute_pressure 						,	
&saej1979_evap_sys_pressure 									,
&saej1979_short_term_fuel_trim_b1 							,
&saej1979_long_term_fuel_trim_b1 							,	
&saej1979_short_term_fuel_trim_b2 							,
&saej1979_long_term_fuel_trim_b2 							,	
&saej1979_fuel_rail_pressure_absolute 						,
&saej1979_relative_accelerator_pedal_position 				,
&saej1979_hybrid_battery_pack_charge 						,	
&saej1979_engine_oil_temp 									,
&saej1979_fuel_injection_timing 								,
&saej1979_engine_fuel_rate 									,
&saej1979_emission_requirements 								,
&saej1979_available_pids_3									,
&saej1979_driver_commanded_engine_tq_percentage 				,
&saej1979_actual_engine_tq_percentage 						,
&saej1979_engine_ref_tq 										,
&saej1979_engine_percent_tq_data 							,	
&saej1979_aux_io_status 										,
&saej1979_maf_sensor 										,	
&saej1979_engine_coolant_temp 								,
&saej1979_intake_air_temp_2 									,
&saej1979_egr 												,
&saej1979_diesel_air_intake 									,
&saej1979_exhaust_gas_recirc_temp 							,
&saej1979_throttle_ctrl_and_pos 								,
&saej1979_fuel_pres_ctrl_sys 								,	
&saej1979_injection_pres_ctrl_sys 							,
&saej1979_turbo_inlet_pressure 								,
&saej1979_boost_pressure_control 							,	
&saej1979_variable_geometry_turbo_control 					,
&saej1979_wastegate_control 									,
&saej1979_exhaust_pressure 									,
&saej1979_turbo_rpm 											,
&saej1979_turbo_temp_a 										,
&saej1979_turbo_temp_b 										,
&saej1979_charge_air_cooler_temp 							,	
&saej1979_exhaust_gas_temp_b1 								,
&saej1979_exhaust_gas_temp_b2 								,
&saej1979_particulate_filter_b1 								,
&saej1979_particulate_filter_b2 								,
&saej1979_particulate_filter_temp 							,
&saej1979_nox_nte_control_status								,
&saej1979_pm_nte_control_status 								,
&saej1979_engine_runtime 									,	
&saej1979_available_pids_4 									,
&saej1979_engine_runtime_aecd_1_5 							,
&saej1979_engine_runtime_aecd_6_10 							,
&saej1979_nox_sensor 										,	
&saej1979_manifold_surface_temp 								,
&saej1979_nox_control_system 								,	
&saej1979_particulate_matter_sensor 							,
&saej1979_intake_manifold_abs_pres 							,
&saej1979_scr_inducement_sys 								,	
&saej1979_engine_runtime_aecd_11_15 							,
&saej1979_engine_runtime_aecd_16_20 							,
&saej1979_aftertreatment_status 								,
&saej1979_o2_sensor 											,
&saej1979_abs_throttle_pos_g									,
&saej1979_engine_friction_percent_tq 						,	
&saej1979_particulate_matter 								,	
&saej1979_obd_sys_info 										,
&saej1979_obd_ecu_sys_info 									,
&saej1979_fuel_sys_status 									,
&saej1979_obd_counters 										,
&saej1979_nox_control 										,
&saej1979_scr_catalyst 										,
&saej1979_hydrocarbon_doser 									,
&saej1979_nox_mass_emission_rate 							,	
&saej1979_exhaust_gas_temp_b1_2 								,
&saej1979_exhaust_gas_temp_b2_2								,
&saej1979_hybrid_ev_sys_data 								,	
&saej1979_def_sensor_output 									,
&saej1979_o2_sensor_2 										,
&saej1979_fuel_rate 											,
&saej1979_engine_exhaust_flow_rate 							,
&saej1979_fuel_system_percentage_use 						,	
&saej1979_available_pids_5 									,
&saej1979_nox_sensor_corrected 								,
&saej1979_cylinder_fuel_rate 								,	
&saej1979_evap_system_vapor_pressure_2 						,
&saej1979_transmission_actual_gear 							,
&saej1979_def_dosing 										,	
&saej1979_odometer 											,
&saej1979_nox_sensor_2 										,
&saej1979_nox_sensor_corrected_2 							,	
&saej1979_motorcycle_io_status 								,
&saej1979_speed_limiter_set 									,
&saej1979_alternate_fuel_vehicle_data 						,
&saej1979_max_def_dosing 									,	
&saej1979_crankcase_ventilation_data 						,	
&saej1979_evap_system_purge_sensor 							,
&saej1979_commanded_fresh_air_flow 							,
&saej1979_egr_mass_flow 										,
&saej1979_compression_ignition_fuel_sys 						,
&saej1979_traction_battery_pack_health 						,
&saej1979_hvesp_actual_charge_rate 							,
&saej1979_hvesp_temp 										,	
&saej1979_hvesp_current 										,
&saej1979_hvesp_voltage 										,
&saej1979_hybrid_ev_battery_temp_data 						,
&saej1979_hybrid_ev_battery_hv_sys_time_since_balance 		,
&saej1979_hybrid_ev_battery_min_max_cell_voltage 			,	
&saej1979_hybrid_ev_battery_continuous_power 				,	
&saej1979_energy_into_hv_storage 							,	
&saej1979_energy_from_hv_storage 							,	
&saej1979_hvess_energy_throughput 							,
&saej1979_hvess_state_of_health  							,	
&saej1979_hvess_min_state_of_charge 							,
&saej1979_available_pids_6 									,
&saej1979_hvess_max_state_of_charge 							,
&saej1979_hvess_discharge_energy_capacity 					,
&saej1979_fuel_level_input_2 								,	
&saej1979_exhaust_particulate_ctrl_sys 						,
&saej1979_low_pres_fuel_sys 									,
&saej1979_particulate_ctrl 									,
&saej1979_dist_since_reflash 								,	
&saej1979_ncd_and_pcd_warning_status 						,	
&saej1979_o2_sensor_3 										,
&saej1979_reserved	 										,
&saej1979_exhuast_pressure 									,
&saej1979_electric_motor_rpm 								,	
&saej1979_electric_motor_tq 									,
&saej1979_evap_sys_pressure_2 								,
&saej1979_catalyst_temp 										,
};

saej1979_current_data_t* saej1979_boost_pressure_ctrl_arr[] = {
&saej1979_boost_pressure_ctrl_support							,
&saej1979_commanded_boost_pressure_a 							,
&saej1979_boost_pressure_sensor_a								,
&saej1979_commanded_boost_pressure_b 							,
&saej1979_boost_pressure_sensor_b		 						,
};

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
void saej1979_current_data_set_getter(uint8_t pid)
{
	/* Data byte 2 needs to be changed depending on the data being requested. */
	static can_tx_buffer_entry_t saej1979_getter_template =
	{
		.T0.bit.ID = 0x7DF << 18, .T0.bit.XTD = CAN_ID_STD, .T0.bit.RTR = CAN_RTR_DATA_FRAME,
		.T1.bit.DLC = 8, .T1.bit.EFC = 0, .T1.bit.BRS = 0, .T1.bit.FDF = 0,
		.data[0] = 0x02, .data[1] = 0x01, .data[2] = 0x00, .data[3] = 0xCC,
		.data[4] = 0xCC, .data[5] = 0xCC, .data[6] = 0xCC, .data[7] = 0xCC,
	};


	saej1979_getter_template.data[2] = pid;
	can_add_tx_buffer(FDCAN1, &saej1979_getter_template, 0);
	common.p_can_transmit_period_list[0] = 25;
	return;

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
			//data = saej1979_current_data_arr[i]->conversion_func(data);
			return data;
		}
	}	
	return 0;
}

saej1979_current_data_t* saej1979_get_current_data(uint8_t pid)
{
	return saej1979_current_data_arr[pid];
}
