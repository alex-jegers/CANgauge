/**
 * @file pwr_monitor.h
 * @brief Responsible for anything related to the power of CANgauge primarily monitoring the input voltage
 * and entering and exiting low power mode. The transfer function for the monitor op amp is Vout = (Vin * 0.424302) - 3.792.
 * Vout is referenced to 3.3V in the ADC so the ADC counts = (Vout / 3.3) * 65535. The op amp measures after the filter and
 * transient clamp so the the voltage measurment is more representative of what the buck converter is seeing rather than
 * the true battery voltage. Counts = (8426.252 * Vbattery) - 75305.67. Vbattery = (counts / 8426.252) + 8.937.
 *
 */
#ifndef _PWR_MONITOR_H_
#define _PWR_MONITOR_H_

#ifdef __cplusplus
extern "C" {
#endif

/**********     INCLUDES        **********/
#include "cangauge.h"

/**********     TYPEDEFS      **********/

/**********     DEFINES      **********/

/**********     GLOBAL VARIABLE DECLRATIONS     **********/

/**********		GLOBAL FUNCTION DECLRATIONS		**********/
/**
 * @brief Creates the power monitor task that monitors the input voltage and
 * puts the device into low power mode if it drops below a certain threshold.
 * Conversely, it restarts the device if the input voltage rises above the
 * threshold while in low power mode.
 */
void pwr_monitor_run(uint8_t priority);

/**
 * @brief Pauses the power monitor so the device will not enter low power mode
 * if the input voltages drops below the threshold.
 */
void pwr_monitor_suspend();

/**
 * @brief Resumes the power monitor task.
 */
void pwr_monitor_resume();

/**
 * @brief Puts the gauge into low power mode by setting the system clock to
 * the HSI and adjusts the SysTick interval accordingly, disabling all the
 * PLLs (and therefore the peripherals that they clock), changes the system
 * power mode to 3 (lowest), disables the CAN transceivers, turns off the
 * LCD back light. Prior to performing all of this, the function will call
 * any function callbacks assigned with pwr_monitor_add_low_pwr_mode_cb.
 * This is used to disable any tasks that use resources that wont
 * be available in this state.
 */
void pwr_monitor_enter_low_pwr_mode();

/**
 * @brief Adds a function to be called prior to the gauge entering low power mode.
 * This is used to suspend or delete tasks that wont be able to function in low
 * power mode.
 */
bool pwr_monitor_add_low_pwr_mode_cb(void (*func)());

/**
 * @brief Remove a function from the list of functions to call when entering low
 * power mode.
 * @attention UNIMPLEMENTED, DONT USE.
 */
bool pwr_monitor_remove_low_pwr_mode_cb(void (*func()));

/**
 * @brief Returns the value in volts that the gauge is set to turn on at.
 */
float pwr_monitor_get_on_th_volts();

/**
 * @brief Returns the value in volts that the gauge is set to turn off at.
 */
float pwr_monitor_get_off_th_volts();

/**
 * @brief Set the thresholds that the gauge should automatically turn on and
 * off at. The off threshold should be at least 0.5V less than the on threshold
 * otherwise the gauge will chatter due to a voltage jump when the gauge enters
 * low power mode. If off_th_volts is not < on_th_volts - 0.5 this function
 * will override the value passed for off_th_volts to satisfy said condition.
 * @param off_th_volts The level that the gauge should automatically turn off at
 * expressed in volts.
 * @param on_th_volts The level that the gauge should automatically turn on at
 * expressed in volts.
 */
void pwr_monitor_set_auto_on_off_th(float off_th_volts, float on_th_volts);

/**
 * @returns The last input voltage sample converted to volts, samples happen approx
 * every 1 second.
 */
float pwr_monitor_get_last_conversion_volts();


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif  //_PWR_MONITOR_H_
