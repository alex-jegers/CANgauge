
/***********	INCLUDES	************/
#include "stm32_canbus.h"
#include "stm32_io.h"
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"


/***********	STATIC VARIABLES DEFINITIONS	************/

/***********	STATIC VARIABLES DECLARATIONS	************/
static can_rx_buffer_entry_t (*can1_rx_buffer)[CAN1_RX_BUFFER_ELEMENTS] = (can_rx_buffer_entry_t(*)[CAN1_RX_BUFFER_ELEMENTS])CAN1_RX_BUFFER_ADDR;
static can_rx_buffer_entry_t (*can1_rx_fifo0)[CAN1_RX_FIFO0_ELEMENTS] = (can_rx_buffer_entry_t(*)[CAN1_RX_FIFO0_ELEMENTS])CAN1_RX_FIFO0_ADDR;
static can_rx_buffer_entry_t (*can1_rx_fifo1)[CAN1_RX_FIFO1_ELEMENTS] = (can_rx_buffer_entry_t(*)[CAN1_RX_FIFO1_ELEMENTS])CAN1_RX_FIFO1_ADDR;

static can_rx_buffer_entry_t (*can2_rx_buffer)[CAN2_RX_BUFFER_ELEMENTS] = (can_rx_buffer_entry_t(*)[CAN2_RX_BUFFER_ELEMENTS])CAN2_RX_BUFFER_ADDR;
static can_rx_buffer_entry_t (*can2_rx_fifo0)[CAN2_RX_FIFO0_ELEMENTS] = (can_rx_buffer_entry_t(*)[CAN2_RX_FIFO0_ELEMENTS])CAN2_RX_FIFO0_ADDR;
static can_rx_buffer_entry_t (*can2_rx_fifo1)[CAN2_RX_FIFO1_ELEMENTS] = (can_rx_buffer_entry_t(*)[CAN2_RX_FIFO1_ELEMENTS])CAN2_RX_FIFO1_ADDR;
static can_tx_buffer_entry_t (*can2_tx_buffer)[CAN2_TX_BUFFER_ELEMENTS] = (can_tx_buffer_entry_t(*)[CAN2_TX_BUFFER_ELEMENTS])CAN2_TX_BUFFER_ADDR;
static can_std_id_filter_t (*can2_std_id_filter)[CAN2_STD_ID_FILTER_ELEMENTS] = (can_std_id_filter_t(*)[CAN2_STD_ID_FILTER_ELEMENTS])CAN2_STD_ID_FILTER_ADDR;
static can_ext_id_filter_t (*can2_ext_id_filter)[CAN2_EXT_ID_FILTER_ELEMENTS] = (can_ext_id_filter_t(*)[CAN2_EXT_ID_FILTER_ELEMENTS])CAN2_EXT_ID_FILTER_ADDR;

static void (*can1_rx_rf0n_ir_cb)();
static void (*can2_rx_rf0n_ir_cb)();
static void (*can1_rx_rf1n_ir_cb)();
static void (*can2_rx_rf1n_ir_cb)();
static void (*can1_rx_rf0f_ir_cb)();
static void (*can2_rx_rf0f_ir_cb)();

/***********	STATIC FUNCTION DECLARATIONS	************/
/* Enables the bus clock and sets the kernel clock. */
static void can_init_clk();		
/* Fills in the addresses for the filter buffers. Helper for can_init. */
static void can_filter_init(FDCAN_GlobalTypeDef* canbus);
/* Selects a kernel clock for FDCAN. */
static void canbus_kernel_clk_select(canbus_kernel_clk_t clk);
/* Sets the start addresses for the RX and TX FIFOs/buffers. */
static void can_init_msg_ram(FDCAN_GlobalTypeDef* canbus);
/* Sets listen only mode bit. */
static void can_set_listen_only(FDCAN_GlobalTypeDef* canbus);	
/* Clears listen only mode bit. */							
static void can_clear_listen_only(FDCAN_GlobalTypeDef* canbus);															
/* Returns the interrupt status register. TODO: Can be macro. */
static uint32_t can_get_int_status(FDCAN_GlobalTypeDef* canbus);								
/* Clears the bits specified by mask in the interrupt register. */
static void can_clear_int_status(FDCAN_GlobalTypeDef* canbus, uint32_t mask);					


/*RX related functions.*/
static uint32_t can_get_fifo0_fill_level(FDCAN_GlobalTypeDef* canbus);							//returns the fill level of FIFO0.
static uint32_t can_get_fifo1_fill_level(FDCAN_GlobalTypeDef* canbus);							//returns the fill level of FIFO1.

