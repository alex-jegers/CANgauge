/**
 * @file template.h
 * @brief 
 * @date yyyy-mm-dd
 * 
 */
#ifndef _UI_LOADING_SCR_H_
#define _UI_LOADING_SCR_H_

#ifdef __cplusplus
extern "C" {
#endif

/**********     INCLUDES        **********/

/**********     TYPEDEFS         **********/

/**********     DEFINES      **********/

/**********     GLOBAL VARIABLE DECLRATIONS     **********/

/**********		GLOBAL FUNCTION DECLRATIONS		**********/
/**
 * @brief
 */
void ui_loading_scr_init();

/**
 * @brief 
 * 
 */
void ui_loading_scr_load();

/**
 * @brief 
 * 
 */
void ui_loading_scr_set_text(const char* txt);

/**
 * @brief 
 * 
 */
void ui_loading_scr_delete();

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif  //_UI_LOADING_SCR_H_