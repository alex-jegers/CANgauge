/*
 * stm32_canbus_def.h
 *
 *  Created on: Feb 8, 2026
 *      Author: awjpp
 */

#ifndef _STM32_CANBUS_DEF_H_
#define _STM32_CANBUS_DEF_H_

/***********	INCLUDES	************/
#include "stm32h745xx.h"
#include "stdbool.h"

/***********	TYPEDEFS	************/
typedef enum can_id_t
{
	CAN_ID_STD,
	CAN_ID_XTD,
}can_id_t;

typedef enum can_rtr_t
{
	CAN_RTR_DATA_FRAME,
	CAN_RTR_REMOTE_FRAME,
}can_rtr_t;

typedef enum can_fd_t
{
	CLASSIC_CAN,
	FD_CAN,
}can_fd_t;

typedef enum can_baud_rate_t
{
	CAN_BAUD_125K,
	CAN_BAUD_250K,
	CAN_BAUD_500K,
	CAN_BAUD_1M,
	CAN_BAUD_ERROR
}can_baud_rate_t;

typedef enum can_sft_t
{
	CAN_SFT_RANGE,			//Range filter from SFID1 to SFID2.
	CAN_SFT_DUAL_ID,		//Dual ID filter: SFID1 or SFID2
	CAN_SFT_CLASSIC,		//Classic mode: SFID1 = filter, SFID2 = mask.
	CAN_SFT_DISABLE,		//Disable the filter element.
}can_sft_t;

typedef enum can_eft_t
{
	CAN_EFT_RANGE,			//Range filter from SFID1 to SFID2.
	CAN_EFT_DUAL_ID,		//Dual ID filter: SFID1 or SFID2
	CAN_EFT_CLASSIC,		//Classic mode: SFID1 = filter, SFID2 = mask.
	CAN_EFT_RANGE_NO_MSK,	//Disable the filter element.
}can_eft_t;

typedef enum can_sfec_t
{
	CAN_SFEC_DISABLE,		//Disable the filter element.
	CAN_SFEC_STORE_FIFO0,
	CAN_SFEC_STORE_FIFO1,
	CAN_SFEC_REJECT,
	CAN_SFEC_SET_PRI,		//Set priority.
	CAN_SFEC_SET_PRI_FIFO0,	//Set priority and store in FIFO0.
	CAN_SFEC_SET_PRI_FIFO1, //Set priority and store in FIFO1.
	CAN_SFEC_DBG,			//Store into RX buffer or as a debug message.
}can_sfec_t;

typedef enum can_efec_t
{
	CAN_EFEC_DISABLE,		//Disable the filter element.
	CAN_EFEC_STORE_FIFO0,
	CAN_EFEC_STORE_FIFO1,
	CAN_EFEC_REJECT,
	CAN_EFEC_SET_PRI,		//Set priority.
	CAN_EFEC_SET_PRI_FIFO0,	//Set priority and store in FIFO0.
	CAN_EFEC_SET_PRI_FIFO1, //Set priority and store in FIFO1.
	CAN_EFEC_DBG,			//Store into RX buffer or as a debug message.
}can_efec_t;

/***********	DEFINES		************/

/***********	VARIABLE DEFINITIONS	************/
/*TX buffer/fifo/queue message RAM structure.*/
typedef struct  {
	union {
		struct {
			uint32_t ID : 29;		/* Identifier, has to be left shifted 18 bits for std id */
			can_rtr_t RTR : 1;		/* Remote Transmission Request, data frame or remote frame. */
			can_id_t XTD : 1;		/* Extended Identifier = 1, standard ID = 0. */
			uint32_t ESI : 1;		/* Error State Indicator, only used in FD. */
		} bit;
		uint32_t val; /* Type used for register access */
	} T0;
	union {
		struct {
			uint32_t : 16;		/* Reserved */
			uint32_t DLC : 4;	/* Data Length Code */
			uint32_t BRS : 1;	/* Bit Rate Switch = 1, No BRS = 0, only used in FD. */
			can_fd_t FDF : 1;	/* FD Format = 1, Non-FD = 0 */
			uint32_t : 1;		/* Reserved */
			uint32_t EFC : 1;	/* Event FIFO Control, Store TX events = 1, dont store = 0 */
			uint32_t MM : 8;	/* Message Marker, written by CPU during buffer config. */
		} bit;
		uint32_t val; /* Type used for register access */
	} T1;

	uint8_t data[8];			/* Data to transmit. */
}can_tx_buffer_entry_t;

