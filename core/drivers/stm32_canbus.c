
/***********	INCLUDES	************/
#include "stm32_canbus.h"
#include "stm32_io.h"
#include <stdint.h>
#include <stddef.h>
#include <string.h>

/***********	TYPEDEFS	************/
typedef enum
{
	HSE_CLK		= 0x0 << RCC_D2CCIP1R_FDCANSEL_Pos,
	PLL1_Q		= 0x1 << RCC_D2CCIP1R_FDCANSEL_Pos,
	PLL2_Q		= 0x2 << RCC_D2CCIP1R_FDCANSEL_Pos,
}canbus_kernel_clk_t;

typedef enum
{
	CAN_ERROR_CODE_NO_ERROR,
	CAN_ERROR_CODE_STUFF_ERROR,
	CAN_ERROR_CODE_FORM_ERROR,
	CAN_ERROR_CODE_ACK_ERROR,
	CAN_ERROR_CODE_BIT1_ERROR,
	CAN_ERROR_CODE_BIT0_ERROR,
	CAN_ERROR_CODE_CRC_ERROR,
	CAN_ERROR_CODE_NO_CHANGE,
}can_error_code_t;

/***********	DEFINES	************/
#define CAN_BTP_TSEG2_1M		1
#define CAN_BTP_TSEG1_1M		6
#define CAN_BTP_BRP_1M			6
#define CAN_BTP_SJW_1M			3

#define CAN_BTP_TSEG2_500K		2
#define CAN_BTP_TSEG1_500K		13
#define CAN_BTP_BRP_500K		6
#define CAN_BTP_SJW_500K		3

#define CAN_BTP_TSEG2_250K		9
#define CAN_BTP_TSEG1_250K		22
#define CAN_BTP_BRP_250K		6
#define CAN_BTP_SJW_250K		3

#define CAN_BTP_TSEG2_125K		15
#define CAN_BTP_TSEG1_125K		48
#define CAN_BTP_BRP_125K		6
#define CAN_BTP_SJW_125K		3



#define CAN_CCU_CDIV_Val		0x5 << FDCANCCU_CCFG_CDIV_Pos

/***********	STATIC VARIABLES DEFINITIONS	************/
struct can1_tx_timing_struct
{
	uint32_t interval;
	uint32_t last_time;
	bool active;
	bool index_used;
};

/***********	STATIC VARIABLES DECLARATIONS	************/
static bool can1_state = false;
static bool can2_state = false;

static uint32_t can_timer_ms = 0;

static can_rx_buffer_entry_t (*can1_rx_buffer)[CAN1_RX_BUFFER_ELEMENTS] = (can_rx_buffer_entry_t(*)[CAN1_RX_BUFFER_ELEMENTS])CAN1_RX_BUFFER_ADDR;
static can_rx_buffer_entry_t (*can1_rx_fifo0)[CAN1_RX_FIFO0_ELEMENTS] = (can_rx_buffer_entry_t(*)[CAN1_RX_FIFO0_ELEMENTS])CAN1_RX_FIFO0_ADDR;
static can_rx_buffer_entry_t (*can1_rx_fifo1)[CAN1_RX_FIFO1_ELEMENTS] = (can_rx_buffer_entry_t*)CAN1_RX_FIFO1_ADDR;

static can_rx_buffer_entry_t (*can2_rx_buffer)[CAN2_RX_BUFFER_ELEMENTS] = (can_rx_buffer_entry_t(*)[CAN2_RX_BUFFER_ELEMENTS])CAN2_RX_BUFFER_ADDR;
static can_rx_buffer_entry_t (*can2_rx_fifo0)[CAN2_RX_FIFO0_ELEMENTS] = (can_rx_buffer_entry_t(*)[CAN2_RX_FIFO0_ELEMENTS])CAN2_RX_FIFO0_ADDR;
static can_rx_buffer_entry_t (*can2_rx_fifo1)[CAN2_RX_FIFO1_ELEMENTS] = (can_rx_buffer_entry_t(*)[CAN2_RX_FIFO1_ELEMENTS])CAN2_RX_FIFO1_ADDR;
static can_tx_buffer_entry_t (*can2_tx_buffer)[CAN2_TX_BUFFER_ELEMENTS] = (can_tx_buffer_entry_t(*)[CAN2_TX_BUFFER_ELEMENTS])CAN2_TX_BUFFER_ADDR;
static can_std_id_filter_t (*can2_std_id_filter)[CAN2_STD_ID_FILTER_ELEMENTS] = (can_std_id_filter_t(*)[CAN2_STD_ID_FILTER_ELEMENTS])CAN2_STD_ID_FILTER_ADDR;
static can_ext_id_filter_t (*can2_ext_id_filter)[CAN2_EXT_ID_FILTER_ELEMENTS] = (can_ext_id_filter_t(*)[CAN2_EXT_ID_FILTER_ELEMENTS])CAN2_EXT_ID_FILTER_ADDR;


