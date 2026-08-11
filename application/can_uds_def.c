/**********     INCLUDES        **********/
#include "can_uds_def.h"
#include "can_uds.h"
/**********     TYPEDEFS         **********/

/**********		DEFINES		**********/

/**********		EXTERNAL VARIABLE DEFINITIONS		**********/

/**********		STATIC VARIABLES		**********/
const char* const celsius = "C";
const char* const farenheit = "F";
const char* const kpa = "kPa";
const char* const bar = "bar";
const char* const lambda = "Equiv. Ratio";
const char* const rpm = "RPM";
const char* const degrees = "Degrees";
const char* const percent = "%";
const char* const psi = "PSI";
const char* const volts = "Volts";
const char* const kph = "kph";
const char* const g_per_s = "g/s";
const char* const seconds = "sec";
const char* const l_per_h = "L/hr";
const char* const nm = "Nm";

const float kpa_to_psi = 0.145038;;
const float kpa_to_bar = 0.01;
const float c_to_f = 1.8;			//Only the scale needs to be adjusted because they're all offset by -40C.







/*** Generated with current_data_array_generator_script.py. ***/
static saej1979_current_data_t saej1979_mass_air_flow_nest_pid_0 = {.name = "Mass Air Flow A", .pid_code = 0x66, .scale = 0.03125, .offset = 0, .data_bytes = 2, .first_byte = 1,  .min = 0, .max = 2040, .units = g_per_s, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_mass_air_flow_nest_pid_1 = {.name = "Mass Air Flow B", .pid_code = 0x66, .scale = 0.03125, .offset = 0, .data_bytes = 2, .first_byte = 3,  .min = 0, .max = 2040, .units = g_per_s, .available = false, .nested = NULL};


static saej1979_current_data_t* saej1979_mass_air_flow_nest[8] = {
&saej1979_mass_air_flow_nest_pid_0,
&saej1979_mass_air_flow_nest_pid_1,
};

static saej1979_current_data_t saej1979_engine_coolant_temp_nest_pid_0 = {.name = "Engine Coolant Temp 1", .pid_code = 0x67, .scale = 1, .offset = -40, .data_bytes = 1, .first_byte = 1,  .min = 0, .max = 200, .units = celsius, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_engine_coolant_temp_nest_pid_1 = {.name = "Engine Coolant Temp 2", .pid_code = 0x67, .scale = 1, .offset = -40, .data_bytes = 1, .first_byte = 2,  .min = 0, .max = 200, .units = celsius, .available = false, .nested = NULL};


static saej1979_current_data_t* saej1979_engine_coolant_temp_nest[8] = {
&saej1979_engine_coolant_temp_nest_pid_0,
&saej1979_engine_coolant_temp_nest_pid_1,
};

static saej1979_current_data_t saej1979_intake_air_temp_nest_pid_0 = {.name = "Intake Air Temp B1 S1", .pid_code = 0x68, .scale = 1, .offset = -40, .data_bytes = 1, .first_byte = 1,  .min = -20, .max = 100, .units = celsius, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_intake_air_temp_nest_pid_1 = {.name = "Intake Air Temp B1 S2", .pid_code = 0x68, .scale = 1, .offset = -40, .data_bytes = 1, .first_byte = 2,  .min = -20, .max = 100, .units = celsius, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_intake_air_temp_nest_pid_2 = {.name = "Intake Air Temp B1 S3", .pid_code = 0x68, .scale = 1, .offset = -40, .data_bytes = 1, .first_byte = 3,  .min = -20, .max = 100, .units = celsius, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_intake_air_temp_nest_pid_3 = {.name = "Intake Air Temp B2 S1", .pid_code = 0x68, .scale = 1, .offset = -40, .data_bytes = 1, .first_byte = 4,  .min = -20, .max = 100, .units = celsius, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_intake_air_temp_nest_pid_4 = {.name = "Intake Air Temp B2 S2", .pid_code = 0x68, .scale = 1, .offset = -40, .data_bytes = 1, .first_byte = 5,  .min = -20, .max = 100, .units = celsius, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_intake_air_temp_nest_pid_5 = {.name = "Intake Air Temp B2 S3", .pid_code = 0x68, .scale = 1, .offset = -40, .data_bytes = 1, .first_byte = 6,  .min = -20, .max = 100, .units = celsius, .available = false, .nested = NULL};


static saej1979_current_data_t* saej1979_intake_air_temp_nest[8] = {
&saej1979_intake_air_temp_nest_pid_0,
&saej1979_intake_air_temp_nest_pid_1,
&saej1979_intake_air_temp_nest_pid_2,
&saej1979_intake_air_temp_nest_pid_3,
&saej1979_intake_air_temp_nest_pid_4,
&saej1979_intake_air_temp_nest_pid_5,
};

static saej1979_current_data_t saej1979_diesel_air_flow_nest_pid_0 = {.name = "Cmd. Intake Air Flow A", .pid_code = 0x6A, .scale = 0.392157, .offset = 0, .data_bytes = 1, .first_byte = 1,  .min = 0, .max = 100, .units = percent, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_diesel_air_flow_nest_pid_1 = {.name = "Rel. Intake Air Flow A", .pid_code = 0x6A, .scale = 0.392157, .offset = 0, .data_bytes = 1, .first_byte = 2,  .min = 0, .max = 100, .units = percent, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_diesel_air_flow_nest_pid_2 = {.name = "Cmd. Intake Air Flow B", .pid_code = 0x6A, .scale = 0.392157, .offset = 0, .data_bytes = 1, .first_byte = 3,  .min = 0, .max = 100, .units = percent, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_diesel_air_flow_nest_pid_3 = {.name = "Rel. Intake Air Flow B", .pid_code = 0x6A, .scale = 0.392157, .offset = 0, .data_bytes = 1, .first_byte = 4,  .min = 0, .max = 100, .units = percent, .available = false, .nested = NULL};


static saej1979_current_data_t* saej1979_diesel_air_flow_nest[8] = {
&saej1979_diesel_air_flow_nest_pid_0,
&saej1979_diesel_air_flow_nest_pid_1,
&saej1979_diesel_air_flow_nest_pid_2,
&saej1979_diesel_air_flow_nest_pid_3,
};

static saej1979_current_data_t saej1979_fuel_pressure_ctrl_nest_pid_0 = {.name = "Cmd Fuel Rail Pressure A", .pid_code = 0x6D, .scale = 10, .offset = 0, .data_bytes = 2, .first_byte = 1,  .min = 0, .max = 655350, .units = kpa, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_fuel_pressure_ctrl_nest_pid_1 = {.name = "Fuel Rail Pressure A", .pid_code = 0x6D, .scale = 10, .offset = 0, .data_bytes = 2, .first_byte = 3,  .min = 0, .max = 655350, .units = kpa, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_fuel_pressure_ctrl_nest_pid_2 = {.name = "Fuel Rail Temp A", .pid_code = 0x6D, .scale = 1, .offset = -40, .data_bytes = 1, .first_byte = 5,  .min = -40, .max = 215, .units = celsius, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_fuel_pressure_ctrl_nest_pid_3 = {.name = "Cmd Fuel Rail Pressure B", .pid_code = 0x6D, .scale = 10, .offset = 0, .data_bytes = 2, .first_byte = 6,  .min = 0, .max = 655350, .units = kpa, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_fuel_pressure_ctrl_nest_pid_4 = {.name = "Fuel Rail Pressure B", .pid_code = 0x6D, .scale = 10, .offset = 0, .data_bytes = 2, .first_byte = 8,  .min = 0, .max = 655350, .units = kpa, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_fuel_pressure_ctrl_nest_pid_5 = {.name = "Fuel Rail Temp B", .pid_code = 0x6D, .scale = 1, .offset = -40, .data_bytes = 1, .first_byte = 10,  .min = -40, .max = 215, .units = celsius, .available = false, .nested = NULL};


static saej1979_current_data_t* saej1979_fuel_pressure_ctrl_nest[8] = {
&saej1979_fuel_pressure_ctrl_nest_pid_0,
&saej1979_fuel_pressure_ctrl_nest_pid_1,
&saej1979_fuel_pressure_ctrl_nest_pid_2,
&saej1979_fuel_pressure_ctrl_nest_pid_3,
&saej1979_fuel_pressure_ctrl_nest_pid_4,
&saej1979_fuel_pressure_ctrl_nest_pid_5,
};

static saej1979_current_data_t saej1979_injection_pressure_ctrl_nest_pid_0 = {.name = "Cmd Injection Pressure A", .pid_code = 0x6E, .scale = 10, .offset = 0, .data_bytes = 2, .first_byte = 1,  .min = 0, .max = 655350, .units = kpa, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_injection_pressure_ctrl_nest_pid_1 = {.name = "Injection Pressure A", .pid_code = 0x6E, .scale = 10, .offset = 0, .data_bytes = 2, .first_byte = 3,  .min = 0, .max = 655350, .units = kpa, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_injection_pressure_ctrl_nest_pid_2 = {.name = "Cmd Injection Pressure B", .pid_code = 0x6E, .scale = 10, .offset = 0, .data_bytes = 2, .first_byte = 5,  .min = 0, .max = 655350, .units = kpa, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_injection_pressure_ctrl_nest_pid_3 = {.name = "Injection Pressure B", .pid_code = 0x6E, .scale = 10, .offset = 0, .data_bytes = 2, .first_byte = 7,  .min = 0, .max = 655350, .units = kpa, .available = false, .nested = NULL};


static saej1979_current_data_t* saej1979_injection_pressure_ctrl_nest[8] = {
&saej1979_injection_pressure_ctrl_nest_pid_0,
&saej1979_injection_pressure_ctrl_nest_pid_1,
&saej1979_injection_pressure_ctrl_nest_pid_2,
&saej1979_injection_pressure_ctrl_nest_pid_3,
};

static saej1979_current_data_t saej1979_turbo_inlet_pressure_nest_pid_0 = {.name = "Turbo Inlet Pressure A", .pid_code = 0x6F, .scale = 1, .offset = 0, .data_bytes = 1, .first_byte = 1,  .min = 0, .max = 210, .units = kpa, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_turbo_inlet_pressure_nest_pid_1 = {.name = "Turbo Inlet Pressure B", .pid_code = 0x6F, .scale = 1, .offset = 0, .data_bytes = 1, .first_byte = 2,  .min = 0, .max = 210, .units = kpa, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_turbo_inlet_pressure_nest_pid_2 = {.name = "Turbo Inlet Pressure A", .pid_code = 0x6F, .scale = 8, .offset = 0, .data_bytes = 1, .first_byte = 3,  .min = 0, .max = 210, .units = kpa, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_turbo_inlet_pressure_nest_pid_3 = {.name = "Turbo Inlet Pressure B", .pid_code = 0x6F, .scale = 8, .offset = 0, .data_bytes = 1, .first_byte = 4,  .min = 0, .max = 210, .units = kpa, .available = false, .nested = NULL};


static saej1979_current_data_t* saej1979_turbo_inlet_pressure_nest[8] = {
&saej1979_turbo_inlet_pressure_nest_pid_0,
&saej1979_turbo_inlet_pressure_nest_pid_1,
&saej1979_turbo_inlet_pressure_nest_pid_2,
&saej1979_turbo_inlet_pressure_nest_pid_3,
};

static saej1979_current_data_t saej1979_boost_pressure_nest_pid_0 = {.name = "Cmd Boost Pressure A", .pid_code = 0x70, .scale = 0.03125, .offset = 0, .data_bytes = 2, .first_byte = 1,  .min = 0, .max = 210, .units = kpa, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_boost_pressure_nest_pid_1 = {.name = "Boost Pressure A", .pid_code = 0x70, .scale = 0.03125, .offset = 0, .data_bytes = 2, .first_byte = 3,  .min = 0, .max = 210, .units = kpa, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_boost_pressure_nest_pid_2 = {.name = "Boost Ctrl Sts A", .pid_code = 0x70, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_boost_pressure_nest_pid_3 = {.name = "Cmd Boost Pressure B", .pid_code = 0x70, .scale = 0.03125, .offset = 0, .data_bytes = 2, .first_byte = 5,  .min = 0, .max = 210, .units = kpa, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_boost_pressure_nest_pid_4 = {.name = "Boost Pressure B", .pid_code = 0x70, .scale = 0.03125, .offset = 0, .data_bytes = 2, .first_byte = 7,  .min = 0, .max = 210, .units = kpa, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_boost_pressure_nest_pid_5 = {.name = "Boost Ctrl Sts B", .pid_code = 0x70, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};


static saej1979_current_data_t* saej1979_boost_pressure_nest[8] = {
&saej1979_boost_pressure_nest_pid_0,
&saej1979_boost_pressure_nest_pid_1,
&saej1979_boost_pressure_nest_pid_2,
&saej1979_boost_pressure_nest_pid_3,
&saej1979_boost_pressure_nest_pid_4,
&saej1979_boost_pressure_nest_pid_5,
};

static saej1979_current_data_t saej1979_vari_geometry_turbo_ctrl_nest_pid_0 = {.name = "Cmd Turbo Pos. A", .pid_code = 0x71, .scale = 0.392157, .offset = 0, .data_bytes = 1, .first_byte = 1,  .min = 0, .max = 100, .units = percent, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_vari_geometry_turbo_ctrl_nest_pid_1 = {.name = "Turbo Position A", .pid_code = 0x71, .scale = 0.392157, .offset = 0, .data_bytes = 1, .first_byte = 2,  .min = 0, .max = 100, .units = percent, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_vari_geometry_turbo_ctrl_nest_pid_2 = {.name = "Turbo Pos. Sts A", .pid_code = 0x71, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_vari_geometry_turbo_ctrl_nest_pid_3 = {.name = "Cmd Turbo Pos. B", .pid_code = 0x71, .scale = 0.392157, .offset = 0, .data_bytes = 1, .first_byte = 3,  .min = 0, .max = 100, .units = percent, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_vari_geometry_turbo_ctrl_nest_pid_4 = {.name = "Turbo Position B", .pid_code = 0x71, .scale = 0.392157, .offset = 0, .data_bytes = 1, .first_byte = 4,  .min = 0, .max = 100, .units = percent, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_vari_geometry_turbo_ctrl_nest_pid_5 = {.name = "Turbo Pos. Sts B", .pid_code = 0x71, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};


static saej1979_current_data_t* saej1979_vari_geometry_turbo_ctrl_nest[8] = {
&saej1979_vari_geometry_turbo_ctrl_nest_pid_0,
&saej1979_vari_geometry_turbo_ctrl_nest_pid_1,
&saej1979_vari_geometry_turbo_ctrl_nest_pid_2,
&saej1979_vari_geometry_turbo_ctrl_nest_pid_3,
&saej1979_vari_geometry_turbo_ctrl_nest_pid_4,
&saej1979_vari_geometry_turbo_ctrl_nest_pid_5,
};

static saej1979_current_data_t saej1979_wastegate_ctrl_nest_pid_0 = {.name = "Cmd Wastegate Pos. A", .pid_code = 0x72, .scale = 0.392157, .offset = 0, .data_bytes = 1, .first_byte = 1,  .min = 0, .max = 100, .units = percent, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_wastegate_ctrl_nest_pid_1 = {.name = "Wastegate Pos. A", .pid_code = 0x72, .scale = 0.392157, .offset = 0, .data_bytes = 1, .first_byte = 2,  .min = 0, .max = 100, .units = percent, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_wastegate_ctrl_nest_pid_2 = {.name = "Cmd Wastegate Pos. B", .pid_code = 0x72, .scale = 0.392157, .offset = 0, .data_bytes = 1, .first_byte = 3,  .min = 0, .max = 100, .units = percent, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_wastegate_ctrl_nest_pid_3 = {.name = "Wastegate Pos. B", .pid_code = 0x72, .scale = 0.392157, .offset = 0, .data_bytes = 1, .first_byte = 4,  .min = 0, .max = 100, .units = percent, .available = false, .nested = NULL};


static saej1979_current_data_t* saej1979_wastegate_ctrl_nest[8] = {
&saej1979_wastegate_ctrl_nest_pid_0,
&saej1979_wastegate_ctrl_nest_pid_1,
&saej1979_wastegate_ctrl_nest_pid_2,
&saej1979_wastegate_ctrl_nest_pid_3,
};

static saej1979_current_data_t saej1979_exhaust_pressure_nest_pid_0 = {.name = "Exhaust Pressure B1", .pid_code = 0x73, .scale = 0.01, .offset = 0, .data_bytes = 2, .first_byte = 1,  .min = 0, .max = 655, .units = kpa, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_exhaust_pressure_nest_pid_1 = {.name = "Exhaust Pressure B2", .pid_code = 0x73, .scale = 0.01, .offset = 0, .data_bytes = 2, .first_byte = 3,  .min = 0, .max = 655, .units = kpa, .available = false, .nested = NULL};


static saej1979_current_data_t* saej1979_exhaust_pressure_nest[8] = {
&saej1979_exhaust_pressure_nest_pid_0,
&saej1979_exhaust_pressure_nest_pid_1,
};

static saej1979_current_data_t saej1979_turbo_rpm_nest_pid_0 = {.name = "Turbo RPM A", .pid_code = 0x74, .scale = 10, .offset = 0, .data_bytes = 2, .first_byte = 1,  .min = 0, .max = 655350, .units = rpm, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_turbo_rpm_nest_pid_1 = {.name = "Turbo RPM B", .pid_code = 0x74, .scale = 10, .offset = 0, .data_bytes = 2, .first_byte = 3,  .min = 0, .max = 655350, .units = rpm, .available = false, .nested = NULL};


static saej1979_current_data_t* saej1979_turbo_rpm_nest[8] = {
&saej1979_turbo_rpm_nest_pid_0,
&saej1979_turbo_rpm_nest_pid_1,
};

static saej1979_current_data_t saej1979_turbo_temp_a_nest_pid_0 = {.name = "Turbo Compressor In Temp A", .pid_code = 0x75, .scale = 1, .offset = -40, .data_bytes = 1, .first_byte = 1,  .min = -20, .max = 200, .units = celsius, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_turbo_temp_a_nest_pid_1 = {.name = "Turbo Compressor Out Temp A", .pid_code = 0x75, .scale = 1, .offset = -40, .data_bytes = 1, .first_byte = 2,  .min = -20, .max = 200, .units = celsius, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_turbo_temp_a_nest_pid_2 = {.name = "Turbo Turbine In Temp A", .pid_code = 0x75, .scale = 0.1, .offset = -40, .data_bytes = 2, .first_byte = 3,  .min = -20, .max = 1500, .units = celsius, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_turbo_temp_a_nest_pid_3 = {.name = "Turbo Turbine Out Temp A", .pid_code = 0x75, .scale = 0.1, .offset = -40, .data_bytes = 2, .first_byte = 5,  .min = -20, .max = 1500, .units = celsius, .available = false, .nested = NULL};


static saej1979_current_data_t* saej1979_turbo_temp_a_nest[8] = {
&saej1979_turbo_temp_a_nest_pid_0,
&saej1979_turbo_temp_a_nest_pid_1,
&saej1979_turbo_temp_a_nest_pid_2,
&saej1979_turbo_temp_a_nest_pid_3,
};

static saej1979_current_data_t saej1979_turbo_temp_b_nest_pid_0 = {.name = "Turbo Compressor In Temp B", .pid_code = 0x76, .scale = 1, .offset = -40, .data_bytes = 1, .first_byte = 1,  .min = -20, .max = 200, .units = celsius, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_turbo_temp_b_nest_pid_1 = {.name = "Turbo Compressor Out Temp B", .pid_code = 0x76, .scale = 1, .offset = -40, .data_bytes = 1, .first_byte = 2,  .min = -20, .max = 200, .units = celsius, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_turbo_temp_b_nest_pid_2 = {.name = "Turbo Turbine In Temp B", .pid_code = 0x76, .scale = 0.1, .offset = -40, .data_bytes = 2, .first_byte = 3,  .min = -20, .max = 1500, .units = celsius, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_turbo_temp_b_nest_pid_3 = {.name = "Turbo Turbine Out Temp B", .pid_code = 0x76, .scale = 0.1, .offset = -40, .data_bytes = 2, .first_byte = 5,  .min = -20, .max = 1500, .units = celsius, .available = false, .nested = NULL};


static saej1979_current_data_t* saej1979_turbo_temp_b_nest[8] = {
&saej1979_turbo_temp_b_nest_pid_0,
&saej1979_turbo_temp_b_nest_pid_1,
&saej1979_turbo_temp_b_nest_pid_2,
&saej1979_turbo_temp_b_nest_pid_3,
};

static saej1979_current_data_t saej1979_charge_air_cooler_temp_nest_pid_0 = {.name = "Charge Air Temp B1 S1", .pid_code = 0x77, .scale = 1, .offset = -40, .data_bytes = 1, .first_byte = 1,  .min = -20, .max = 80, .units = celsius, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_charge_air_cooler_temp_nest_pid_1 = {.name = "Charge Air Temp B1 S2", .pid_code = 0x77, .scale = 1, .offset = -40, .data_bytes = 1, .first_byte = 2,  .min = -20, .max = 80, .units = celsius, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_charge_air_cooler_temp_nest_pid_2 = {.name = "Charge Air Temp B2 S1", .pid_code = 0x77, .scale = 1, .offset = -40, .data_bytes = 1, .first_byte = 3,  .min = -20, .max = 80, .units = celsius, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_charge_air_cooler_temp_nest_pid_3 = {.name = "Charge Air Temp B2 S2", .pid_code = 0x77, .scale = 1, .offset = -40, .data_bytes = 1, .first_byte = 4,  .min = -20, .max = 80, .units = celsius, .available = false, .nested = NULL};


static saej1979_current_data_t* saej1979_charge_air_cooler_temp_nest[8] = {
&saej1979_charge_air_cooler_temp_nest_pid_0,
&saej1979_charge_air_cooler_temp_nest_pid_1,
&saej1979_charge_air_cooler_temp_nest_pid_2,
&saej1979_charge_air_cooler_temp_nest_pid_3,
};

static saej1979_current_data_t saej1979_exhaust_gas_temp_a_nest_pid_0 = {.name = "Exhaust Gas Temp B1 S1", .pid_code = 0x78, .scale = 0.1, .offset = -40, .data_bytes = 2, .first_byte = 1,  .min = 0, .max = 760, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_exhaust_gas_temp_a_nest_pid_1 = {.name = "Exhaust Gas Temp B1 S2", .pid_code = 0x78, .scale = 0.1, .offset = -40, .data_bytes = 2, .first_byte = 3,  .min = 0, .max = 760, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_exhaust_gas_temp_a_nest_pid_2 = {.name = "Exhaust Gas Temp B1 S3", .pid_code = 0x78, .scale = 0.1, .offset = -40, .data_bytes = 2, .first_byte = 5,  .min = 0, .max = 760, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_exhaust_gas_temp_a_nest_pid_3 = {.name = "Exhaust Gas Temp B1 S4", .pid_code = 0x78, .scale = 0.1, .offset = -40, .data_bytes = 2, .first_byte = 7,  .min = 0, .max = 760, .units = NULL, .available = false, .nested = NULL};


static saej1979_current_data_t* saej1979_exhaust_gas_temp_a_nest[8] = {
&saej1979_exhaust_gas_temp_a_nest_pid_0,
&saej1979_exhaust_gas_temp_a_nest_pid_1,
&saej1979_exhaust_gas_temp_a_nest_pid_2,
&saej1979_exhaust_gas_temp_a_nest_pid_3,
};

static saej1979_current_data_t saej1979_exhaust_gas_temp_b_nest_pid_0 = {.name = "Exhaust Gas Temp B2 S1", .pid_code = 0x79, .scale = 0.1, .offset = -40, .data_bytes = 2, .first_byte = 1,  .min = 0, .max = 760, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_exhaust_gas_temp_b_nest_pid_1 = {.name = "Exhaust Gas Temp B2 S2", .pid_code = 0x79, .scale = 0.1, .offset = -40, .data_bytes = 2, .first_byte = 3,  .min = 0, .max = 760, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_exhaust_gas_temp_b_nest_pid_2 = {.name = "Exhaust Gas Temp B2 S3", .pid_code = 0x79, .scale = 0.1, .offset = -40, .data_bytes = 2, .first_byte = 5,  .min = 0, .max = 760, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_exhaust_gas_temp_b_nest_pid_3 = {.name = "Exhaust Gas Temp B2 S4", .pid_code = 0x79, .scale = 0.1, .offset = -40, .data_bytes = 2, .first_byte = 7,  .min = 0, .max = 760, .units = NULL, .available = false, .nested = NULL};


static saej1979_current_data_t* saej1979_exhaust_gas_temp_b_nest[8] = {
&saej1979_exhaust_gas_temp_b_nest_pid_0,
&saej1979_exhaust_gas_temp_b_nest_pid_1,
&saej1979_exhaust_gas_temp_b_nest_pid_2,
&saej1979_exhaust_gas_temp_b_nest_pid_3,
};

static saej1979_current_data_t saej1979_intake_manifold_pressure_nest_pid_0 = {.name = "Intake Manifold Pressure A", .pid_code = 0x87, .scale = 0.03125, .offset = 0, .data_bytes = 2, .first_byte = 1,  .min = 0, .max = 300, .units = kpa, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_intake_manifold_pressure_nest_pid_1 = {.name = "Intake Manifold Pressure B", .pid_code = 0x87, .scale = 0.03125, .offset = 0, .data_bytes = 2, .first_byte = 3,  .min = 0, .max = 300, .units = kpa, .available = false, .nested = NULL};


static saej1979_current_data_t* saej1979_intake_manifold_pressure_nest[8] = {
&saej1979_intake_manifold_pressure_nest_pid_0,
&saej1979_intake_manifold_pressure_nest_pid_1,
};

static saej1979_current_data_t saej1979_current_data_pid_0 = {.name = "Available PIDs 1", .pid_code = 0x00, .scale = 1, .offset = 0, .data_bytes = 4, .first_byte = 0,  .min = 0, .max = 0xFFFFFFFF, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_1 = {.name = "I/M Readiness Data", .pid_code = 0x01, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_2 = {.name = "Freeze Frame DTC", .pid_code = 0x02, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_3 = {.name = "Fuel system status ", .pid_code = 0x03, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_4 = {.name = "Calculated Load Value", .pid_code = 0x04, .scale = 0.3915686, .offset = 0, .data_bytes = 1, .first_byte = 0,  .min = 0, .max = 100, .units = percent, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_5 = {.name = "Engine Coolant Temperature", .pid_code = 0x05, .scale = 1, .offset = -40, .data_bytes = 1, .first_byte = 0,  .min = -40, .max = 215, .units = celsius, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_6 = {.name = "Short Term Fuel Trim - B1", .pid_code = 0x06, .scale = 0.78125, .offset = -100, .data_bytes = 1, .first_byte = 0,  .min = -20, .max = 20, .units = percent, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_7 = {.name = "Long Term Fuel Trim - B1", .pid_code = 0x07, .scale = 0.78125, .offset = -100, .data_bytes = 1, .first_byte = 0,  .min = -20, .max = 20, .units = percent, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_8 = {.name = "Short Term Fuel Trim - B2", .pid_code = 0x08, .scale = 0.78125, .offset = -100, .data_bytes = 1, .first_byte = 0,  .min = -20, .max = 20, .units = percent, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_9 = {.name = "Long Term Fuel Trim - B2", .pid_code = 0x09, .scale = 0.78125, .offset = -100, .data_bytes = 1, .first_byte = 0,  .min = -20, .max = 20, .units = percent, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_10 = {.name = "Fuel Pressure (gauge)", .pid_code = 0x0A, .scale = 3, .offset = 0, .data_bytes = 1, .first_byte = 0,  .min = 0, .max = 765, .units = kpa, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_11 = {.name = "Abs. Intake Manifold Pressure", .pid_code = 0x0B, .scale = 1, .offset = 0, .data_bytes = 1, .first_byte = 0,  .min = 0, .max = 255, .units = kpa, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_12 = {.name = "Engine RPM", .pid_code = 0x0C, .scale = 0.25, .offset = 0, .data_bytes = 2, .first_byte = 0,  .min = 0, .max = 10000, .units = rpm, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_13 = {.name = "Vehicle Speed", .pid_code = 0x0D, .scale = 1, .offset = 0, .data_bytes = 1, .first_byte = 0,  .min = 0, .max = 255, .units = kph, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_14 = {.name = "Ignition Timing Angle", .pid_code = 0x0E, .scale = 0.5, .offset = -64, .data_bytes = 1, .first_byte = 0,  .min = -60, .max = 60, .units = degrees, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_15 = {.name = "Intake Air Temp", .pid_code = 0x0F, .scale = 1, .offset = -40, .data_bytes = 1, .first_byte = 0,  .min = -20, .max = 160, .units = celsius, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_16 = {.name = "Mass Air Flow Rate", .pid_code = 0x10, .scale = 0.01, .offset = 0, .data_bytes = 2, .first_byte = 0,  .min = 0, .max = 655, .units = g_per_s, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_17 = {.name = "Abs. Throttle Position", .pid_code = 0x11, .scale = 0.3915686, .offset = 0, .data_bytes = 1, .first_byte = 0,  .min = 0, .max = 100, .units = percent, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_18 = {.name = "Commanded Secondary Air Status", .pid_code = 0x12, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_19 = {.name = "Location of oxygen sensors", .pid_code = 0x13, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_20 = {.name = "Short Term Fuel Trim B1 S1", .pid_code = 0x14, .scale = 0.78125, .offset = -100, .data_bytes = 1, .first_byte = 1,  .min = -20, .max = 20, .units = percent, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_21 = {.name = "Short Term Fuel Trim B1 S2", .pid_code = 0x15, .scale = 0.78125, .offset = -100, .data_bytes = 1, .first_byte = 1,  .min = -20, .max = 20, .units = percent, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_22 = {.name = "Short Term Fuel Trim B1 S3", .pid_code = 0x16, .scale = 0.78125, .offset = -100, .data_bytes = 1, .first_byte = 1,  .min = -20, .max = 20, .units = percent, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_23 = {.name = "Short Term Fuel Trim B1 S4", .pid_code = 0x17, .scale = 0.78125, .offset = -100, .data_bytes = 1, .first_byte = 1,  .min = -20, .max = 20, .units = percent, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_24 = {.name = "Short Term Fuel Trim B2 S1", .pid_code = 0x18, .scale = 0.78125, .offset = -100, .data_bytes = 1, .first_byte = 1,  .min = -20, .max = 20, .units = percent, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_25 = {.name = "Short Term Fuel Trim B2 S2", .pid_code = 0x19, .scale = 0.78125, .offset = -100, .data_bytes = 1, .first_byte = 1,  .min = -20, .max = 20, .units = percent, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_26 = {.name = "Short Term Fuel Trim B2 S3", .pid_code = 0x1A, .scale = 0.78125, .offset = -100, .data_bytes = 1, .first_byte = 1,  .min = -20, .max = 20, .units = percent, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_27 = {.name = "Short Term Fuel Trim B2 S4", .pid_code = 0x1B, .scale = 0.78125, .offset = -100, .data_bytes = 1, .first_byte = 1,  .min = -20, .max = 20, .units = percent, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_28 = {.name = "OBD requirements for vehicle or engine", .pid_code = 0x1C, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_29 = {.name = "Location of oxygen sensors", .pid_code = 0x1D, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_30 = {.name = "Auxiliary Input Status", .pid_code = 0x1E, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_31 = {.name = "Time Since Engine Start", .pid_code = 0x1F, .scale = 1, .offset = 0, .data_bytes = 2, .first_byte = 0,  .min = 0, .max = 65535, .units = seconds, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_32 = {.name = "Available PIDs 2", .pid_code = 0x20, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_33 = {.name = "Distance Traveled While MIL is Activated", .pid_code = 0x21, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_34 = {.name = "Fuel Pressure (relative to manifold)", .pid_code = 0x22, .scale = 0.079, .offset = 0, .data_bytes = 2, .first_byte = 0,  .min = 0, .max = 5000, .units = kpa, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_35 = {.name = "Fuel Rail Pressure", .pid_code = 0x23, .scale = 10, .offset = 0, .data_bytes = 2, .first_byte = 0,  .min = 0, .max = 655350, .units = kpa, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_36 = {.name = "Wideband A/F Ratio B1 S1", .pid_code = 0x24, .scale = 3.05e-05, .offset = 0, .data_bytes = 2, .first_byte = 0,  .min = 0, .max = 2, .units = lambda, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_37 = {.name = "Wideband A/F Ratio B1 S2", .pid_code = 0x25, .scale = 3.05e-05, .offset = 0, .data_bytes = 2, .first_byte = 0,  .min = 0, .max = 2, .units = lambda, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_38 = {.name = "Wideband A/F Ratio B1 S3", .pid_code = 0x26, .scale = 3.05e-05, .offset = 0, .data_bytes = 2, .first_byte = 0,  .min = 0, .max = 2, .units = lambda, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_39 = {.name = "Wideband A/F Ratio B1 S4", .pid_code = 0x27, .scale = 3.05e-05, .offset = 0, .data_bytes = 2, .first_byte = 0,  .min = 0, .max = 2, .units = lambda, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_40 = {.name = "Wideband A/F Ratio B2 S1", .pid_code = 0x28, .scale = 3.05e-05, .offset = 0, .data_bytes = 2, .first_byte = 0,  .min = 0, .max = 2, .units = lambda, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_41 = {.name = "Wideband A/F Ratio B2 S2", .pid_code = 0x29, .scale = 3.05e-05, .offset = 0, .data_bytes = 2, .first_byte = 0,  .min = 0, .max = 2, .units = lambda, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_42 = {.name = "Wideband A/F Ratio B2 S3", .pid_code = 0x2A, .scale = 3.05e-05, .offset = 0, .data_bytes = 2, .first_byte = 0,  .min = 0, .max = 2, .units = lambda, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_43 = {.name = "Wideband A/F Ratio B2 S4", .pid_code = 0x2B, .scale = 3.05e-05, .offset = 0, .data_bytes = 2, .first_byte = 0,  .min = 0, .max = 2, .units = lambda, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_44 = {.name = "Commanded EGR", .pid_code = 0x2C, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_45 = {.name = "EGR Error", .pid_code = 0x2D, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_46 = {.name = "Commanded Evaporative Purge", .pid_code = 0x2E, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_47 = {.name = "Fuel Level Input", .pid_code = 0x2F, .scale = 0.39215686, .offset = 0, .data_bytes = 1, .first_byte = 0,  .min = 0, .max = 100, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_48 = {.name = "Number of warm-ups since DTCs cleared", .pid_code = 0x30, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_49 = {.name = "Distance traveled since DTCs cleared", .pid_code = 0x31, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_50 = {.name = "Evap System Vapor Pressure", .pid_code = 0x32, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_51 = {.name = "Barometric Pressure ", .pid_code = 0x33, .scale = 1, .offset = 0, .data_bytes = 1, .first_byte = 0,  .min = 75, .max = 110, .units = kpa, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_52 = {.name = "Wideband A/F Ratio B1 S1", .pid_code = 0x34, .scale = 3.05e-05, .offset = 0, .data_bytes = 2, .first_byte = 0,  .min = 0, .max = 2, .units = lambda, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_53 = {.name = "Wideband A/F Ratio B1 S2", .pid_code = 0x35, .scale = 3.05e-05, .offset = 0, .data_bytes = 2, .first_byte = 0,  .min = 0, .max = 2, .units = lambda, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_54 = {.name = "Wideband A/F Ratio B1 S3", .pid_code = 0x36, .scale = 3.05e-05, .offset = 0, .data_bytes = 2, .first_byte = 0,  .min = 0, .max = 2, .units = lambda, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_55 = {.name = "Wideband A/F Ratio B1 S4", .pid_code = 0x37, .scale = 3.05e-05, .offset = 0, .data_bytes = 2, .first_byte = 0,  .min = 0, .max = 2, .units = lambda, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_56 = {.name = "Wideband A/F Ratio B2 S1", .pid_code = 0x38, .scale = 3.05e-05, .offset = 0, .data_bytes = 2, .first_byte = 0,  .min = 0, .max = 2, .units = lambda, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_57 = {.name = "Wideband A/F Ratio B2 S2", .pid_code = 0x39, .scale = 3.05e-05, .offset = 0, .data_bytes = 2, .first_byte = 0,  .min = 0, .max = 2, .units = lambda, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_58 = {.name = "Wideband A/F Ratio B2 S3", .pid_code = 0x3A, .scale = 3.05e-05, .offset = 0, .data_bytes = 2, .first_byte = 0,  .min = 0, .max = 2, .units = lambda, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_59 = {.name = "Wideband A/F Ratio B2 S4", .pid_code = 0x3B, .scale = 3.05e-05, .offset = 0, .data_bytes = 2, .first_byte = 0,  .min = 0, .max = 2, .units = lambda, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_60 = {.name = "Catalyst Temperature B1 S1", .pid_code = 0x3C, .scale = 0.1, .offset = -40, .data_bytes = 2, .first_byte = 0,  .min = -20, .max = 1000, .units = celsius, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_61 = {.name = "Catalyst Temperature B2 S1", .pid_code = 0x3D, .scale = 0.1, .offset = -40, .data_bytes = 2, .first_byte = 0,  .min = -20, .max = 1000, .units = celsius, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_62 = {.name = "Catalyst Temperature B1 S2", .pid_code = 0x3E, .scale = 0.1, .offset = -40, .data_bytes = 2, .first_byte = 0,  .min = -20, .max = 1000, .units = celsius, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_63 = {.name = "Catalyst Temperature B2 S2", .pid_code = 0x3F, .scale = 0.1, .offset = -40, .data_bytes = 2, .first_byte = 0,  .min = -20, .max = 1000, .units = celsius, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_64 = {.name = "Defined in Appendix A", .pid_code = 0x40, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_65 = {.name = "Monitor status this driving cycle", .pid_code = 0x41, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_66 = {.name = "Control Module Voltage", .pid_code = 0x42, .scale = 0.001, .offset = 0, .data_bytes = 2, .first_byte = 0,  .min = 0, .max = 20, .units = volts, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_67 = {.name = "Absolute Load Value", .pid_code = 0x43, .scale = 0.3915686, .offset = 0, .data_bytes = 2, .first_byte = 0,  .min = 0, .max = 400, .units = percent, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_68 = {.name = "Cmd A/F Ratio", .pid_code = 0x44, .scale = 3.05e-05, .offset = 0, .data_bytes = 2, .first_byte = 0,  .min = 0, .max = 2, .units = lambda, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_69 = {.name = "Relative Throttle Position", .pid_code = 0x45, .scale = 0.3915686, .offset = 0, .data_bytes = 1, .first_byte = 0,  .min = 0, .max = 100, .units = percent, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_70 = {.name = "Ambient Air Temperature", .pid_code = 0x46, .scale = 1, .offset = -40, .data_bytes = 1, .first_byte = 0,  .min = -30, .max = 40, .units = celsius, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_71 = {.name = "Absolute Throttle Position B", .pid_code = 0x47, .scale = 0.3915686, .offset = 0, .data_bytes = 1, .first_byte = 0,  .min = 0, .max = 100, .units = percent, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_72 = {.name = "Absolute Throttle Position C", .pid_code = 0x48, .scale = 0.3915686, .offset = 0, .data_bytes = 1, .first_byte = 0,  .min = 0, .max = 100, .units = percent, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_73 = {.name = "Accelerator Pedal Position D", .pid_code = 0x49, .scale = 0.3915686, .offset = 0, .data_bytes = 1, .first_byte = 0,  .min = 0, .max = 100, .units = percent, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_74 = {.name = "Accelerator Pedal Position E", .pid_code = 0x4A, .scale = 0.3915686, .offset = 0, .data_bytes = 1, .first_byte = 0,  .min = 0, .max = 100, .units = percent, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_75 = {.name = "Accelerator Pedal Position F", .pid_code = 0x4B, .scale = 0.3915686, .offset = 0, .data_bytes = 1, .first_byte = 0,  .min = 0, .max = 100, .units = percent, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_76 = {.name = "Cmd Throttle Actuator Control", .pid_code = 0x4C, .scale = 0.3915686, .offset = 0, .data_bytes = 1, .first_byte = 0,  .min = 0, .max = 100, .units = percent, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_77 = {.name = "Engine Run Time w/ MIL", .pid_code = 0x4D, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_78 = {.name = "Engine Run Time Since DTC Cleared", .pid_code = 0x4E, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_79 = {.name = "External Test Equipment Configuration Information 1", .pid_code = 0x4F, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_80 = {.name = "External Test Equipment Configuration Information 2", .pid_code = 0x50, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_81 = {.name = "Type of fuel currently being utilized by the internal combustion engine", .pid_code = 0x51, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_82 = {.name = "Alcohol Fuel Percentage", .pid_code = 0x52, .scale = 0.3915686, .offset = 0, .data_bytes = 1, .first_byte = 0,  .min = 0, .max = 100, .units = percent, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_83 = {.name = "Abs Evap System Vapor Pressure", .pid_code = 0x53, .scale = 0.005, .offset = 0, .data_bytes = 2, .first_byte = 0,  .min = 0, .max = 330, .units = kpa, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_84 = {.name = "Evap System Vapor Pressure", .pid_code = 0x54, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_85 = {.name = "Short Term Secondary Fuel Trim B1", .pid_code = 0x55, .scale = 0.78125, .offset = -100, .data_bytes = 1, .first_byte = 1,  .min = -20, .max = 20, .units = percent, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_86 = {.name = "Long Term Secondary Fuel Trim B1", .pid_code = 0x56, .scale = 0.78125, .offset = -100, .data_bytes = 1, .first_byte = 1,  .min = -20, .max = 20, .units = percent, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_87 = {.name = "Short Term Secondary  Fuel Trim B2 ", .pid_code = 0x57, .scale = 0.78125, .offset = -100, .data_bytes = 1, .first_byte = 1,  .min = -20, .max = 20, .units = percent, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_88 = {.name = "Long Term Secondary Fuel Trim B2", .pid_code = 0x58, .scale = 0.78125, .offset = -100, .data_bytes = 1, .first_byte = 1,  .min = -20, .max = 20, .units = percent, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_89 = {.name = "Fuel Rail Pressure (absolute)", .pid_code = 0x59, .scale = 10, .offset = 0, .data_bytes = 2, .first_byte = 0,  .min = 0, .max = 655350, .units = kpa, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_90 = {.name = "Relative Accel Pedal Position", .pid_code = 0x5A, .scale = 0.3915686, .offset = 0, .data_bytes = 1, .first_byte = 0,  .min = 0, .max = 100, .units = percent, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_91 = {.name = "Hybrid Battery Charge Remain.", .pid_code = 0x5B, .scale = 0.3915686, .offset = 0, .data_bytes = 1, .first_byte = 0,  .min = 0, .max = 100, .units = percent, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_92 = {.name = "Engine Oil Temperature", .pid_code = 0x5C, .scale = 1, .offset = -40, .data_bytes = 1, .first_byte = 0,  .min = -20, .max = 200, .units = celsius, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_93 = {.name = "Fuel Injection Timing", .pid_code = 0x5D, .scale = 0.0078125, .offset = -210, .data_bytes = 2, .first_byte = 0,  .min = -210, .max = 300, .units = degrees, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_94 = {.name = "Engine Fuel Rate", .pid_code = 0x5E, .scale = 0.05, .offset = 0, .data_bytes = 2, .first_byte = 0,  .min = 0, .max = 3200, .units = l_per_h, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_95 = {.name = "Emission requirements to which vehicle is designed", .pid_code = 0x5F, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_96 = {.name = "Defined in Appendix A", .pid_code = 0x60, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_97 = {.name = "Demanded Percent Torque", .pid_code = 0x61, .scale = 1, .offset = -125, .data_bytes = 1, .first_byte = 0,  .min = -125, .max = 130, .units = percent, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_98 = {.name = "Actual Percent Torque", .pid_code = 0x62, .scale = 1, .offset = -125, .data_bytes = 1, .first_byte = 0,  .min = -125, .max = 130, .units = percent, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_99 = {.name = "Engine Reference Torque", .pid_code = 0x63, .scale = 1, .offset = 0, .data_bytes = 2, .first_byte = 0,  .min = 0, .max = 1000, .units = nm, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_100 = {.name = "Engine Percent Torque Data", .pid_code = 0x64, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_101 = {.name = "Auxiliary Inputs/Output Status", .pid_code = 0x65, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_102 = {.name = "Mass Air Flow Sensor ", .pid_code = 0x66, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = g_per_s, .available = false, .nested = saej1979_mass_air_flow_nest};
static saej1979_current_data_t saej1979_current_data_pid_103 = {.name = "Engine Coolant Temperature", .pid_code = 0x67, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = celsius, .available = false, .nested = saej1979_engine_coolant_temp_nest};
static saej1979_current_data_t saej1979_current_data_pid_104 = {.name = "Intake Air Temperature", .pid_code = 0x68, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = celsius, .available = false, .nested = saej1979_intake_air_temp_nest};
static saej1979_current_data_t saej1979_current_data_pid_105 = {.name = "Commanded EGR and EGR Error", .pid_code = 0x69, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_106 = {.name = "Cmd Diesel Intake Air Flow Control and Relative Intake Air Flow Position", .pid_code = 0x6A, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = percent, .available = false, .nested = saej1979_diesel_air_flow_nest};
static saej1979_current_data_t saej1979_current_data_pid_107 = {.name = "Exhaust Gas Recirculation Temperature", .pid_code = 0x6B, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_108 = {.name = "Commanded Throttle Actuator Control and Relative Throttle Position", .pid_code = 0x6C, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_109 = {.name = "Fuel Pressure Ctrl", .pid_code = 0x6D, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = kpa, .available = false, .nested = saej1979_fuel_pressure_ctrl_nest};
static saej1979_current_data_t saej1979_current_data_pid_110 = {.name = "Injection Pressure Control System", .pid_code = 0x6E, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = kpa, .available = false, .nested = saej1979_injection_pressure_ctrl_nest};
static saej1979_current_data_t saej1979_current_data_pid_111 = {.name = "Turbocharger Inlet Pressure", .pid_code = 0x6F, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = kpa, .available = false, .nested = saej1979_turbo_inlet_pressure_nest};
static saej1979_current_data_t saej1979_current_data_pid_112 = {.name = "Boost Pressure Control", .pid_code = 0x70, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = kpa, .available = false, .nested = saej1979_boost_pressure_nest};
static saej1979_current_data_t saej1979_current_data_pid_113 = {.name = "Variable Geometry Turbo Control", .pid_code = 0x71, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = kpa, .available = false, .nested = saej1979_vari_geometry_turbo_ctrl_nest};
static saej1979_current_data_t saej1979_current_data_pid_114 = {.name = "Wastegate Control", .pid_code = 0x72, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = percent, .available = false, .nested = saej1979_wastegate_ctrl_nest};
static saej1979_current_data_t saej1979_current_data_pid_115 = {.name = "Exhaust Pressure", .pid_code = 0x73, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = kpa, .available = false, .nested = saej1979_exhaust_pressure_nest};
static saej1979_current_data_t saej1979_current_data_pid_116 = {.name = "Turbocharger RPM", .pid_code = 0x74, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = rpm, .available = false, .nested = saej1979_turbo_rpm_nest};
static saej1979_current_data_t saej1979_current_data_pid_117 = {.name = "Turbocharger A Temperature", .pid_code = 0x75, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = celsius, .available = false, .nested = saej1979_turbo_temp_a_nest};
static saej1979_current_data_t saej1979_current_data_pid_118 = {.name = "Turbocharger B Temperature", .pid_code = 0x76, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = celsius, .available = false, .nested = saej1979_turbo_temp_b_nest};
static saej1979_current_data_t saej1979_current_data_pid_119 = {.name = "Charge Air Cooler Temperature", .pid_code = 0x77, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = celsius, .available = false, .nested = saej1979_charge_air_cooler_temp_nest};
static saej1979_current_data_t saej1979_current_data_pid_120 = {.name = "Exhaust Gas Temperature B1", .pid_code = 0x78, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = celsius, .available = false, .nested = saej1979_exhaust_gas_temp_a_nest};
static saej1979_current_data_t saej1979_current_data_pid_121 = {.name = "Exhaust Gas Temperature B2", .pid_code = 0x79, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = celsius, .available = false, .nested = saej1979_exhaust_gas_temp_b_nest};
static saej1979_current_data_t saej1979_current_data_pid_122 = {.name = "Particulate Filter B1", .pid_code = 0x7A, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_123 = {.name = "Particulate Filter B2", .pid_code = 0x7B, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_124 = {.name = "Particulate Filter Temperature", .pid_code = 0x7C, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_125 = {.name = "NOx NTE control area status", .pid_code = 0x7D, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_126 = {.name = "PM NTE control area status", .pid_code = 0x7E, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_127 = {.name = "Engine Run Time", .pid_code = 0x7F, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_128 = {.name = "Defined in Appendix A", .pid_code = 0x80, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_129 = {.name = "Engine Run Time for AECD #1 - #5", .pid_code = 0x81, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_130 = {.name = "Engine Run Time for AECD #6 - #10", .pid_code = 0x82, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_131 = {.name = "NOx Sensor", .pid_code = 0x83, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_132 = {.name = "Manifold Surface Temp", .pid_code = 0x84, .scale = 1, .offset = -40, .data_bytes = 1, .first_byte = 0,  .min = -20, .max = 70, .units = celsius, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_133 = {.name = "NOx Control System ", .pid_code = 0x85, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_134 = {.name = "Particulate Matter Sensor", .pid_code = 0x86, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_135 = {.name = "Intake Manifold Pressure", .pid_code = 0x87, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = kpa, .available = false, .nested = saej1979_intake_manifold_pressure_nest};
static saej1979_current_data_t saej1979_current_data_pid_136 = {.name = "SCR Inducement System", .pid_code = 0x88, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_137 = {.name = "Engine Run Time for AECD #11 - #15", .pid_code = 0x89, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_138 = {.name = "Engine Run Time for AECD #16 - #20", .pid_code = 0x8A, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_139 = {.name = "Aftertreatment Status", .pid_code = 0x8B, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_140 = {.name = "O2 Sensor (Wide Range)", .pid_code = 0x8C, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_141 = {.name = "Absolute Throttle Position G", .pid_code = 0x8D, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_142 = {.name = "Engine Friction - Percent Torque", .pid_code = 0x8E, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_143 = {.name = "Particulate Matter (PM) Sensor Output", .pid_code = 0x8F, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_144 = {.name = "WWH-OBD Vehicle OBD System Information", .pid_code = 0x90, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_145 = {.name = "WWH-OBD ECU OBD System Information", .pid_code = 0x91, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_146 = {.name = "Fuel system status (Compression Ignition)", .pid_code = 0x92, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_147 = {.name = "WWH-OBD Vehicle OBD Counters", .pid_code = 0x93, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_148 = {.name = "NOx control - driver inducement system status and counters", .pid_code = 0x94, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_149 = {.name = "SCR Catalyst NH3 Storage data", .pid_code = 0x95, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_150 = {.name = "Hydrocarbon Doser", .pid_code = 0x96, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_151 = {.name = "NOx Mass Emission Rate", .pid_code = 0x97, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_152 = {.name = "Exhaust Gas Temperature B1", .pid_code = 0x98, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_153 = {.name = "Exhaust Gas Temperature B2", .pid_code = 0x99, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_154 = {.name = "Hybrid/EV Vehicle System Data", .pid_code = 0x9A, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_155 = {.name = "Diesel Exhaust Fluid Sensor Output", .pid_code = 0x9B, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_156 = {.name = "O2 Sensor (Wide Range)", .pid_code = 0x9C, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_157 = {.name = "Fuel Rate", .pid_code = 0x9D, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_158 = {.name = "Engine Exhaust Flow Rate", .pid_code = 0x9E, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_159 = {.name = "Fuel System Percentage Use", .pid_code = 0x9F, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_160 = {.name = "Defined in Appendix A", .pid_code = 0xA0, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_161 = {.name = "NOx Sensor Corrected", .pid_code = 0xA1, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_162 = {.name = "Cylinder Fuel Rate", .pid_code = 0xA2, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_163 = {.name = "Evap System Vapor Pressure", .pid_code = 0xA3, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_164 = {.name = "Transmission Actual Gear", .pid_code = 0xA4, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_165 = {.name = "Diesel Exhaust Fluid Dosing", .pid_code = 0xA5, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_166 = {.name = "Vehicle Odometer", .pid_code = 0xA6, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_167 = {.name = "NOx Sensor", .pid_code = 0xA7, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_168 = {.name = "NOx Sensor Corrected", .pid_code = 0xA8, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_169 = {.name = "Motorcycle Input/Output Status", .pid_code = 0xA9, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_170 = {.name = "Vehicle Speed Limiter Set Speed", .pid_code = 0xAA, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_171 = {.name = "Alternative Fuel Vehicle Data", .pid_code = 0xAB, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_172 = {.name = "Maximum DEF Dosing Rate/Mode", .pid_code = 0xAC, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_173 = {.name = "Crankcase Ventilation Data", .pid_code = 0xAD, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_174 = {.name = "EVAP System Purge Pressure Sensor", .pid_code = 0xAE, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_175 = {.name = "Commanded/Target Fresh Air Flow", .pid_code = 0xAF, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};
static saej1979_current_data_t saej1979_current_data_pid_176 = {.name = "EGR Mass Flow", .pid_code = 0xB0, .scale = 0, .offset = 0, .data_bytes = 0, .first_byte = 0,  .min = 0, .max = 0, .units = NULL, .available = false, .nested = NULL};


static saej1979_current_data_t* saej1979_current_data[179] = {
&saej1979_current_data_pid_0,
&saej1979_current_data_pid_1,
&saej1979_current_data_pid_2,
&saej1979_current_data_pid_3,
&saej1979_current_data_pid_4,
&saej1979_current_data_pid_5,
&saej1979_current_data_pid_6,
&saej1979_current_data_pid_7,
&saej1979_current_data_pid_8,
&saej1979_current_data_pid_9,
&saej1979_current_data_pid_10,
&saej1979_current_data_pid_11,
&saej1979_current_data_pid_12,
&saej1979_current_data_pid_13,
&saej1979_current_data_pid_14,
&saej1979_current_data_pid_15,
&saej1979_current_data_pid_16,
&saej1979_current_data_pid_17,
&saej1979_current_data_pid_18,
&saej1979_current_data_pid_19,
&saej1979_current_data_pid_20,
&saej1979_current_data_pid_21,
&saej1979_current_data_pid_22,
&saej1979_current_data_pid_23,
&saej1979_current_data_pid_24,
&saej1979_current_data_pid_25,
&saej1979_current_data_pid_26,
&saej1979_current_data_pid_27,
&saej1979_current_data_pid_28,
&saej1979_current_data_pid_29,
&saej1979_current_data_pid_30,
&saej1979_current_data_pid_31,
&saej1979_current_data_pid_32,
&saej1979_current_data_pid_33,
&saej1979_current_data_pid_34,
&saej1979_current_data_pid_35,
&saej1979_current_data_pid_36,
&saej1979_current_data_pid_37,
&saej1979_current_data_pid_38,
&saej1979_current_data_pid_39,
&saej1979_current_data_pid_40,
&saej1979_current_data_pid_41,
&saej1979_current_data_pid_42,
&saej1979_current_data_pid_43,
&saej1979_current_data_pid_44,
&saej1979_current_data_pid_45,
&saej1979_current_data_pid_46,
&saej1979_current_data_pid_47,
&saej1979_current_data_pid_48,
&saej1979_current_data_pid_49,
&saej1979_current_data_pid_50,
&saej1979_current_data_pid_51,
&saej1979_current_data_pid_52,
&saej1979_current_data_pid_53,
&saej1979_current_data_pid_54,
&saej1979_current_data_pid_55,
&saej1979_current_data_pid_56,
&saej1979_current_data_pid_57,
&saej1979_current_data_pid_58,
&saej1979_current_data_pid_59,
&saej1979_current_data_pid_60,
&saej1979_current_data_pid_61,
&saej1979_current_data_pid_62,
&saej1979_current_data_pid_63,
&saej1979_current_data_pid_64,
&saej1979_current_data_pid_65,
&saej1979_current_data_pid_66,
&saej1979_current_data_pid_67,
&saej1979_current_data_pid_68,
&saej1979_current_data_pid_69,
&saej1979_current_data_pid_70,
&saej1979_current_data_pid_71,
&saej1979_current_data_pid_72,
&saej1979_current_data_pid_73,
&saej1979_current_data_pid_74,
&saej1979_current_data_pid_75,
&saej1979_current_data_pid_76,
&saej1979_current_data_pid_77,
&saej1979_current_data_pid_78,
&saej1979_current_data_pid_79,
&saej1979_current_data_pid_80,
&saej1979_current_data_pid_81,
&saej1979_current_data_pid_82,
&saej1979_current_data_pid_83,
&saej1979_current_data_pid_84,
&saej1979_current_data_pid_85,
&saej1979_current_data_pid_86,
&saej1979_current_data_pid_87,
&saej1979_current_data_pid_88,
&saej1979_current_data_pid_89,
&saej1979_current_data_pid_90,
&saej1979_current_data_pid_91,
&saej1979_current_data_pid_92,
&saej1979_current_data_pid_93,
&saej1979_current_data_pid_94,
&saej1979_current_data_pid_95,
&saej1979_current_data_pid_96,
&saej1979_current_data_pid_97,
&saej1979_current_data_pid_98,
&saej1979_current_data_pid_99,
&saej1979_current_data_pid_100,
&saej1979_current_data_pid_101,
&saej1979_current_data_pid_102,
&saej1979_current_data_pid_103,
&saej1979_current_data_pid_104,
&saej1979_current_data_pid_105,
&saej1979_current_data_pid_106,
&saej1979_current_data_pid_107,
&saej1979_current_data_pid_108,
&saej1979_current_data_pid_109,
&saej1979_current_data_pid_110,
&saej1979_current_data_pid_111,
&saej1979_current_data_pid_112,
&saej1979_current_data_pid_113,
&saej1979_current_data_pid_114,
&saej1979_current_data_pid_115,
&saej1979_current_data_pid_116,
&saej1979_current_data_pid_117,
&saej1979_current_data_pid_118,
&saej1979_current_data_pid_119,
&saej1979_current_data_pid_120,
&saej1979_current_data_pid_121,
&saej1979_current_data_pid_122,
&saej1979_current_data_pid_123,
&saej1979_current_data_pid_124,
&saej1979_current_data_pid_125,
&saej1979_current_data_pid_126,
&saej1979_current_data_pid_127,
&saej1979_current_data_pid_128,
&saej1979_current_data_pid_129,
&saej1979_current_data_pid_130,
&saej1979_current_data_pid_131,
&saej1979_current_data_pid_132,
&saej1979_current_data_pid_133,
&saej1979_current_data_pid_134,
&saej1979_current_data_pid_135,
&saej1979_current_data_pid_136,
&saej1979_current_data_pid_137,
&saej1979_current_data_pid_138,
&saej1979_current_data_pid_139,
&saej1979_current_data_pid_140,
&saej1979_current_data_pid_141,
&saej1979_current_data_pid_142,
&saej1979_current_data_pid_143,
&saej1979_current_data_pid_144,
&saej1979_current_data_pid_145,
&saej1979_current_data_pid_146,
&saej1979_current_data_pid_147,
&saej1979_current_data_pid_148,
&saej1979_current_data_pid_149,
&saej1979_current_data_pid_150,
&saej1979_current_data_pid_151,
&saej1979_current_data_pid_152,
&saej1979_current_data_pid_153,
&saej1979_current_data_pid_154,
&saej1979_current_data_pid_155,
&saej1979_current_data_pid_156,
&saej1979_current_data_pid_157,
&saej1979_current_data_pid_158,
&saej1979_current_data_pid_159,
&saej1979_current_data_pid_160,
&saej1979_current_data_pid_161,
&saej1979_current_data_pid_162,
&saej1979_current_data_pid_163,
&saej1979_current_data_pid_164,
&saej1979_current_data_pid_165,
&saej1979_current_data_pid_166,
&saej1979_current_data_pid_167,
&saej1979_current_data_pid_168,
&saej1979_current_data_pid_169,
&saej1979_current_data_pid_170,
&saej1979_current_data_pid_171,
&saej1979_current_data_pid_172,
&saej1979_current_data_pid_173,
&saej1979_current_data_pid_174,
&saej1979_current_data_pid_175,
&saej1979_current_data_pid_176,
};


/*** Generated with current_data_array_generator_script.py. ***/


/**********		STATIC FUNCTION DECLRATIONS		**********/
float prv_celsius_to_farenheit(float celsius_val);
/**********		STATIC FUNCTION DEFINITIONS		**********/
float prv_celsius_to_farenheit(float celsius_val)
{
	float farenheit = (celsius_val * 1.8) + 32;
	return farenheit;
}
/**********		GLOBAL FUNCTION DEFINITIONS		**********/
int32_t can_uds_process_raw_data(can_rx_buffer_entry_t* input)
{
	uint32_t id = input->R0.bit.ID;
	uint8_t pid = input->data[2];
	uint32_t data = 0;
	uint32_t num_params = 255;			//TODO: calculate num_params.
    for (uint8_t i = 0; i < num_params; i++)
	{
		if (pid == saej1979_current_data[i]->pid_code)
		{
			if (saej1979_current_data[i]->data_bytes == 1)
			{
				data = input->data[3];
			}
			else if (saej1979_current_data[i]->data_bytes == 2)
			{
				data = (input->data[3] << 8) | input->data[4];
			}
			//data = saej1979_current_data[i]->conversion_func(data);
			return data;
		}
	}
	return 0;
}

saej1979_current_data_t* saej1979_get_current_data_lut_by_pid(uint8_t pid)
{
	return saej1979_current_data[pid];
}

void can_uds_change_pressure_units(const char* units)
{
	float conversion = 0;
	const char** new_lbl;
	if (!strcmp(units, psi))
	{
		conversion = kpa_to_psi;
		new_lbl = &psi;
	}
	else if (!strcmp(units, bar))
	{
		conversion = kpa_to_bar;
		new_lbl = &bar;
	}
	else
	{
		return;
	}

	for (uint8_t i = 0; i < 176; i++)
	{
		saej1979_current_data_t* y = saej1979_get_current_data_lut_by_pid(i);
		if (y->units == kpa)
		{
			y->min *= conversion;
			y->max *= conversion;
			y->scale *= conversion;
			y->offset *= conversion;	//TODO: check that this math works.
			y->units = *new_lbl;
			/* Check if this one is nested. */
			if (y->nested != NULL)
			{
				/* If it is, cycle through its parameters. */
				for (uint8_t idx = 0; idx < 8; idx++)
				{
					saej1979_current_data_t* x = y->nested[idx];
					if (x == NULL) { continue; }
					if (x->available)
					{
						x->min *= conversion;
						x->max *= conversion;
						x->scale *= conversion;
						x->offset *= conversion;	//TODO: check that this math works.
						x->units = *new_lbl;
					}
				}

			}
		}
	}
}
void can_uds_change_temperature_units(const char* units)
{
	float conversion = 0;
	if (!strcmp(units, farenheit))
	{
		conversion = c_to_f;
	}
	else
	{
		return;
	}

	for (uint8_t i = 0; i < 176; i++)
	{
		saej1979_current_data_t* y = saej1979_get_current_data_lut_by_pid(i);
		if (y->units == celsius)
		{
			y->min *= conversion;
			y->max *= conversion;
			y->scale *= conversion;
			y->units = farenheit;
			/* Check if this one is nested. */
			if (y->nested != NULL)
			{
				/* If it is, cycle through its parameters. */
				for (uint8_t idx = 0; idx < 8; idx++)
				{
					saej1979_current_data_t* x = y->nested[idx];
					if (x == NULL) { continue; }
					if (x->available)
					{
						x->min *= conversion;
						x->max *= conversion;
						x->scale *= conversion;
						x->units = farenheit;
					}
				}
			}
		}
	}
}