/*RX buffer/fifo/queue message RAM structure.*/
/*Size is 16 bytes*/
typedef struct  {
	union {
		struct {
			uint32_t ID : 29; /*!< Identifier, has to be right shifted 18 bits for std id.*/
			can_rtr_t RTR : 1; /*!< Remote Transmission Request */
			can_id_t XTD : 1; /*!< Extended Identifier */
			uint32_t ESI : 1; /*!< Error State Indicator */
		} bit;
		uint32_t val; /*!< Type used for register access */
	} R0;
	union {
		struct {
			uint32_t RXTS : 16; /*!< Rx Timestamp */
			uint32_t DLC : 4;   /*!< Data Length Code */
			uint32_t BRS : 1;   /*!< Bit Rate Switch */
			can_fd_t FDF : 1;   /*!< FD Format */
			uint32_t : 2;       /*!< Reserved */
			uint32_t FIDX : 7;  /*!< Filter Index */
			uint32_t ANMF : 1;  /*!< Accepted Non-matching Frame */
		} bit;
		uint32_t val; /*!< Type used for register access */
	} R1;
	uint8_t data[8];
}can_rx_buffer_entry_t;

/*Standard ID filter element message RAM structure.*/
typedef struct
{
	union {
		struct {
			uint32_t SFID2 : 11; /*!< Standard Filter ID 2 */
			uint32_t : 5;        /*!< Reserved */
			uint32_t SFID1 : 11; /*!< Standard Filter ID 1 */
			can_sfec_t SFEC : 3;   /*!< Standard Filter Configuration */
			can_sft_t SFT : 2;    /*!< Standard Filter Type */
		} bit;
		uint32_t val; /*!< Type used for register access */
	} S0;
}can_std_id_filter_t;

/*Extended ID filter element message RAM structure.*/
typedef struct
 {
	union
	{
		struct
		{
			uint32_t EFID1		:29;
			can_efec_t EFEC		:3;
		} bit;
		uint32_t reg; /*!< Type used for register access */
	} F0;

	union
	{
		struct
		{
			uint32_t EFID2		:29;
			uint32_t			:1;
			can_eft_t EFT		:2;
		} bit;
	uint32_t reg; /*!< Type used for register access */
	} F1;

}can_ext_id_filter_t;

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

typedef enum
{
	HSE_CLK		= 0x0 << RCC_D2CCIP1R_FDCANSEL_Pos,
	PLL1_Q		= 0x1 << RCC_D2CCIP1R_FDCANSEL_Pos,
	PLL2_Q		= 0x2 << RCC_D2CCIP1R_FDCANSEL_Pos,
}canbus_kernel_clk_t;


/**********		DEFINES 		**********/
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

#define CAN1_RX_BUFFER_ELEMENTS			1
#define CAN1_RX_FIFO0_ELEMENTS			64
#define CAN1_RX_FIFO1_ELEMENTS			64
#define CAN1_TX_EVENT_FIFO_ELEMENTS		0				/*TODO: Need tx_event_fifo_t.*/
#define CAN1_TX_BUFFER_ELEMENTS			32
#define CAN1_STD_ID_FILTER_ELEMENTS		16
#define CAN1_EXT_ID_FILTER_ELEMENTS		16
#define CAN1_TRIGGER_MEMORY_ELEMENTS	0				/*TODO: Need can_trigger_memory_t.*/

#define CAN2_RX_BUFFER_ELEMENTS			64
#define CAN2_RX_FIFO0_ELEMENTS			64
#define CAN2_RX_FIFO1_ELEMENTS			64
#define CAN2_TX_EVENT_FIFO_ELEMENTS		0				/*TODO: Need tx_event_fifo_t.*/
#define CAN2_TX_BUFFER_ELEMENTS			32
#define CAN2_STD_ID_FILTER_ELEMENTS		4
#define CAN2_EXT_ID_FILTER_ELEMENTS		4
#define CAN2_TRIGGER_MEMORY_ELEMENTS	0				/*TODO: Need can_trigger_memory_t.*/

#define CAN_MSG_RAM_BASE_ADDR			0x4000AC00
#define CAN_MSG_RAM_END_ADDR			0x4000D3FF