static struct can1_tx_timing_struct can1_tx_timer[CAN1_TX_BUFFER_ELEMENTS];

static uint32_t can1_ext_id_filter_list[CAN1_EXT_ID_FILTER_ELEMENTS * 2];
static uint32_t can2_ext_id_filter_list[CAN2_EXT_ID_FILTER_ELEMENTS * 2];

static void (*can1_rx_rf0n_ir_cb)();
static void (*can2_rx_rf0n_ir_cb)();
static void (*can1_rx_rf0f_ir_cb)();
static void (*can2_rx_rf0f_ir_cb)();

/***********	STATIC FUNCTION DECLARATIONS	************/
/*Control and initialization functions.*/
static void canbus_kernel_clk_select(canbus_kernel_clk_t clk);									//selects a kernel clock to run off of.
static void can_init_msg_ram();
static void can_set_listen_only(FDCAN_GlobalTypeDef* canbus);									//puts the device in listen only mode.
static void can_clear_listen_only(FDCAN_GlobalTypeDef* canbus);									//takes the device out of listen only mode.

/*Status related functions.*/
static uint32_t can_get_int_status(FDCAN_GlobalTypeDef* canbus);								//returns the interrupt register.
static void can_clear_int_status(FDCAN_GlobalTypeDef* canbus, uint32_t mask);					//clears interrupt bits.
static can_error_code_t can_get_last_error_code(FDCAN_GlobalTypeDef* canbus);

/*TX related functions.*/
static int32_t can_tx(FDCAN_GlobalTypeDef* canbus, uint8_t index);								//immediately transmits a message from the tx buffer of a given index.
static can_tx_buffer_entry_t* can_get_buffer_entry(FDCAN_GlobalTypeDef* canbus, uint8_t index);

/*RX related functions.*/
static uint32_t can_get_fifo0_fill_level(FDCAN_GlobalTypeDef* canbus);							//returns the fill level of FIFO0.
static uint32_t can_get_fifo1_fill_level(FDCAN_GlobalTypeDef* canbus);							//returns the fill level of FIFO1.
static can_rx_buffer_entry_t* can_get_rx_fifo0_entry(FDCAN_GlobalTypeDef* canbus, uint32_t index);	//returns rx_buffer_entry from specified index of fifo0.


/*Filter related functions.*/
static can_ext_id_filter_t* can_get_ext_id_filter(FDCAN_GlobalTypeDef* canbus, uint8_t index);	//returns a filter element at specified index.


/***********	STATIC FUNCTION DEFINTIONS	************/
static void canbus_kernel_clk_select(canbus_kernel_clk_t clk)
{
	RCC->D2CCIP1R &= ~(0x3 << RCC_D2CCIP1R_FDCANSEL_Pos);		//clear the bits.
	RCC->D2CCIP1R |= clk;
}

static int32_t can_tx(FDCAN_GlobalTypeDef* canbus, uint8_t index)
{
	canbus->TXBAR = 1 << index;		//Request next transfer.
}

static can_tx_buffer_entry_t* can_get_buffer_entry(FDCAN_GlobalTypeDef* canbus, uint8_t index)
{
	can_tx_buffer_entry_t* dest_addr = NULL;
	if (canbus == FDCAN1)
	{
		dest_addr = (can_tx_buffer_entry_t*)((uint8_t*)CAN1_TX_BUFFER_ADDR + (index * sizeof(can_tx_buffer_entry_t)));
	}

	if (canbus == FDCAN2)
	{
		dest_addr = (can_tx_buffer_entry_t*)((uint8_t*)CAN2_TX_BUFFER_ADDR + (index * sizeof(can_tx_buffer_entry_t)));
	}

	return dest_addr;
}

static void can_set_listen_only(FDCAN_GlobalTypeDef* canbus)
{
	canbus->CCCR |= FDCAN_CCCR_MON;
}

