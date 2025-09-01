/*
 * system.h
 *
 *  Created on: May 31, 2024
 *      Author: awjpp
 */

#ifdef CORE_CM7

#ifndef CORE_SYSTEM_CM7_SYSTEM_H_
#define CORE_SYSTEM_CM7_SYSTEM_H_

#include "stm32h745xx.h"




void system_init();
void system_run();
void system_set_app_ptr(void (*app_ptr)());

void system_hsem_handler();


#endif /* CORE_SYSTEM_CM7_SYSTEM_H_ */

#endif	//CORE_CM7
