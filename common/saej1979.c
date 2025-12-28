/**********     INCLUDES        **********/
#include "saej1979.h"
#include "string.h"
#include "common/app_shared_mem.h"
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
static saej1979_current_data_t saej1979_available_pids_1 									= {.name = "Available PIDs 1",     				.pid_code = 0x00, 	.conversion_func = NULL, 						.data_bytes = 4};
static saej1979_current_data_t saej1979_monitor_status 										= {.name = "Monitor Status",     				.pid_code = 0x01, 	.conversion_func = NULL, 						.data_bytes = 4};
static saej1979_current_data_t saej1979_dtc_freeze_frame									= {.name = "DTC Freeze Frame",     				.pid_code = 0x02,  	.conversion_func = NULL, 						.data_bytes = 2};
static saej1979_current_data_t saej1979_fuel_system_status 									= {.name = "Fuel System Status",        		.pid_code = 0x03,   .conversion_func = NULL, 						.data_bytes = 0};
static saej1979_current_data_t saej1979_calculated_engine_load 								= {.name = "Calculated Engine Load",   			.pid_code = 0x04,   .conversion_func = NULL, 						.data_bytes = 0};
static saej1979_current_data_t saej1979_coolant_temp 										= {.name = "Coolant Temp",        				.pid_code = 0x05,   .conversion_func = convert_coolant_temp, 		.data_bytes = 1};
static saej1979_current_data_t saej1979_short_term_fuel_trim_1 								= {.name = "Short Term Fuel Trim 1",   			.pid_code = 0x06,   .conversion_func = NULL, 						.data_bytes = 0};
static saej1979_current_data_t saej1979_long_term_fuel_trim_1 								= {.name = "Long Term Fuel Trim 1",    			.pid_code = 0x07,   .conversion_func = NULL, 						.data_bytes = 0};
static saej1979_current_data_t saej1979_short_term_fuel_trim_2 								= {.name = "Short Term Fuel Trim 2",   			.pid_code = 0x08,   .conversion_func = NULL, 						.data_bytes = 0};
static saej1979_current_data_t saej1979_long_term_fuel_trim_2								= {.name = "Long Term Fuel Trim 2",    			.pid_code = 0x09,   .conversion_func = NULL, 						.data_bytes = 0};
static saej1979_current_data_t saej1979_gauge_fuel_pressure 								= {.name = "Gauge Fuel Pressure",       		.pid_code = 0x0A,   .conversion_func = convert_gauge_fuel_pressure, .data_bytes = 1};
static saej1979_current_data_t saej1979_intake_air_pressure 								= {.name = "Intake Air Temp",     				.pid_code = 0x0B,   .conversion_func = NULL, 						.data_bytes = 1};
static saej1979_current_data_t saej1979_engine_rpm 											= {.name = "Engine RPM",  	   					.pid_code = 0x0C,   .conversion_func = NULL, 						.data_bytes = 1};
static saej1979_current_data_t saej1979_vehicle_speed 										= {.name = "Vehicle Speed",     				.pid_code = 0x0D,   .conversion_func = NULL, 						.data_bytes = 1};
static saej1979_current_data_t saej1979_timing_advance 										= {.name = "Timing Advance",      				.pid_code = 0x0E,   .conversion_func = convert_timing_advance, 		.data_bytes = 1};
static saej1979_current_data_t saej1979_intake_air_temp 									= {.name = "Intake Air Temp",     				.pid_code = 0x0F,   .conversion_func = convert_intake_air_temp, 	.data_bytes = 1};
static saej1979_current_data_t saej1979_maf_flow_rate 										= {.name = "MAF Flow Rate",       				.pid_code = 0x10,   .conversion_func = convert_maf_flow_rate, 		.data_bytes = 2};
static saej1979_current_data_t saej1979_throttle_position 									= {.name = "Throttle Position",       			.pid_code = 0x11,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_secondary_air_status								= {.name = "Secondary Air Status",       		.pid_code = 0x12,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_oxy_sensors_present									= {.name = "Oxygen Sensors Present 2 Banks",    .pid_code = 0x13,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_oxy_sensor_1 										= {.name = "Oxygen Sensor 1",       			.pid_code = 0x14,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_oxy_sensor_2										= {.name = "Oxygen Sensor 2",       			.pid_code = 0x15,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_oxy_sensor_3 										= {.name = "Oxygen Sensor 3",       			.pid_code = 0x16,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_oxy_sensor_4 										= {.name = "Oxygen Sensor 4",       			.pid_code = 0x17,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_oxy_sensor_5 										= {.name = "Oxygen Sensor 5",       			.pid_code = 0x18,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_oxy_sensor_6 										= {.name = "Oxygen Sensor 6",       			.pid_code = 0x19,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_oxy_sensor_7 										= {.name = "Oxygen Sensor 7",       			.pid_code = 0x1A,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_oxy_sensor_8 										= {.name = "Oxygen Sensor 8",       			.pid_code = 0x1B,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_obd_standards 										= {.name = "OBD Standards",       				.pid_code = 0x1C,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_oxy_sensors_present_2								= {.name = "Oxygen Sensors Present 4 Banks",    .pid_code = 0x1D,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_aux_input_stats										= {.name = "Auxiliary Input Status",       		.pid_code = 0x1E,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_runtime												= {.name = "Runtime Since Engine Start",       	.pid_code = 0x1F,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_pids_supported_2									= {.name = "PIDs Supported 2",		       		.pid_code = 0x20,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_dist_travel_w_mil									= {.name = "Distance With MIL On",       		.pid_code = 0x21,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_fuel_rail_pressure_vac 								= {.name = "Fuel Rail Pressure",  				.pid_code = 0x22,   .conversion_func = convert_fuel_rail_pressure, 	.data_bytes = 2}; //Fuel rail pressure relative to manifold vacuum
static saej1979_current_data_t saej1979_distance_while_mil_is_on							= {.name = "",       		.pid_code = 0x21,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_fuel_pres_to_manifold_vacuum						= {.name = "",       		.pid_code = 0x22,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_fuel_rail_pressure 									= {.name = "",       		.pid_code = 0x23,   .conversion_func = NULL, 		.data_bytes = 0}; //Absolute fuel rail pressure
static saej1979_current_data_t saej1979_wide_o2_ratio_volt_b1_s1							= {.name = "",       		.pid_code = 0x24,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_wide_o2_ratio_volt_b1_s2							= {.name = "",       		.pid_code = 0x25,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_wide_o2_ratio_volt_b1_s3 							= {.name = "",       		.pid_code = 0x26,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_wide_o2_ratio_volt_b1_s4 							= {.name = "",       		.pid_code = 0x27,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_wide_o2_ratio_volt_b2_s1 							= {.name = "",       		.pid_code = 0x28,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_wide_o2_ratio_volt_b2_s2 							= {.name = "",       		.pid_code = 0x29,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_wide_o2_ratio_volt_b2_s3 							= {.name = "",       		.pid_code = 0x2A,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_wide_o2_ratio_volt_b2_s4 							= {.name = "",       		.pid_code = 0x2B,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_commanded_egr 										= {.name = "",       		.pid_code = 0x2C,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_egr_error 											= {.name = "",       		.pid_code = 0x2D,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_commanded_evap_purge 								= {.name = "",       		.pid_code = 0x2E,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_fuel_level_input 									= {.name = "",       		.pid_code = 0x2F,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_num_warmups_since_dtc_cleared 						= {.name = "",       		.pid_code = 0x30,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_dist_traveled_since_dtc_cleared 					= {.name = "",       		.pid_code = 0x31,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_evap_system_vapor_pressure 							= {.name = "",       		.pid_code = 0x32,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_barometric_pressure 								= {.name = "",       		.pid_code = 0x33,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_wide_o2_ratio_curr_b1_s1 							= {.name = "Air/Fuel Ratio",      		.pid_code = 0x34,   .conversion_func = convert_air_fuel_ratio, 		.data_bytes = 2};
static saej1979_current_data_t saej1979_wide_o2_ratio_curr_b1_s2 							= {.name = "",       		.pid_code = 0x35,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_wide_o2_ratio_curr_b1_s3 							= {.name = "",       		.pid_code = 0x36,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_wide_o2_ratio_curr_b1_s4 							= {.name = "",       		.pid_code = 0x37,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_wide_o2_ratio_curr_b2_s1 							= {.name = "",       		.pid_code = 0x38,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_wide_o2_ratio_curr_b2_s2 							= {.name = "",       		.pid_code = 0x39,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_wide_o2_ratio_curr_b2_s3 							= {.name = "",       		.pid_code = 0x3A,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_wide_o2_ratio_curr_b2_s4 							= {.name = "",       		.pid_code = 0x3B,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_catalyst_temp_b1_s1 								= {.name = "",       		.pid_code = 0x3C,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_catalyst_temp_b1_s2 								= {.name = "",       		.pid_code = 0x3D,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_catalyst_temp_b2_s1 								= {.name = "",       		.pid_code = 0x3E,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_catalyst_temp_b2_s2 								= {.name = "",       		.pid_code = 0x3F,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_available_pids_2 									= {.name = "",       		.pid_code = 0x40,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_monitor_status_2									= {.name = "",       		.pid_code = 0x41,   .conversion_func = NULL, 		.data_bytes = 0}; //Dupe
static saej1979_current_data_t saej1979_control_module_voltage 								= {.name = "",       		.pid_code = 0x42,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_absolute_load_value 								= {.name = "",       		.pid_code = 0x43,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_commanded_air_fuel_ratio 							= {.name = "",       		.pid_code = 0x44,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_relative_throttle_position 							= {.name = "",       		.pid_code = 0x45,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ambient_air_temperature 							= {.name = "",       		.pid_code = 0x46,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_absolute_throttle_pos_b 							= {.name = "",       		.pid_code = 0x47,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_absolute_throttle_pos_c 							= {.name = "",       		.pid_code = 0x48,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_accelerator_pedal_pos_d 							= {.name = "",       		.pid_code = 0x49,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_accelerator_pedal_pos_e 							= {.name = "",       		.pid_code = 0x4A,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_accelerator_pedal_pos_f 							= {.name = "",       		.pid_code = 0x4B,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_commanded_throttle_actuator_control 				= {.name = "",       		.pid_code = 0x4C,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_engine_runtime_w_mil 								= {.name = "",       		.pid_code = 0x4D,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_engine_runtime_since_dtc_cleared 					= {.name = "",       		.pid_code = 0x4E,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_external_test_equip_config_1 						= {.name = "",       		.pid_code = 0x4F,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_external_test_equip_config_2 						= {.name = "",       		.pid_code = 0x50,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_type_of_fuel_used 									= {.name = "",       		.pid_code = 0x51,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_alcohol_fuel_percentage 							= {.name = "",       		.pid_code = 0x52,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_evap_sys_absolute_pressure 							= {.name = "",       		.pid_code = 0x53,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_evap_sys_pressure 									= {.name = "",       		.pid_code = 0x54,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_short_term_fuel_trim_b1 							= {.name = "",       		.pid_code = 0x55,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_long_term_fuel_trim_b1 								= {.name = "",       		.pid_code = 0x56,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_short_term_fuel_trim_b2 							= {.name = "",       		.pid_code = 0x57,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_long_term_fuel_trim_b2 								= {.name = "",       		.pid_code = 0x58,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_fuel_rail_pressure_absolute 						= {.name = "",       		.pid_code = 0x59,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_relative_accelerator_pedal_position 				= {.name = "",       		.pid_code = 0x5a,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_hybrid_battery_pack_charge 							= {.name = "",       		.pid_code = 0x5b,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_engine_oil_temp 									= {.name = "",       		.pid_code = 0x5c,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_fuel_injection_timing 								= {.name = "",       		.pid_code = 0x5d,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_engine_fuel_rate 									= {.name = "",       		.pid_code = 0x5e,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_emission_requirements 								= {.name = "",       		.pid_code = 0x5f,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_driver_commanded_engine_tq_percentage 				= {.name = "",       		.pid_code = 0x61,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_actual_engine_tq_percentage 						= {.name = "",       		.pid_code = 0x62,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_engine_ref_tq 										= {.name = "",       		.pid_code = 0x63,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_engine_percent_tq_data 								= {.name = "",       		.pid_code = 0x64,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_aux_io_status 										= {.name = "",       		.pid_code = 0x65,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_maf_sensor 											= {.name = "",       		.pid_code = 0x66,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_engine_coolant_temp 								= {.name = "",       		.pid_code = 0x67,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_intake_air_temp_2 									= {.name = "",       		.pid_code = 0x68,   .conversion_func = NULL, 		.data_bytes = 0}; //Dupe
static saej1979_current_data_t saej1979_egr 												= {.name = "",       		.pid_code = 0x69,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_diesel_air_intake 									= {.name = "",       		.pid_code = 0x6a,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_exhaust_gas_recirc_temp 							= {.name = "",       		.pid_code = 0x6b,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_throttle_ctrl_and_pos 								= {.name = "",       		.pid_code = 0x6c,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_fuel_pres_ctrl_sys 									= {.name = "",       		.pid_code = 0x6d,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_injection_pres_ctrl_sys 							= {.name = "",       		.pid_code = 0x6e,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_turbo_inlet_pressure 								= {.name = "",       		.pid_code = 0x6f,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_boost_pressure_control 								= {.name = "",       		.pid_code = 0x70,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_variable_geometry_turbo_control 					= {.name = "",       		.pid_code = 0x71,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_wastegate_control 									= {.name = "",       		.pid_code = 0x72,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_exhaust_pressure 									= {.name = "",       		.pid_code = 0x73,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_turbo_rpm 											= {.name = "",       		.pid_code = 0x74,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_turbo_temp_a 										= {.name = "",       		.pid_code = 0x75,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_turbo_temp_b 										= {.name = "",       		.pid_code = 0x76,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_charge_air_cooler_temp 								= {.name = "",       		.pid_code = 0x77,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_exhaust_gas_temp_b1 								= {.name = "",       		.pid_code = 0x78,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_exhaust_gas_temp_b2 								= {.name = "",       		.pid_code = 0x79,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_particulate_filter_b1 								= {.name = "",       		.pid_code = 0x7a,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_particulate_filter_b2 								= {.name = "",       		.pid_code = 0x7b,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_particulate_filter_temp 							= {.name = "",       		.pid_code = 0x7c,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_nox_nte_control_status								= {.name = "",       		.pid_code = 0x7d,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_pm_nte_control_status 								= {.name = "",       		.pid_code = 0x7e,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_engine_runtime 										= {.name = "",       		.pid_code = 0x7f,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_available_pids_3 									= {.name = "",       		.pid_code = 0x80,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_engine_runtime_aecd_1_5 							= {.name = "",       		.pid_code = 0x81,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_engine_runtime_aecd_6_10 							= {.name = "",       		.pid_code = 0x82,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_nox_sensor 											= {.name = "",       		.pid_code = 0x83,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_manifold_surface_temp 								= {.name = "",       		.pid_code = 0x84,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_nox_control_system 									= {.name = "",       		.pid_code = 0x85,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_particulate_matter_sensor 							= {.name = "",       		.pid_code = 0x86,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_intake_manifold_abs_pres 							= {.name = "",       		.pid_code = 0x87,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_scr_inducement_sys 									= {.name = "",       		.pid_code = 0x88,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_engine_runtime_aecd_11_15 							= {.name = "",       		.pid_code = 0x89,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_engine_runtime_aecd_16_20 							= {.name = "",       		.pid_code = 0x8a,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_aftertreatment_status 								= {.name = "",       		.pid_code = 0x8b,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_o2_sensor 											= {.name = "",       		.pid_code = 0x8c,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_abs_throttle_pos_g									= {.name = "",       		.pid_code = 0x8d,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_engine_friction_percent_tq 							= {.name = "",       		.pid_code = 0x8e,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_particulate_matter 									= {.name = "",       		.pid_code = 0x8f,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_obd_sys_info 										= {.name = "",       		.pid_code = 0x90,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_obd_ecu_sys_info 									= {.name = "",       		.pid_code = 0x91,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_fuel_sys_status 									= {.name = "",       		.pid_code = 0x92,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_obd_counters 										= {.name = "",       		.pid_code = 0x93,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_nox_control 										= {.name = "",       		.pid_code = 0x94,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_scr_catalyst 										= {.name = "",       		.pid_code = 0x95,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_hydrocarbon_doser 									= {.name = "",       		.pid_code = 0x96,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_nox_mass_emission_rate 								= {.name = "",       		.pid_code = 0x97,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_exhaust_gas_temp_b1_2 								= {.name = "",       		.pid_code = 0x98,   .conversion_func = NULL, 		.data_bytes = 0};	//Dupe
static saej1979_current_data_t saej1979_exhaust_gas_temp_b2_2								= {.name = "",       		.pid_code = 0x99,   .conversion_func = NULL, 		.data_bytes = 0}; //Dupe
static saej1979_current_data_t saej1979_hybrid_ev_sys_data 									= {.name = "",       		.pid_code = 0x9a,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_def_sensor_output 									= {.name = "",       		.pid_code = 0x9b,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_o2_sensor_2 										= {.name = "",       		.pid_code = 0x9c,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_fuel_rate 											= {.name = "",       		.pid_code = 0x9d,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_engine_exhaust_flow_rate 							= {.name = "",       		.pid_code = 0x9e,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_fuel_system_percentage_use 							= {.name = "",       		.pid_code = 0x9f,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_available_pids_4 									= {.name = "",       		.pid_code = 0xa0,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_nox_sensor_corrected 								= {.name = "",       		.pid_code = 0xa1,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_cylinder_fuel_rate 									= {.name = "",       		.pid_code = 0xa2,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_evap_system_vapor_pressure_2 						= {.name = "",       		.pid_code = 0xa3,   .conversion_func = NULL, 		.data_bytes = 0}; //Dupe
static saej1979_current_data_t saej1979_transmission_actual_gear 							= {.name = "",       		.pid_code = 0xa4,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_def_dosing 											= {.name = "",       		.pid_code = 0xa5,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_odometer 											= {.name = "",       		.pid_code = 0xa6,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_nox_sensor_2 										= {.name = "",       		.pid_code = 0xa7,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_nox_sensor_corrected_2 								= {.name = "",       		.pid_code = 0xa8,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_motorcycle_io_status 								= {.name = "",       		.pid_code = 0xa9,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_speed_limiter_set 									= {.name = "",       		.pid_code = 0xaa,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_alternate_fuel_vehicle_data 						= {.name = "",       		.pid_code = 0xab,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_max_def_dosing 										= {.name = "",       		.pid_code = 0xac,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_crankcase_ventilation_data 							= {.name = "",       		.pid_code = 0xad,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_evap_system_purge_sensor 							= {.name = "",       		.pid_code = 0xae,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_commanded_fresh_air_flow 							= {.name = "",       		.pid_code = 0xaf,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_egr_mass_flow 										= {.name = "",       		.pid_code = 0xb0,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_compression_ignition_fuel_sys 						= {.name = "",       		.pid_code = 0xb1,   .conversion_func = NULL, 		.data_bytes = 0};	//TODO: this one
static saej1979_current_data_t saej1979_traction_battery_pack_health 						= {.name = "",       		.pid_code = 0xb2,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_hvesp_actual_charge_rate 							= {.name = "",       		.pid_code = 0xb3,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_hvesp_temp 											= {.name = "",       		.pid_code = 0xb4,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_hvesp_current 										= {.name = "",       		.pid_code = 0xb5,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_hvesp_voltage 										= {.name = "",       		.pid_code = 0xb6,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_hybrid_ev_battery_temp_data 						= {.name = "",       		.pid_code = 0xb7,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_hybrid_ev_battery_hv_sys_time_since_balance 		= {.name = "",       		.pid_code = 0xb8,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_hybrid_ev_battery_min_max_cell_voltage 				= {.name = "",       		.pid_code = 0xb9,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_hybrid_ev_battery_continuous_power 					= {.name = "",       		.pid_code = 0xba,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_energy_into_hv_storage 								= {.name = "",       		.pid_code = 0xbb,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_energy_from_hv_storage 								= {.name = "",       		.pid_code = 0xbc,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_hvess_energy_throughput 							= {.name = "",       		.pid_code = 0xbd,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_hvess_state_of_health  								= {.name = "",       		.pid_code = 0xbe,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_hvess_min_state_of_charge 							= {.name = "",       		.pid_code = 0xbf,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_available_pids_5 									= {.name = "",       		.pid_code = 0xc0,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_hvess_max_state_of_charge 							= {.name = "",       		.pid_code = 0xc1,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_hvess_discharge_energy_capacity 					= {.name = "",       		.pid_code = 0xc2,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_fuel_level_input_2 									= {.name = "",       		.pid_code = 0xc3,   .conversion_func = NULL, 		.data_bytes = 0};	//Dupe
static saej1979_current_data_t saej1979_exhaust_particulate_ctrl_sys 						= {.name = "",       		.pid_code = 0xc4,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_low_pres_fuel_sys 									= {.name = "",       		.pid_code = 0xc5,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_particulate_ctrl 									= {.name = "",       		.pid_code = 0xc6,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_dist_since_reflash 									= {.name = "",       		.pid_code = 0xc7,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_ncd_and_pcd_warning_status 							= {.name = "",       		.pid_code = 0xc8,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_o2_sensor_3 										= {.name = "",       		.pid_code = 0xc9,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_reserved	 										= {.name = "",       		.pid_code = 0xca,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_exhuast_pressure 									= {.name = "",       		.pid_code = 0xcb,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_electric_motor_rpm 									= {.name = "",       		.pid_code = 0xcc,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_electric_motor_tq 									= {.name = "",       		.pid_code = 0xcd,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_evap_sys_pressure_2 								= {.name = "",       		.pid_code = 0xce,   .conversion_func = NULL, 		.data_bytes = 0};
static saej1979_current_data_t saej1979_catalyst_temp 										= {.name = "",       		.pid_code = 0xcf,   .conversion_func = NULL, 		.data_bytes = 0};



saej1979_current_data_t* saej1979_current_data_arr[] = {
&saej1979_available_pids_1 									,		
&saej1979_monitor_status 										,		
&saej1979_dtc_freeze_frame									,		
&saej1979_fuel_system_status 									,		
&saej1979_calculated_engine_load 								,		
&saej1979_coolant_temp 										,		
&saej1979_short_term_fuel_trim_1 								,		
&saej1979_long_term_fuel_trim_1 								,		
&saej1979_short_term_fuel_trim_2 								,		
&saej1979_long_term_fuel_trim_2								,		
&saej1979_gauge_fuel_pressure 								,		
&saej1979_intake_air_pressure 								,		
&saej1979_engine_rpm 											,		
&saej1979_vehicle_speed 										,		
&saej1979_timing_advance 										,		
&saej1979_intake_air_temp 									,		
&saej1979_maf_flow_rate 										,		
&saej1979_throttle_position 									,		
&saej1979_secondary_air_status								,		
&saej1979_oxy_sensors_present									,		
&saej1979_oxy_sensor_1 										,		
&saej1979_oxy_sensor_2										,		
&saej1979_oxy_sensor_3 										,		
&saej1979_oxy_sensor_4 										,		
&saej1979_oxy_sensor_5 										,		
&saej1979_oxy_sensor_6 										,		
&saej1979_oxy_sensor_7 										,		
&saej1979_oxy_sensor_8 										,		
&saej1979_obd_standards 										,		
&saej1979_oxy_sensors_present_2								,		
&saej1979_aux_input_stats										,		
&saej1979_runtime												,		
&saej1979_pids_supported_2									,		
&saej1979_dist_travel_w_mil									,		
&saej1979_fuel_rail_pressure_vac 								,		
&saej1979_distance_while_mil_is_on							,		
&saej1979_fuel_pres_to_manifold_vacuum						,		
&saej1979_fuel_rail_pressure 									,		
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
&saej1979_evap_system_vapor_pressure 							,		
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
&saej1979_monitor_status 										,		
&saej1979_control_module_voltage 								,		
&saej1979_absolute_load_value 								,		
&saej1979_commanded_air_fuel_ratio 							,		
&saej1979_relative_throttle_position 							,		
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
&saej1979_evap_sys_absolute_pressure 							,		
&saej1979_evap_sys_pressure 									,		
&saej1979_short_term_fuel_trim_b1 							,		
&saej1979_long_term_fuel_trim_b1 								,		
&saej1979_short_term_fuel_trim_b2 							,		
&saej1979_long_term_fuel_trim_b2 								,		
&saej1979_fuel_rail_pressure_absolute 						,		
&saej1979_relative_accelerator_pedal_position 				,		
&saej1979_hybrid_battery_pack_charge 							,		
&saej1979_engine_oil_temp 									,		
&saej1979_fuel_injection_timing 								,		
&saej1979_engine_fuel_rate 									,		
&saej1979_emission_requirements 								,		
&saej1979_driver_commanded_engine_tq_percentage 				,		
&saej1979_actual_engine_tq_percentage 						,		
&saej1979_engine_ref_tq 										,		
&saej1979_engine_percent_tq_data 								,		
&saej1979_aux_io_status 										,		
&saej1979_maf_sensor 											,		
&saej1979_engine_coolant_temp 								,		
&saej1979_intake_air_temp 									,		
&saej1979_egr 												,		
&saej1979_diesel_air_intake 									,		
&saej1979_exhaust_gas_recirc_temp 							,		
&saej1979_throttle_ctrl_and_pos 								,		
&saej1979_fuel_pres_ctrl_sys 									,		
&saej1979_injection_pres_ctrl_sys 							,		
&saej1979_turbo_inlet_pressure 								,		
&saej1979_boost_pressure_control 								,		
&saej1979_variable_geometry_turbo_control 					,		
&saej1979_wastegate_control 									,		
&saej1979_exhaust_pressure 									,		
&saej1979_turbo_rpm 											,		
&saej1979_turbo_temp_a 										,		
&saej1979_turbo_temp_b 										,		
&saej1979_charge_air_cooler_temp 								,		
&saej1979_exhaust_gas_temp_b1 								,		
&saej1979_exhaust_gas_temp_b2 								,		
&saej1979_particulate_filter_b1 								,		
&saej1979_particulate_filter_b2 								,		
&saej1979_particulate_filter_temp 							,		
&saej1979_nox_nte_control_status								,		
&saej1979_pm_nte_control_status 								,		
&saej1979_engine_runtime 										,		
&saej1979_available_pids_3 									,		
&saej1979_engine_runtime_aecd_1_5 							,		
&saej1979_engine_runtime_aecd_6_10 							,		
&saej1979_nox_sensor 											,		
&saej1979_manifold_surface_temp 								,		
&saej1979_nox_control_system 									,		
&saej1979_particulate_matter_sensor 							,		
&saej1979_intake_manifold_abs_pres 							,		
&saej1979_scr_inducement_sys 									,		
&saej1979_engine_runtime_aecd_11_15 							,		
&saej1979_engine_runtime_aecd_16_20 							,		
&saej1979_aftertreatment_status 								,		
&saej1979_o2_sensor 											,		
&saej1979_abs_throttle_pos_g									,		
&saej1979_engine_friction_percent_tq 							,		
&saej1979_particulate_matter 									,		
&saej1979_obd_sys_info 										,		
&saej1979_obd_ecu_sys_info 									,		
&saej1979_fuel_sys_status 									,		
&saej1979_obd_counters 										,		
&saej1979_nox_control 										,		
&saej1979_scr_catalyst 										,		
&saej1979_hydrocarbon_doser 									,		
&saej1979_nox_mass_emission_rate 								,		
&saej1979_exhaust_gas_temp_b1_2 								,		
&saej1979_exhaust_gas_temp_b2_2								,		
&saej1979_hybrid_ev_sys_data 									,		
&saej1979_def_sensor_output 									,		
&saej1979_o2_sensor_2 										,		
&saej1979_fuel_rate 											,		
&saej1979_engine_exhaust_flow_rate 							,		
&saej1979_fuel_system_percentage_use 							,		
&saej1979_available_pids_4 									,		
&saej1979_nox_sensor_corrected 								,		
&saej1979_cylinder_fuel_rate 									,		
&saej1979_evap_system_vapor_pressure_2 						,		
&saej1979_transmission_actual_gear 							,		
&saej1979_def_dosing 											,		
&saej1979_odometer 											,		
&saej1979_nox_sensor_2 										,		
&saej1979_nox_sensor_corrected_2 								,		
&saej1979_motorcycle_io_status 								,		
&saej1979_speed_limiter_set 									,		
&saej1979_alternate_fuel_vehicle_data 						,		
&saej1979_max_def_dosing 										,		
&saej1979_crankcase_ventilation_data 							,		
&saej1979_evap_system_purge_sensor 							,		
&saej1979_commanded_fresh_air_flow 							,		
&saej1979_egr_mass_flow 										,		
&saej1979_compression_ignition_fuel_sys 						,		
&saej1979_traction_battery_pack_health 						,		
&saej1979_hvesp_actual_charge_rate 							,		
&saej1979_hvesp_temp 											,		
&saej1979_hvesp_current 										,		
&saej1979_hvesp_voltage 										,		
&saej1979_hybrid_ev_battery_temp_data 						,		
&saej1979_hybrid_ev_battery_hv_sys_time_since_balance 		, 		
&saej1979_hybrid_ev_battery_min_max_cell_voltage 				,		
&saej1979_hybrid_ev_battery_continuous_power 					,		
&saej1979_energy_into_hv_storage 								,		
&saej1979_energy_from_hv_storage 								,		
&saej1979_hvess_energy_throughput 							,		
&saej1979_hvess_state_of_health  								,		
&saej1979_hvess_min_state_of_charge 							,		
&saej1979_available_pids_5 									,		
&saej1979_hvess_max_state_of_charge 							,		
&saej1979_hvess_discharge_energy_capacity 					,		
&saej1979_fuel_level_input_2 									,		
&saej1979_exhaust_particulate_ctrl_sys 						,		
&saej1979_low_pres_fuel_sys 									,		
&saej1979_particulate_ctrl 									,		
&saej1979_dist_since_reflash 									,		
&saej1979_ncd_and_pcd_warning_status 							,		
&saej1979_o2_sensor_3 										,		
&saej1979_reserved	 										,		
&saej1979_exhuast_pressure 									,		
&saej1979_electric_motor_rpm 									,		
&saej1979_electric_motor_tq 									,		
&saej1979_evap_sys_pressure_2 								,		
&saej1979_catalyst_temp 										,		
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