static void can_clear_listen_only(FDCAN_GlobalTypeDef* canbus)
{
	canbus->CCCR &= ~(FDCAN_CCCR_MON);
}

static void can_init_msg_ram()
{
	for (uint32_t i = CAN_MSG_RAM_BASE_ADDR; i < CAN_MSG_RAM_END_ADDR; i+=4)
	{
		*(uint32_t*)i = 0;
	}

	/*RX fifo0 start address, mode and size.*/
	FDCAN1->RXF0C = (uint32_t)can1_rx_fifo0 - CAN_MSG_RAM_BASE_ADDR;				//F0SA, FIFO 0 Start Address.
	FDCAN1->RXF0C |= CAN1_RX_FIFO0_ELEMENTS << FDCAN_RXF0C_F0S_Pos;

	/*RX FIFO1 start address, mode, and size.*/
	FDCAN1->RXF1C = (uint32_t)can1_rx_fifo1 - CAN_MSG_RAM_BASE_ADDR;
	FDCAN1->RXF1C |= CAN1_RX_FIFO1_ELEMENTS << FDCAN_RXF1C_F1S_Pos;

	/*RX buffer start address.*/
	FDCAN1->RXBC = (uint32_t)can1_rx_buffer - CAN_MSG_RAM_BASE_ADDR;

	/*Sets the size of data field for the RX/TX buffer, operating in CAN2.0B not FD, so this is always 8 bytes.*/
	FDCAN1->RXESC = FDCAN_RXESC_F0DS_DATA8 << FDCAN_RXESC_F0DS_Pos;
	FDCAN1->TXESC = FDCAN_TXESC_TBDS_DATA8 << FDCAN_TXESC_TBDS_Pos;

	/*Sets the size and start address of the TX FIFO. No FIFO/Queue, just buffers. */
	FDCAN1->TXBC = (uint32_t)CAN1_TX_BUFFER_ADDR - CAN_MSG_RAM_BASE_ADDR;
	//FDCAN1->TXBC |= FDCAN_TXBC_TFQM;													//TODO: This might not matter because I'm only using buffers i think.
	FDCAN1->TXBC |= CAN1_TX_BUFFER_ELEMENTS << FDCAN_TXBC_NDTB_Pos;
}

static uint32_t can_get_int_status(FDCAN_GlobalTypeDef* canbus)
{
	return canbus->IR;
}

static void can_clear_int_status(FDCAN_GlobalTypeDef* canbus, uint32_t mask)
{
	canbus->IR = mask;
}

static uint32_t can_get_fifo0_fill_level(FDCAN_GlobalTypeDef* canbus)
{
	return (canbus->RXF0S & FDCAN_RXF0S_F0FL) >> FDCAN_RXF0S_F0FL_Pos;
}

static uint32_t can_get_fifo1_fill_level(FDCAN_GlobalTypeDef* canbus)
{
	return (canbus->RXF1S & FDCAN_RXF1S_F1FL) >> FDCAN_RXF1S_F1FL_Pos;
}

static can_rx_buffer_entry_t* can_get_rx_fifo0_entry(FDCAN_GlobalTypeDef* canbus, uint32_t index)
{
	can_rx_buffer_entry_t* dest_addr = NULL;
	if (canbus == FDCAN1)
	{
		dest_addr = (can_rx_buffer_entry_t*)((uint8_t*)CAN1_RX_BUFFER_ADDR + (index * sizeof(can_rx_buffer_entry_t)));
	}

	if (canbus == FDCAN2)
	{
		dest_addr = (can_rx_buffer_entry_t*)((uint8_t*)CAN2_RX_BUFFER_ADDR + (index * sizeof(can_rx_buffer_entry_t)));
	}

	return dest_addr;
}

static can_ext_id_filter_t* can_get_ext_id_filter(FDCAN_GlobalTypeDef* canbus, uint8_t index)
{
	can_ext_id_filter_t* dest_addr = NULL;
	if (canbus == FDCAN1)
	{
		if (index > CAN1_EXT_ID_FILTER_ELEMENTS)
		{
			return NULL;
		}
		dest_addr = (can_ext_id_filter_t*)((uint8_t*)CAN1_EXT_ID_FILTER_ADDR + (index * sizeof(can_ext_id_filter_t)));
	}

	if (canbus == FDCAN2)
	{
		if (index > CAN2_EXT_ID_FILTER_ELEMENTS)
		{
			return NULL;
		}
		dest_addr = (can_ext_id_filter_t*)((uint8_t*)CAN2_EXT_ID_FILTER_ADDR + (index * sizeof(can_ext_id_filter_t)));
	}

	return dest_addr;
}

