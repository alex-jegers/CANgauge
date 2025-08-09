/*
 * system_cm4.h
 *
 *  Created on: May 31, 2024
 *      Author: awjpp
 */

#ifdef CORE_CM4

#ifndef CORE_SYSTEM_CM4_SYSTEM_CM4_H_
#define CORE_SYSTEM_CM4_SYSTEM_CM4_H_

#include "stm32h745xx.h"
#include "application/app_shared_mem.h"



void system_init();
void system_run();
void system_hsem_handler();


#endif /* CORE_SYSTEM_CM4_SYSTEM_CM4_H_ */

#endif //CORE_CM4