/*Filter related functions.*/
static can_std_id_filter_t* can_get_std_id_filter_addr(FDCAN_GlobalTypeDef* canbus, uint8_t index);
static can_ext_id_filter_t* can_get_ext_id_filter_addr(FDCAN_GlobalTypeDef* canbus, uint8_t index);	//returns a filter element at specified index.


/***********	STATIC FUNCTION DEFINTIONS	************/
static void canbus_kernel_clk_select(canbus_kernel_clk_t clk)
{
	RCC->D2CCIP1R &= ~(0x3 << RCC_D2CCIP1R_FDCANSEL_Pos);		//clear the bits.
	RCC->D2CCIP1R |= clk;
}

static void can_set_listen_only(FDCAN_GlobalTypeDef* canbus)
{
	canbus->CCCR |= FDCAN_CCCR_MON;
}

static void can_clear_listen_only(FDCAN_GlobalTypeDef* canbus)
{
	canbus->CCCR &= ~(FDCAN_CCCR_MON);
}

static void can_init_clk()
{
	/*Select and enable the clocks.*/
	canbus_kernel_clk_select(PLL1_Q);
	RCC->APB1HENR |= RCC_APB1HENR_FDCANEN;

	can_stop(FDCAN1);
	can_stop(FDCAN2);

	/*Configure clock, bypass the CCU, divide by 10. This is at 48MHz*/
	FDCAN_CCU->CCFG = CAN_CCU_CDIV_Val | FDCANCCU_CCFG_BCC;
}

static void can_init_msg_ram(FDCAN_GlobalTypeDef* canbus)
{
	/*RX fifo0 start address, mode and size.*/
	canbus->RXF0C = (uint32_t)can1_rx_fifo0 - CAN_MSG_RAM_BASE_ADDR;				//F0SA, FIFO 0 Start Address.
	canbus->RXF0C |= CAN1_RX_FIFO0_ELEMENTS << FDCAN_RXF0C_F0S_Pos;

	/*RX FIFO1 start address, mode, and size.*/
	canbus->RXF1C = (uint32_t)can1_rx_fifo1 - CAN_MSG_RAM_BASE_ADDR;
	canbus->RXF1C |= CAN1_RX_FIFO1_ELEMENTS << FDCAN_RXF1C_F1S_Pos;

	/*RX buffer start address.*/
	canbus->RXBC = (uint32_t)can1_rx_buffer - CAN_MSG_RAM_BASE_ADDR;

	/*Sets the size of data field for the RX/TX buffer, operating in CAN2.0B not FD, so this is always 8 bytes.*/
	canbus->RXESC = FDCAN_RXESC_F0DS_DATA8 << FDCAN_RXESC_F0DS_Pos;
	canbus->TXESC = FDCAN_TXESC_TBDS_DATA8 << FDCAN_TXESC_TBDS_Pos;

	/*Sets the size and start address of the TX FIFO. No FIFO/Queue, just buffers. */
	canbus->TXBC = (uint32_t)CAN1_TX_BUFFER_ADDR - CAN_MSG_RAM_BASE_ADDR;
	//FDCAN1->TXBC |= FDCAN_TXBC_TFQM;													//TODO: This might not matter because I'm only using buffers i think.
	canbus->TXBC |= CAN1_TX_BUFFER_ELEMENTS << FDCAN_TXBC_NDTB_Pos;
}