static can_error_code_t can_get_last_error_code(FDCAN_GlobalTypeDef* canbus)
{
	static can_error_code_t last_error_code = 0;
	if (last_error_code == CAN_ERROR_CODE_NO_CHANGE)
	{
		return last_error_code;
	}
	last_error_code = canbus->PSR & FDCAN_PSR_LEC_Msk;

	/*If it's still "no change" that means this is the first time reading the register
	 * and there is no error.*/
	if (last_error_code == CAN_ERROR_CODE_NO_CHANGE)
	{
		last_error_code = CAN_ERROR_CODE_NO_ERROR;
		return last_error_code;
	}

	return last_error_code;
}

/***********	GLOBAL FUNCTION DEFINTIONS	************/
void can_init()
{
	/*IO Multiplexing.*/
	io_init();
	io_set_pin_mux(GPIOH, GPIO_PIN13_Msk, GPIO_AFR_AF9);
	io_set_pin_mux(GPIOH, GPIO_PIN14_Msk, GPIO_AFR_AF9);
	
	/*Put CAN into configuration mode.*/
	can_stop(FDCAN1);
	can_stop(FDCAN2);

	/*Sets the message RAM to all 0s.*/
	can_init_msg_ram();
	
	/*Initialize the filter registers.*/
	can_filter_init(FDCAN1);
	can_filter_init(FDCAN2);
	
	/*Reset all interrupt flags.*/
	can_clear_int_status(FDCAN1, 0xFFFFFFFF);
	can_clear_int_status(FDCAN2, 0xFFFFFFFF);
}

void can_init_clk()
{
	/*Select and enable the clocks.*/
	canbus_kernel_clk_select(PLL1_Q);
	RCC->APB1HENR |= RCC_APB1HENR_FDCANEN;

	can_stop(FDCAN1);
	can_stop(FDCAN2);

	/*Configure clock, bypass the CCU, divide by 10.*/
	FDCAN_CCU->CCFG = CAN_CCU_CDIV_Val | FDCANCCU_CCFG_BCC;
}

void can_filter_init(FDCAN_GlobalTypeDef* canbus)
{
	/*Configure global filter to reject all non-matching frames.*/
	FDCAN1->GFC = 0;
	FDCAN1->GFC |= 0x1 << FDCAN_GFC_ANFS_Pos;								//Non-matching standard frames store in FIFO 1.
	FDCAN1->GFC |= 0x1 << FDCAN_GFC_ANFE_Pos;								//Non-matching extended frames store in FIFO 1.
	FDCAN1->GFC |= FDCAN_GFC_RRFS;											//Standard remote frames (reject).
	FDCAN1->GFC |= FDCAN_GFC_RRFE;											//Extended remote frames (reject).

	FDCAN1->SIDFC = (uint32_t)CAN1_STD_ID_FILTER_ADDR - CAN_MSG_RAM_BASE_ADDR;
	FDCAN1->SIDFC |= CAN1_STD_ID_FILTER_ELEMENTS << FDCAN_SIDFC_LSS_Pos;
	FDCAN1->XIDFC = (uint32_t)CAN1_EXT_ID_FILTER_ADDR - CAN_MSG_RAM_BASE_ADDR;
	FDCAN1->XIDFC |= CAN1_EXT_ID_FILTER_ELEMENTS << FDCAN_XIDFC_LSE_Pos;
	
	/*Set all the filter elements to store a matching message in FIFO0.*/
	for (uint32_t i = 0; i < CAN1_EXT_ID_FILTER_ELEMENTS; i++)
	{
		can_get_ext_id_filter(FDCAN1, i)->F0.bit.EFEC = FDCAN_XIDFE_EFEC_STF0M_Val;
	}

	for (uint32_t i = 0; i < CAN2_EXT_ID_FILTER_ELEMENTS; i++)
	{
		can_get_ext_id_filter(FDCAN2, i)->F0.bit.EFEC = FDCAN_XIDFE_EFEC_STF0M_Val;
	}

}

