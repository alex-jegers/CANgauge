
#ifndef _DATA_LOGGER_H_
#define _DATA_LOGGER_H_

#ifdef __cplusplus
extern "C" {
#endif

/**********     INCLUDES        **********/
#include "cangauge.h"
#include "can_uds_def.h"
#include "can_uds.h"

/**********     TYPEDEFS         **********/
typedef enum
{
	DATA_LOGGER_ERROR_NO_MEM,
	DATA_LOGGER_ERROR_FILE_SYS_ERR,
	DATA_LOGGER_ERROR_HEAP_ERR,
}data_logger_error_code_t;

typedef struct
{
	saej1979_current_data_t* data[4];
	uint32_t period_ms;
	bool run;
	TaskHandle_t task_handle;
	void (*error_cb)(data_logger_error_code_t error_code);
}data_logger_handle_t;



/**********     DEFINES      **********/

/**********     GLOBAL VARIABLE DECLRATIONS     **********/

/**********		GLOBAL FUNCTION DECLRATIONS		**********/
/**
 * name:
 *      desc:
 *      params:
 *      returns:
 */
bool data_logger_start_recording(data_logger_handle_t* handle);
void data_logger_stop_recording(data_logger_handle_t* hndl);
void data_logger_set_data(data_logger_handle_t* hndl, saej1979_current_data_t* data_to_record[4]);
void data_logger_set_period(data_logger_handle_t* hndl, uint32_t period_ms);
void data_logger_set_error_cb(data_logger_handle_t* hndl, void (*func)(data_logger_error_code_t ec));
bool data_logger_recording(data_logger_handle_t* hndl);



#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif  //_TEMPLATE_H_