#define CAN1_STD_ID_FILTER_ADDR			(CAN_MSG_RAM_BASE_ADDR)
#define CAN1_EXT_ID_FILTER_ADDR			(CAN_MSG_RAM_BASE_ADDR + (CAN1_STD_ID_FILTER_ELEMENTS * sizeof(can_std_id_filter_t)))
#define CAN1_RX_FIFO0_ADDR				(CAN1_EXT_ID_FILTER_ADDR + (CAN1_EXT_ID_FILTER_ELEMENTS * sizeof(can_ext_id_filter_t)))
#define CAN1_RX_FIFO1_ADDR				(CAN1_RX_FIFO0_ADDR + (CAN1_RX_FIFO0_ELEMENTS * sizeof(can_rx_buffer_entry_t)))
#define CAN1_RX_BUFFER_ADDR				(CAN1_RX_FIFO1_ADDR + (CAN1_RX_FIFO1_ELEMENTS * sizeof(can_rx_buffer_entry_t)))
#define CAN1_TX_EVENT_FIFO_ADDR			(CAN1_RX_BUFFER_ADDR + (CAN1_RX_BUFFER_ELEMENTS * sizeof(can_rx_buffer_entry_t)))
#define CAN1_TX_BUFFER_ADDR				(CAN1_TX_EVENT_FIFO_ADDR + (CAN1_TX_EVENT_FIFO_ELEMENTS * 0))		/*TODO: Need can_tx_event_fifo_t.*/
#define CAN1_TRIGGER_MEMORY_ADDR		(CAN1_TX_BUFFER_ADDR + (CAN1_TX_BUFFER_ELEMENTS * sizeof(can_tx_buffer_entry_t)))			/*TODO: Need can_trigger_memory_t.*/

#define CAN2_STD_ID_FILTER_ADDR			(CAN_MSG_RAM_BASE_ADDR)
#define CAN2_EXT_ID_FILTER_ADDR			(CAN_MSG_RAM_BASE_ADDR + (CAN2_STD_ID_FILTER_ELEMENTS * sizeof(can_std_id_filter_t)))
#define CAN2_RX_FIFO0_ADDR				(CAN2_EXT_ID_FILTER_ADDR + (CAN2_EXT_ID_FILTER_ELEMENTS * sizeof(can_ext_id_filter_t)))
#define CAN2_RX_FIFO1_ADDR				(CAN2_RX_FIFO0_ADDR + (CAN2_RX_FIFO0_ELEMENTS * sizeof(can_rx_buffer_entry_t)))
#define CAN2_RX_BUFFER_ADDR				(CAN2_RX_FIFO1_ADDR + (CAN2_RX_FIFO1_ELEMENTS * sizeof(can_rx_buffer_entry_t)))
#define CAN2_TX_EVENT_FIFO_ADDR			(CAN2_RX_BUFFER_ADDR + (CAN2_RX_BUFFER_ELEMENTS * sizeof(can_rx_buffer_entry_t)))
#define CAN2_TX_BUFFER_ADDR				(CAN2_TX_EVENT_FIFO_ADDR + (CAN2_TX_EVENT_FIFO_ELEMENTS * 0))		/*TODO: Need can_tx_event_fifo_t.*/
#define CAN2_TRIGGER_MEMORY_ADDR		(CAN2_TX_BUFFER_ADDR + (CAN2_TX_BUFFER_ELEMENTS * sizeof(can_tx_buffer_entry_t)))			/*TODO: Need can_trigger_memory_t.*/

#define FDCAN_RXESC_F0DS_DATA8			0
#define FDCAN_RXESC_F1DS_DATA8			0
#define FDCAN_TXESC_TBDS_DATA8			0

#define FDCAN_TSCC_TSS_INC				0x1

#define FDCAN_SIDFE_SFT_CLASSIC_Val		0x2			//Standard ID filter, classic mode.
#define FDCAN_SIDFE_SFEC_STF0M_Val		0x1			//Standard ID filter, store in FIF0.
#define FDCAN_XIDFE_EFEC_STF0M_Val		0x1			//Extended ID filter, store in FIFO0.
#define FDCAN_SIDFE_SFEC_STF1M_Val		0x2			//Standard ID filter, store in FIFO1.
#define FDCAN_XIDFE_EFEC_STF1M_Val		0x2			//Extended ID filter, store in FIFO1.
#define FDCAN_XIDFE_EFEC_Pos			29
#define FDCAN_XIDFE_EFEC_STF0M_Msk		FDCAN_XIDFE_EFEC_STF0M_Val << FDCAN_XIDFE_EFEC_Pos
#define FDCAN_XIDFE_EFT_RANGE_Val		0x3			//Ext ID filter, filter range from EFID1 to EFID2.
#define FDCAN_XIDFE_EFT_DUAL_ID_Val		0x1			//Filter is EF1ID or EF2ID.
#define FDCAN_XIDFE_EFT_Pos				30
#define FDCAN_XIDFE_EFT_DUAL_ID_Msk		FDCAN_XIDFE_EFT_DUAL_ID_Val << FDCAN_XIDFE_EFT_Pos


/***********	GLOBAL FUNCTION DECLARATIONS	************/


#endif /* _STM32_CANBUS_DEF_H_ */