void can_stop(FDCAN_GlobalTypeDef* canbus)
{
	/*Unlocks CAN1 for editing.*/
	canbus->CCCR |= FDCAN_CCCR_INIT;					//Write to the init bit.
	while ((canbus->CCCR & FDCAN_CCCR_INIT) == 0) {}	//Wait for init to be set.
	canbus->CCCR |= FDCAN_CCCR_CCE;						//Write to the CCE bit.
}

void can_run(FDCAN_GlobalTypeDef* canbus)
{
	/*Reset the CCE and INIT bits to start CAN operation.*/
	canbus->CCCR &= ~FDCAN_CCCR_CCE;					//Clear the CCE bit.
	canbus->CCCR &= ~FDCAN_CCCR_INIT;					//Clear the init bit.
	while ((canbus->CCCR & FDCAN_CCCR_INIT) != 0) {}	//Wait for init to be clear.
}

void can_set_baud_rate(FDCAN_GlobalTypeDef* canbus, can_baud_rate_t baud_rate)
{
	if (baud_rate == CAN_BAUD_ERROR)
	{
		return;
	}

	/*Clear the bits first.*/
	canbus->NBTP = 0;

	if(baud_rate == CAN_BAUD_1M)
	{
		canbus->NBTP |= (CAN_BTP_BRP_1M - 1) << FDCAN_NBTP_NBRP_Pos;
		canbus->NBTP |= (CAN_BTP_SJW_1M - 1) << FDCAN_NBTP_NSJW_Pos;
		canbus->NBTP |= (CAN_BTP_TSEG1_1M - 1) << FDCAN_NBTP_NTSEG1_Pos;
		canbus->NBTP |= (CAN_BTP_TSEG2_1M - 1) << FDCAN_NBTP_NTSEG2_Pos;
	}

	if(baud_rate == CAN_BAUD_500K)
	{
		canbus->NBTP |= (CAN_BTP_BRP_500K - 1) << FDCAN_NBTP_NBRP_Pos;
		canbus->NBTP |= (CAN_BTP_SJW_500K - 1) << FDCAN_NBTP_NSJW_Pos;
		canbus->NBTP |= (CAN_BTP_TSEG1_500K - 1) << FDCAN_NBTP_NTSEG1_Pos;
		canbus->NBTP |= (CAN_BTP_TSEG2_500K - 1) << FDCAN_NBTP_NTSEG2_Pos;
	}

	if(baud_rate == CAN_BAUD_250K)
	{
		canbus->NBTP |= (CAN_BTP_BRP_250K - 1) << FDCAN_NBTP_NBRP_Pos;
		canbus->NBTP |= (CAN_BTP_SJW_250K - 1) << FDCAN_NBTP_NSJW_Pos;
		canbus->NBTP |= (CAN_BTP_TSEG1_250K - 1) << FDCAN_NBTP_NTSEG1_Pos;
		canbus->NBTP |= (CAN_BTP_TSEG2_250K - 1) << FDCAN_NBTP_NTSEG2_Pos;
	}

	if(baud_rate == CAN_BAUD_125K)
	{
		canbus->NBTP |= (CAN_BTP_BRP_125K - 1) << FDCAN_NBTP_NBRP_Pos;
		canbus->NBTP |= (CAN_BTP_SJW_125K - 1) << FDCAN_NBTP_NSJW_Pos;
		canbus->NBTP |= (CAN_BTP_TSEG1_125K - 1) << FDCAN_NBTP_NTSEG1_Pos;
		canbus->NBTP |= (CAN_BTP_TSEG2_125K - 1) << FDCAN_NBTP_NTSEG2_Pos;
	}
}

