/*
 * app_dtc_reader_cm7.c
 *
 *  Created on: April 25, 2025
 *      Author: awjpp
 */

#include "app_dtc_reader_cm4.h"
#include "ui/ui_dtc_reader.h"

 void app_dtc_reader_run_cm4()
 {
	 while (!hsem_wait_bool(5,0))
	 {

	 }
	 hsem_clear_int(5);
 }
 
