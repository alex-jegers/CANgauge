
#ifndef _FILE_MNGR_H_
#define _FILE_MNGR_H_

#ifdef __cplusplus
extern "C" {
#endif

/**********     INCLUDES        **********/
#include "cangauge.h"

/**********     TYPEDEFS         **********/

/**********     DEFINES      **********/
#define FILE_MNGR_CONFIG_FILE_PATH		"0:/System Data.txt"

/**********     GLOBAL VARIABLE DECLRATIONS     **********/

/**********		GLOBAL FUNCTION DECLRATIONS		**********/
/**
 * name:
 *      desc:
 *      params:
 *      returns:
 */
bool file_mngr_run();
bool file_mngr_stop();
void file_mngr_notify();

/**
 * sys_mem_get_config_data:
 * 		desc: gets a line of data from the config file.
 * 		params:
 * 			data_to_get: a pointer to a string of the header of the data wanted (i.e. "LAST GAUGES STATE").
 * 			data_buf: a buffer where the line will be written to.
 * 		returns: the number of bytes written to data_buf.
 */
uint32_t file_mngr_get_config_data(const char* data_to_get, char* data_buf);

/**
 * sys_mem_set_config_data:
 * 		desc: overwrites new data to the config file.
 * 		params:
 * 			data: the line of data to write (i.e. "LAST GAUGES STATE,0,0,0,0).
 */
void file_mngr_set_config_data(char* data);

FRESULT file_mngr_create_default_config_file();
FRESULT file_mngr_config_file_exists();

/**
 * sys_mem_csv_split
 * 		desc: returns an element (index) from a string of comma separated values.
 */
char* file_mngr_csv_split(char* str, uint32_t index);

/**
 * sys_mem_csv_get_num_cols	TODO:find a better file to put this.
 * 		desc:
 */
uint32_t file_mngr_csv_get_num_cols(char* str);



#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif  //_TEMPLATE_H_
