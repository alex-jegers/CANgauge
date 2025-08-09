/*
 * system_cm4.c
 *
 *  Created on: May 31, 2024
 *      Author: awjpp
 */
#ifdef CORE_CM4

#include "system/cm4/system_cm4.h"
#include "application/cm4/app_can_get_baud_rate_cm4.h"
#include "application/cm4/app_gauges_cm4.h"
#include "drivers/stm32_hsem.h"
#include "drivers/stm32_hsem.h"
#include "drivers/stm32_rcc.h"
#include "drivers/stm32_io.h"

/**********     TYPEDEFS     **********/
typedef enum
{
	APP_CAN_BAUD_RATE
}system_cm4_app_t;

typedef struct system_app_info_t
{
	void (*run_func)();
	void (*stop_func)();
	bool is_running;
	bool stop_running;
};

/**********     DEFINES     **********/
#define SCB_CPACR_CP10_FULL_ACCESS			0x3 << 20
#define SCB_CPACR_CP11_FULL_ACCESS			0x3 << 22

#define HSEM_APP_GET_CAN_BAUD_RATE_Msk			1 << 0		//HSEM 0
#define HSEM_STARTUP_Msk						1 << 1		//HSEM 1 (non-interrupt).
#define HSEM_APP_GAUGES_RUN_Msk					1 << 2		//HSEM 2
#define HSEM_APP_DTC_READER_RUN_Msk				1 << 3		//HSEM 3


/**********     STATIC VARIABLES     **********/
static uint32_t system_cm4_app_request;


/**********     STATIC FUNCTION DECLARATIONS     **********/
static void system_set_app_request(uint32_t app_msk);	//sets a new app run request.
static uint32_t system_get_app_request();				//returns the lowest number app run request.
static void system_clear_app_request();					//clears an app request so it doesnt run again.

/**********     STATIC FUNCTION DEFINITIONS     **********/
static void system_set_app_request(uint32_t app_msk)
{
	system_cm4_app_request |= app_msk;
}

static uint32_t system_get_app_request()
{
	for (uint32_t i = 0; i < 32; i++)
	{
		if ((((1 << i) & system_cm4_app_request)) != 0)
		{
			return ((1 << i) & system_cm4_app_request);
		}
	}
	return 0;
}

static void system_clear_app_request(uint32_t app_msk)
{
	system_cm4_app_request &= ~(app_msk);
}

/**********     GLOBAL FUNCTION DEFINITIONS     **********/
void system_init()
{
    /*Enable FPU (I do this in CM7, not sure if it's needed here as well.*/
	SCB->CPACR = SCB_CPACR_CP10_FULL_ACCESS | SCB_CPACR_CP11_FULL_ACCESS;		//enables the FPU.

	hsem_wait_void(1,0);
	hsem_lock(1, 0);

	/*Initialize D2 clocks.*/
	rcc_c2_clock_config();

	/*Enable HSEM interrupts.*/
	hsem_enable_interrupt(HSEM_APP_GET_CAN_BAUD_RATE_Msk);
	hsem_enable_interrupt(HSEM_APP_GAUGES_RUN_Msk);
	hsem_enable_interrupt(HSEM_APP_DTC_READER_RUN_Msk);
	hsem2_assign_int_handler(system_hsem_handler);
	NVIC_EnableIRQ(HSEM2_IRQn);

	/*Signal that initialization is done.*/
	hsem_signal(1,0);
}

void system_run()
{
	while (1)
	{
		if (system_get_app_request() == HSEM_APP_GET_CAN_BAUD_RATE_Msk)
		{
			app_can_baud_rate_run();
			system_clear_app_request(HSEM_APP_GET_CAN_BAUD_RATE_Msk);

		}

		if (system_get_app_request() == HSEM_APP_GAUGES_RUN_Msk)
		{
			app_gauges_run();
			system_clear_app_request(HSEM_APP_GAUGES_RUN_Msk);
		}

		if (system_get_app_request() == HSEM_APP_DTC_READER_RUN_Msk)
		{
			app_dtc_reader_run_cm4();
			system_clear_app_request(HSEM_APP_DTC_READER_RUN_Msk);
		}
	}
}

void system_hsem_handler()
{
	uint32_t ir = hsem_get_status();

	if (ir & HSEM_APP_GET_CAN_BAUD_RATE_Msk)
	{
		system_set_app_request(HSEM_APP_GET_CAN_BAUD_RATE_Msk);
	}

	if (ir & HSEM_APP_GAUGES_RUN_Msk)
	{
		system_set_app_request(HSEM_APP_GAUGES_RUN_Msk);
	}

	if (ir & HSEM_APP_DTC_READER_RUN_Msk)
	{
		system_set_app_request(HSEM_APP_DTC_READER_RUN_Msk);
	}
}


#endif	//CORE_CM4