static void can_filter_init(FDCAN_GlobalTypeDef* canbus)
{
	/*Configure global filter to reject all non-matching frames.*/
	FDCAN1->GFC = 0;
	FDCAN1->GFC |= 0x0 << FDCAN_GFC_ANFS_Pos;								//Non-matching standard frames store in FIFO 0.
	FDCAN1->GFC |= 0x0 << FDCAN_GFC_ANFE_Pos;								//Non-matching extended frames store in FIFO 0.
	FDCAN1->GFC |= FDCAN_GFC_RRFS;											//Standard remote frames (reject).
	FDCAN1->GFC |= FDCAN_GFC_RRFE;											//Extended remote frames (reject).

	/* Write the filter lists addresses. */
	FDCAN1->SIDFC = (uint32_t)CAN1_STD_ID_FILTER_ADDR - CAN_MSG_RAM_BASE_ADDR;
	FDCAN1->SIDFC |= CAN1_STD_ID_FILTER_ELEMENTS << FDCAN_SIDFC_LSS_Pos;
	FDCAN1->XIDFC = (uint32_t)CAN1_EXT_ID_FILTER_ADDR - CAN_MSG_RAM_BASE_ADDR;
	FDCAN1->XIDFC |= CAN1_EXT_ID_FILTER_ELEMENTS << FDCAN_XIDFC_LSE_Pos;
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

static can_std_id_filter_t* can_get_std_id_filter_addr(FDCAN_GlobalTypeDef* canbus, uint8_t index)
{
	can_std_id_filter_t* dest_addr = NULL;
	if (canbus == FDCAN1)
	{
		if (index > CAN1_STD_ID_FILTER_ELEMENTS)
		{
			return NULL;
		}
		dest_addr = (can_std_id_filter_t*)((uint8_t*)CAN1_STD_ID_FILTER_ADDR + (index * sizeof(can_std_id_filter_t)));
	}

	if (canbus == FDCAN2)
	{
		if (index > CAN2_STD_ID_FILTER_ELEMENTS)
		{
			return NULL;
		}
		dest_addr = (can_std_id_filter_t*)((uint8_t*)CAN2_STD_ID_FILTER_ADDR + (index * sizeof(can_std_id_filter_t)));
	}

	return dest_addr;
}

static can_ext_id_filter_t* can_get_ext_id_filter_addr(FDCAN_GlobalTypeDef* canbus, uint8_t index)
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

/***********	GLOBAL FUNCTION DEFINTIONS	************/
void can_init(FDCAN_GlobalTypeDef* canbus)
{

	io_init();

	can_init_clk();

	/* Zero out the message RAM. 0x4000AC00 to 0x4000D3FF. */
	memset((uint8_t*)0x4000AC00, 0, 0x27FF);

	/*CAN1 multiplexing.*/
	io_set_pin_mux(GPIOH, GPIO_PIN13_Msk, GPIO_AFR_AF9);
	io_set_pin_mux(GPIOH, GPIO_PIN14_Msk, GPIO_AFR_AF9);

	/*Initialize the filter registers.*/
	can_filter_init(canbus);

	can_init_msg_ram(canbus);

	/*Reset all interrupt flags.*/
	can_clear_int_status(canbus, 0xFFFFFFFF);
}



void can_deinit()
{

	NVIC_DisableIRQ(FDCAN1_IT0_IRQn);				//Disable CAN1, line 0 IRQ.
	NVIC_DisableIRQ(FDCAN1_IT1_IRQn);				//Disable CAN1, line 1 IRQ.

	NVIC_DisableIRQ(FDCAN2_IT0_IRQn);				//Disable CAN2, line 0 IRQ.
	NVIC_DisableIRQ(FDCAN2_IT1_IRQn);				//Disable CAN2, line 1 IRQ.


	/*Turn off the clocks.*/
	RCC->APB1HENR &= ~(RCC_APB1HENR_FDCANEN);

	/* Reset the block. */
	RCC->APB1HRSTR |= RCC_APB1HRSTR_FDCANRST;
	RCC->APB1HRSTR &= ~(RCC_APB1HRSTR_FDCANRST);
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
		can_stop(canbus);
		can_set_listen_only(canbus);				//put it in listen only mode.
		can_clear_int_status(canbus, 0xFFFFFFFF);	//clear all the interrupts.

		canbus->NBTP = 0;
		canbus->NBTP |= (can_btp_brp[i] - 1) << FDCAN_NBTP_NBRP_Pos;
		canbus->NBTP |= (can_btp_sjw[i] - 1) << FDCAN_NBTP_NSJW_Pos;
		canbus->NBTP |= (can_btp_tseg1[i] - 1) << FDCAN_NBTP_NTSEG1_Pos;
		canbus->NBTP |= (can_btp_tseg2[i] - 1) << FDCAN_NBTP_NTSEG2_Pos;

		can_run(canbus);							//enable CAN.
		vTaskDelay(500);							//give it a moment to run.

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

void can_assign_rx_rf1n_cb(FDCAN_GlobalTypeDef* canbus, void (*func)())
{
	if (canbus == FDCAN1)
	{
		can1_rx_rf1n_ir_cb = func;
	}

	if (canbus == FDCAN2)
	{
		can2_rx_rf1n_ir_cb = func;
	}

}

void can_enable_rx_rf0n_interrupt(FDCAN_GlobalTypeDef* canbus)
{
	if (canbus == FDCAN1)
	{
		canbus->IE |= 1 << FDCAN_IE_RF0NE_Pos;		//New message received in FIFO 0 enable interrupt.
		canbus->ILE |= 1 << FDCAN_ILE_EINT0_Pos;	//Enable interrupt line.
		NVIC_EnableIRQ(FDCAN1_IT0_IRQn);			//Enable CAN1 IRQ.
		NVIC_SetPriority(FDCAN1_IT0_IRQn, 0xf);
	}
	if (canbus == FDCAN2)
	{
		canbus->IE |= 1 << FDCAN_IE_RF0NE_Pos;		//New message received in FIFO 0 enable interrupt.
		canbus->ILE |= 1 << FDCAN_ILE_EINT0_Pos;	//Enable interrupt line.
		NVIC_EnableIRQ(FDCAN2_IT0_IRQn);			//Enable CAN2 IRQ.
		NVIC_SetPriority(FDCAN2_IT0_IRQn, 1);
	}
}

void can_enable_rx_rf1n_interrupt(FDCAN_GlobalTypeDef* canbus)
{
	if (canbus == FDCAN1)
	{
		canbus->IE |= 1 << FDCAN_IE_RF1NE_Pos;		//New message received in FIFO 0 enable interrupt.
		canbus->ILE |= 1 << FDCAN_ILE_EINT0_Pos;	//Enable interrupt line.
		NVIC_EnableIRQ(FDCAN1_IT0_IRQn);			//Enable CAN1 IRQ.
		NVIC_SetPriority(FDCAN1_IT0_IRQn, 0xf);
	}
	if (canbus == FDCAN2)
	{
		canbus->IE |= 1 << FDCAN_IE_RF1NE_Pos;		//New message received in FIFO 0 enable interrupt.
		canbus->ILE |= 1 << FDCAN_ILE_EINT0_Pos;	//Enable interrupt line.
		NVIC_EnableIRQ(FDCAN2_IT0_IRQn);			//Enable CAN2 IRQ.
		NVIC_SetPriority(FDCAN2_IT0_IRQn, 1);
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

int8_t can_add_tx_buffer(FDCAN_GlobalTypeDef* canbus, can_tx_buffer_entry_t* new_message, uint8_t index)
{
	can_tx_buffer_entry_t* dest_addr = can_get_tx_buffer(canbus, index);
	*dest_addr = *new_message;
	return 0;
}

can_tx_buffer_entry_t* can_get_tx_buffer(FDCAN_GlobalTypeDef* canbus, uint8_t index)
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


void can_tx(FDCAN_GlobalTypeDef* canbus, uint8_t index)
{
	if (index > 31)
	{
		return;
	}
	canbus->TXBAR = 1 << index;		//Request next transfer.
}

void can_set_std_id_filter(FDCAN_GlobalTypeDef* canbus, uint8_t index, can_std_id_filter_t* filter)
{
	*can_get_std_id_filter_addr(canbus, index) = *filter;
}

void can_set_ext_id_filter(FDCAN_GlobalTypeDef* canbus, uint8_t index, can_ext_id_filter_t* filter)
{
	*can_get_ext_id_filter_addr(canbus, index) = *filter;
}

bool can_check_for_rx_fifo0(FDCAN_GlobalTypeDef* canbus)
{
	return can_get_fifo0_fill_level(canbus);
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

bool can_check_for_rx_fifo1(FDCAN_GlobalTypeDef* canbus)
{
	return can_get_fifo1_fill_level(canbus);
}

uint8_t can_read_from_fifo1(FDCAN_GlobalTypeDef* canbus, can_rx_buffer_entry_t* message)
{
	uint8_t get_index = (canbus->RXF1S & FDCAN_RXF1S_F1GI) >> FDCAN_RXF1S_F1GI_Pos;		//TODO: Function to clean this up.
	uint32_t dest_addr = (uint32_t)((uint8_t*)CAN1_RX_FIFO1_ADDR + (get_index * sizeof(can_rx_buffer_entry_t)));
	memcpy(message, (void*)dest_addr, sizeof(can_rx_buffer_entry_t));
	canbus->RXF1A = get_index;
	uint32_t fill_level = (canbus->RXF1S & FDCAN_RXF1S_F1FL) >> FDCAN_RXF1S_F1FL_Pos;
	return fill_level;
}

uint32_t can_get_can_id(can_rx_buffer_entry_t* message)
{
	if (message->R0.bit.XTD == CAN_ID_STD)
	{
		return message->R0.bit.ID >> 18;
	}
	else
	{
		return message->R0.bit.ID;
	}
}

can_error_code_t can_get_last_error_code(FDCAN_GlobalTypeDef* canbus)
{
	static can_error_code_t last_error_code = CAN_ERROR_CODE_NO_ERROR;	
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

uint32_t can_get_rx_error_counter(FDCAN_GlobalTypeDef* canbus)
{
	return (canbus->ECR & 0x00007F00) >> 8;
}

uint32_t can_get_tx_error_counter(FDCAN_GlobalTypeDef* canbus)
{
	return (canbus->ECR & 0x000000FF);
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

	if (ir & FDCAN_IR_RF1N)			//New RX in FIFO1.
	{
		if (can1_rx_rf1n_ir_cb)
		{
			can1_rx_rf1n_ir_cb();
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