can_baud_rate_t can_get_baud_rate(FDCAN_GlobalTypeDef* canbus)
{
	/*Arrays as containers for the for loop.*/
	const uint32_t can_btp_tseg2[] = { CAN_BTP_TSEG2_1M,
									CAN_BTP_TSEG2_500K,
									CAN_BTP_TSEG2_250K,
									CAN_BTP_TSEG2_125K };
	const uint32_t can_btp_tseg1[] = { CAN_BTP_TSEG1_1M,
									CAN_BTP_TSEG1_500K,
									CAN_BTP_TSEG1_250K,
									CAN_BTP_TSEG1_125K };
	const uint32_t can_btp_brp[] = { CAN_BTP_BRP_1M,
									CAN_BTP_BRP_500K,
									CAN_BTP_BRP_250K,
									CAN_BTP_BRP_125K };
	const uint32_t can_btp_sjw[] = { CAN_BTP_SJW_1M,
									CAN_BTP_SJW_500K,
									CAN_BTP_SJW_250K,
									CAN_BTP_SJW_125K };
	const can_baud_rate_t can_baud_rate[] = { CAN_BAUD_1M,
											CAN_BAUD_500K,
											CAN_BAUD_250K,
											CAN_BAUD_125K };


	for (uint32_t i = 0; i < 4; i++)
	{
		can_stop(canbus);							//disable CAN
		can_set_listen_only(canbus);				//put it in listen only mode.
		can_clear_int_status(canbus, 0xFFFFFFFF);	//clear all the interrupts.

		canbus->NBTP = 0;
		canbus->NBTP |= (can_btp_brp[i] - 1) << FDCAN_NBTP_NBRP_Pos;
		canbus->NBTP |= (can_btp_sjw[i] - 1) << FDCAN_NBTP_NSJW_Pos;
		canbus->NBTP |= (can_btp_tseg1[i] - 1) << FDCAN_NBTP_NTSEG1_Pos;
		canbus->NBTP |= (can_btp_tseg2[i] - 1) << FDCAN_NBTP_NTSEG2_Pos;

		can_run(canbus);							//enable CAN.
		timer_delay_ms(500);						//give it a moment to run.

		uint32_t ir = can_get_int_status(canbus);
		uint32_t rx_fifo0_fill_lvl = can_get_fifo0_fill_level(canbus);
		uint32_t rx_fifo1_fill_lvl = can_get_fifo1_fill_level(canbus);

		/*Check for any bus errors.*/
		if (can_get_last_error_code(canbus) == CAN_ERROR_CODE_NO_ERROR)
		{
			/*Check if any data was received.*/
			if (rx_fifo0_fill_lvl != 0 || rx_fifo1_fill_lvl != 0)
			{
				can_stop(canbus);							//disable CAN.
				can_clear_listen_only(canbus);
				return can_baud_rate[i];
			}

		}

	}

	can_stop(canbus);							//disable CAN.
	can_clear_listen_only(canbus);
	return CAN_BAUD_ERROR;

}

void can_deinit()
{
	RCC->APB1HRSTR |= RCC_APB1HRSTR_FDCANRST_Msk;		//Reset the peripheral.
	RCC->APB1HRSTR &= ~(RCC_APB1HRSTR_FDCANRST_Msk);	//Clear the reset bit.

	NVIC_DisableIRQ(FDCAN1_IT0_IRQn);				//Disable CAN1, line 0 IRQ.
	NVIC_DisableIRQ(FDCAN1_IT1_IRQn);				//Disable CAN1, line 1 IRQ.
	can1_state = false;

	NVIC_DisableIRQ(FDCAN2_IT0_IRQn);				//Disable CAN2, line 0 IRQ.
	NVIC_DisableIRQ(FDCAN2_IT1_IRQn);				//Disable CAN2, line 1 IRQ.
	can2_state = false;


}

bool can_get_state(FDCAN_GlobalTypeDef* canbus)
{
	if (canbus == FDCAN1)
	{
		return can1_state;
	}
	if (canbus == FDCAN2)
	{
		return can2_state;
	}
}


void can_tick_inc(uint8_t time_ms)
{
	can_timer_ms += time_ms;
}

void can_processes()
{
	for (uint8_t i = 0; i < CAN1_TX_BUFFER_ELEMENTS; i++)
	{
		/*Check if the index is even being used by a message.*/
		if (can1_tx_timer[i].index_used == false)
		{
			return;
		}
		
		/*Check if the message is supposed to be sent.*/
		if (can1_tx_timer[i].active == false)
		{
			continue;
		}
		
		/*Check if it's due to be sent.*/
		if (can_timer_ms - can1_tx_timer[i].last_time > can1_tx_timer[i].interval)
		{
			can_tx(FDCAN1, i);
			can1_tx_timer[i].last_time = can_timer_ms;
		}
	}
}

void can_assign_rx_rf0n_cb(FDCAN_GlobalTypeDef* canbus, void (*func)())
{
	if (canbus == FDCAN1)
	{
		can1_rx_rf0n_ir_cb = func;
	}

	if (canbus == FDCAN2)
	{
		can2_rx_rf0n_ir_cb = func;
	}

}

