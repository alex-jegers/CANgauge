
#include "stm32_exti.h"


static void (*exti0_int_hanlder)();
static void (*exti1_int_hanlder)();
static void (*exti2_int_hanlder)();
static void (*exti3_int_hanlder)();
static void (*exti4_int_hanlder)();
static void (*exti9_5_int_hanlder)();
static void (*exti15_10_int_hanlder)();

void exti_init()
{

}

void exti_enable_io_interrupt(uint32_t exti_event_x, exti_edge_t edge)
{
    if (edge == EXTI_EDGE_RISING || edge == EXTI_EDGE_BOTH)
    {
        EXTI->RTSR1 |= exti_event_x;
    }

    if (edge == EXTI_EDGE_FALLING || edge == EXTI_EDGE_BOTH)
    {
        EXTI->FTSR1 |= exti_event_x;
    }

    EXTI->IMR1 |= exti_event_x;

    if (exti_event_x == EXTI_EVENT_IO_PIN0)
    {
    	NVIC_EnableIRQ(EXTI0_IRQn);
    }
    else if (exti_event_x == EXTI_EVENT_IO_PIN1)
	{
    	NVIC_EnableIRQ(EXTI1_IRQn);
	}
    else if (exti_event_x == EXTI_EVENT_IO_PIN2)
	{
    	NVIC_EnableIRQ(EXTI2_IRQn);
	}
    else if (exti_event_x == EXTI_EVENT_IO_PIN3)
	{
    	NVIC_EnableIRQ(EXTI3_IRQn);
	}
    else if (exti_event_x == EXTI_EVENT_IO_PIN4)
	{
    	NVIC_EnableIRQ(EXTI4_IRQn);
	}
    else if (exti_event_x >= EXTI_EVENT_IO_PIN5 )
	{
    	NVIC_EnableIRQ(EXTI9_5_IRQn);
	}

    if (exti_event_x >= EXTI_EVENT_IO_PIN10)
	{
    	NVIC_EnableIRQ(EXTI15_10_IRQn);
	}
}

uint32_t exti_io_intflags()
{
	return EXTI->PR1 & 0xFFFF;
}

void exti_clear_interrupt(uint32_t exti_event_x)
{
	EXTI->PR1 = exti_event_x;
}


void exti0_assign_int_handler(void (*func)())
{
    exti0_int_hanlder = func;
}
void exti1_assign_int_handler(void (*func)())
{
    exti1_int_hanlder = func;
}
void exti2_assign_int_handler(void (*func)())
{
    exti2_int_hanlder = func;
}
void exti3_assign_int_handler(void (*func)())
{
    exti3_int_hanlder = func;
}
void exti4_assign_int_handler(void (*func)())
{
    exti4_int_hanlder = func;
}

void exti9_5_assign_int_handler(void (*func)())
{
    exti9_5_int_hanlder = func;
}

void exti15_10_assign_int_handler(void (*func)())
{
    exti15_10_int_hanlder = func;
}

void EXTI0_IRQHandler()
{
    if (exti0_int_hanlder)
    {
        exti0_int_hanlder();
    }
}

void EXTI1_IRQHandler()
{
    if (exti1_int_hanlder)
    {
        exti1_int_hanlder();
    }
}

void EXTI2_IRQHandler()
{
    if (exti2_int_hanlder)
    {
        exti2_int_hanlder();
    }
}

void EXTI3_IRQHandler()
{
    if (exti3_int_hanlder)
    {
        exti3_int_hanlder();
    }
}

void EXTI4_IRQHandler()
{
    if (exti4_int_hanlder)
    {
        exti4_int_hanlder();
    }
}

void EXTI9_5_IRQHandler()
{
    if (exti9_5_int_hanlder)
    {
        exti9_5_int_hanlder();
    }

    //NVIC_ClearPendingIRQ(EXTI9_5_IRQn);
    exti_clear_interrupt(EXTI_EVENT_IO_PIN9 | EXTI_EVENT_IO_PIN8
    					| EXTI_EVENT_IO_PIN7 | EXTI_EVENT_IO_PIN6
						| EXTI_EVENT_IO_PIN5);
}


void EXTI15_10_IRQHandler()
{
    if (exti15_10_int_hanlder)
    {
        exti15_10_int_hanlder();
    }
}
