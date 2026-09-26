/**
 * @file loading_scr.h
 * @brief This file is responsible for all the application level initialization
 * including odds and ends that don't really fit into any other application
 * level files like assigning and handling some LVGL UI function callbacks.
 * @date 2026-09-12
 * 
 */
#ifndef _LOADING_SCR_H_
#define _LOADING_SCR_H_

#ifdef __cplusplus
extern "C" {
#endif

/**********     INCLUDES        **********/

/**********     TYPEDEFS         **********/

/**********     DEFINES      **********/

/**********     GLOBAL VARIABLE DECLRATIONS     **********/

/**********		GLOBAL FUNCTION DECLRATIONS		**********/
/**
 * @brief Displays the loading screen and starts the application level
 *      initialization process.
 */
void cg_obd_initialize();



#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif  //_LOADING_SCR_H_