void can_enable_rx_rf0n_interrupt(FDCAN_GlobalTypeDef* canbus)
{
	if (canbus == FDCAN1)
	{
		canbus->IE |= 1 << FDCAN_IE_RF0NE_Pos;		//New message received in FIFO 0 enable interrupt.
		canbus->ILE |= 1 << FDCAN_ILE_EINT0_Pos;	//Enable interrupt line.
		NVIC_EnableIRQ(FDCAN1_IT0_IRQn);			//Enable CAN1 IRQ.
	}
	if (canbus == FDCAN2)
	{
		canbus->IE |= 1 << FDCAN_IE_RF0NE_Pos;		//New message received in FIFO 0 enable interrupt.
		canbus->ILE |= 1 << FDCAN_ILE_EINT0_Pos;	//Enable interrupt line.
		NVIC_EnableIRQ(FDCAN2_IT0_IRQn);			//Enable CAN1 IRQ.
	}
}

void can_assign_rx_rf0f_cb(FDCAN_GlobalTypeDef* canbus, void (*func)())
{
	if (canbus == FDCAN1)
	{
		can1_rx_rf0f_ir_cb = func;
	}
	if (canbus == FDCAN2)
	{
		can2_rx_rf0f_ir_cb = func;
	}
}

int8_t can_add_tx_buffer(FDCAN_GlobalTypeDef* canbus, can_tx_buffer_entry_t* new_message, uint32_t interval, bool overwrite)
{
	/*Find a slot that isn't being used in the buffer.*/
	int8_t unused_index = -1;
	for (int8_t i = 0; i < CAN1_TX_BUFFER_ELEMENTS; i++)
	{
		if (can1_tx_timer[i].index_used == false)
		{
			unused_index = i; 
			break;
		}
	}
	
	/*Couldn't find an open slot.*/
	if (unused_index == -1)
	{
		/*If option to overwrite is true, overwrite index 0.*/
		if (overwrite)
		{
			unused_index = 0;
		}
		/*If option to overwrite is false, do nothing and return -1.*/
		else
		{
			return -1;
		}
	}
	
	can_tx_buffer_entry_t* dest_addr = can_get_buffer_entry(canbus, unused_index);
	*dest_addr = *new_message;
	can1_tx_timer[unused_index].interval = interval;
	can1_tx_timer[unused_index].index_used = true;
	can1_tx_timer[unused_index].active = false;
	
	return unused_index;
}

int8_t can_activate_tx(FDCAN_GlobalTypeDef* canbus, can_tx_buffer_entry_t* message)
{
	for (uint8_t i = 0; i < CAN1_TX_BUFFER_ELEMENTS; i++)
	{
		can_tx_buffer_entry_t* dest_addr = can_get_buffer_entry(canbus, i);
		if (memcmp(dest_addr, message, sizeof(can_tx_buffer_entry_t)) == 0)
		{
			can1_tx_timer[i].active = true;
			return 1;
		}
	}
	return -1;
}

int8_t can_deactivate_tx(FDCAN_GlobalTypeDef* canbus, can_tx_buffer_entry_t* message)
{
	for (uint8_t i = 0; i < CAN1_TX_BUFFER_ELEMENTS; i++)
	{
		can_tx_buffer_entry_t* dest_addr = can_get_buffer_entry(canbus, i);
		if (memcmp(dest_addr, message, sizeof(can_tx_buffer_entry_t)) == 0)
		{
			can1_tx_timer[i].active = false;
			return 1;
		}
	}
	return -1;
}

void can_deactivate_all_tx(FDCAN_GlobalTypeDef* canbus)
{
	for(uint8_t i = 0; i < CAN1_TX_BUFFER_ELEMENTS; i++)
	{
		can1_tx_timer[i].active = false;
	}
}

int8_t can_remove_tx_buffer(FDCAN_GlobalTypeDef* canbus, can_tx_buffer_entry_t* message)
{
	for (uint8_t i = 0; i < CAN1_TX_BUFFER_ELEMENTS; i++)
	{
		can_tx_buffer_entry_t* dest_addr = can_get_buffer_entry(canbus, i);
		if (memcmp(dest_addr, message, sizeof(can_tx_buffer_entry_t)) == 0)
		{
			can1_tx_timer[i].index_used = false;
			return 1;
		}
	}
	return -1;	
}

int8_t can_add_ext_id_filter(FDCAN_GlobalTypeDef* canbus, uint32_t id, bool overwrite)
{
	/*Find a slot that isn't being used in the buffer.*/
	int8_t unused_index = -1;

	if (canbus == FDCAN1)
	{
		for (int8_t i = 0; i < CAN1_EXT_ID_FILTER_ADDR; i++)
		{
			if (can1_ext_id_filter_list[i] == false)
			{
				unused_index = i;
				break;
			}
		}
	}

	if (canbus == FDCAN2)
	{
		for (int8_t i = 0; i < CAN2_EXT_ID_FILTER_ADDR; i++)
		{
			if (can2_ext_id_filter_list[i] == false)
			{
				unused_index = i;
				break;
			}
		}
	}


	/*Couldn't find an open slot, return if overwrite is false.*/
	if (unused_index == -1)
	{
		if (overwrite)
		{
			unused_index = 0;
		}
		else
		{
			return -1;
		}
	}

	uint8_t remainder = unused_index % 2;
	if (canbus == FDCAN1)
	{
		can_ext_id_filter_t* filter = can_get_ext_id_filter(FDCAN1, unused_index / 2);
		if (remainder == 0)
		{
			filter->F0.bit.EFID1 = id;

		}

		if (remainder == 1)
		{
			filter->F1.bit.EFID2 = id;
		}
		filter->F0.reg &= ~(0x7 << FDCAN_XIDFE_EFEC_Pos);
		filter->F0.reg |= FDCAN_XIDFE_EFEC_STF0M_Msk;
		filter->F1.reg &= ~(0x3 << FDCAN_XIDFE_EFT_Pos);
		filter->F1.reg |= FDCAN_XIDFE_EFT_DUAL_ID_Msk;
		can1_ext_id_filter_list[unused_index] = true;
	}

	if (canbus == FDCAN2)
	{

	}

	return unused_index;
}


int8_t can_remove_ext_id_filter(FDCAN_GlobalTypeDef* canbus, uint32_t id)
{
	if (canbus == FDCAN1)
	{
		for (uint8_t i = 0; i < CAN1_EXT_ID_FILTER_ADDR; i++)
		{
			uint32_t filter_id1 = can_get_ext_id_filter(canbus, i)->F0.bit.EFID1;
			uint32_t filter_id2 = can_get_ext_id_filter(canbus, i)->F1.bit.EFID2;
			if (filter_id1 == id)
			{
				can1_ext_id_filter_list[i * 2] = false;
				can_get_ext_id_filter(canbus, i)->F0.bit.EFID1 = 0;
				return 1;
			}
			if (filter_id2 == id)
			{
				can1_ext_id_filter_list[(i * 2) + 1] = false;
				can_get_ext_id_filter(canbus, i)->F1.bit.EFID2 = 0;
				return 1;
			}
		}
		return -1;
	}
}

void can_remove_all_ext_id_filters(FDCAN_GlobalTypeDef* canbus)
{

}

uint8_t can_read_from_fifo0(FDCAN_GlobalTypeDef* canbus, can_rx_buffer_entry_t* message)
{
	uint8_t get_index = (canbus->RXF0S & FDCAN_RXF0S_F0GI) >> FDCAN_RXF0S_F0GI_Pos;		//TODO: Function to clean this up.
	uint32_t dest_addr = (uint32_t)((uint8_t*)CAN1_RX_FIFO0_ADDR + (get_index * sizeof(can_rx_buffer_entry_t)));
	memcpy(message, (void*)dest_addr, sizeof(can_rx_buffer_entry_t));
	canbus->RXF0A = get_index;
	uint32_t fill_level = (canbus->RXF0S & FDCAN_RXF0S_F0FL) >> FDCAN_RXF0S_F0FL_Pos;
	return fill_level;
}


/***********	INTERRUPT HANDLERS		************/
void FDCAN1_IT0_IRQHandler()
{
	uint32_t ir = FDCAN1->IR;		//Save the state of the interrupt register.
	FDCAN1->IR = 0xFFFFFFFF;		//Clear all the interrupts.
	
	if (ir & FDCAN_IR_TC)			//Transfer complete.
	{
		
	}
	
	if (ir & FDCAN_IR_RF0N)			//New RX in FIFO0.
	{
		if (can1_rx_rf0n_ir_cb)
		{
			can1_rx_rf0n_ir_cb();
		}
	}

	if (ir & FDCAN_IR_RF0F)			//FIFO0 full.
	{
		if (can1_rx_rf0f_ir_cb)
		{
			can1_rx_rf0f_ir_cb();
		}
	}
}
