/*
 * stm32_canbus.h
 *
 *  Created on: Mar 9, 2024
 *      Author: awjpp
 */

#ifndef SRC_DRIVERS_STM32_CANBUS_H_
#define SRC_DRIVERS_STM32_CANBUS_H_

/***********	INCLUDES	************/
#include "stm32h745xx.h"
#include "stdbool.h"
#include "stm32_sys_timer.h"

/***********	TYPEDEFS	************/
typedef enum
{
	STD_ID,
	XTD_ID,
}can_id_t;

typedef enum
{
	DATA_FRAME,
	REMOTE_FRAME,
}can_rtr_t;

typedef enum
{
	CLASSIC_CAN,
	FD_CAN,
}can_fd_t;

typedef enum
{
	CAN_BAUD_125K,
	CAN_BAUD_250K,
	CAN_BAUD_500K,
	CAN_BAUD_1M,
	CAN_BAUD_ERROR
}can_baud_rate_t;

/***********	DEFINES		************/
#define CAN1_RX_BUFFER_ELEMENTS			1
#define CAN1_RX_FIFO0_ELEMENTS			64
#define CAN1_RX_FIFO1_ELEMENTS			64
#define CAN1_TX_EVENT_FIFO_ELEMENTS		0				/*TODO: Need tx_event_fifo_t.*/
#define CAN1_TX_BUFFER_ELEMENTS			4
#define CAN1_STD_ID_FILTER_ELEMENTS		4
#define CAN1_EXT_ID_FILTER_ELEMENTS		4
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
#define FDCAN_XIDFE_EFEC_Pos			29
#define FDCAN_XIDFE_EFEC_STF0M_Msk		FDCAN_XIDFE_EFEC_STF0M_Val << FDCAN_XIDFE_EFEC_Pos
#define FDCAN_XIDFE_EFT_RANGE_Val		0x3			//Ext ID filter, filter range from EFID1 to EFID2.
#define FDCAN_XIDFE_EFT_DUAL_ID_Val		0x1			//Filter is EF1ID or EF2ID.
#define FDCAN_XIDFE_EFT_Pos				30
#define FDCAN_XIDFE_EFT_DUAL_ID_Msk		FDCAN_XIDFE_EFT_DUAL_ID_Val << FDCAN_XIDFE_EFT_Pos

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
			uint32_t SFEC : 3;   /*!< Standard Filter Configuration */
			uint32_t SFT : 2;    /*!< Standard Filter Type */
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
			uint32_t EFEC		:3;
		} bit;
		uint32_t reg; /*!< Type used for register access */
	} F0;

	union
	{
		struct
		{
			uint32_t EFID2		:29;
			uint32_t			:1;
			uint32_t EFT		:2;
		} bit;
	uint32_t reg; /*!< Type used for register access */
	} F1;

}can_ext_id_filter_t;

/*CAN Message, used for abstraction.*/
typedef struct
{
	uint32_t        id;   /* Message identifier */
	uint8_t			type; /* Message Type, 0 for data frame, 1 for RTR */
	uint8_t *       data; /* Pointer to Message Data */
	uint8_t         len;  /* Message Length */
	uint8_t			fmt;  /* Identifier format, CAN_STD, CAN_EXT */
}can_message_t;


/***********	GLOBAL FUNCTION DECLARATIONS	************/
/*Configuration and initialization functions*/
void can_init();
void can_init_clk();
void can_deinit();
void can_stop(FDCAN_GlobalTypeDef* canbus);
void can_run(FDCAN_GlobalTypeDef* canbus);
void can_set_baud_rate(FDCAN_GlobalTypeDef* canbus, can_baud_rate_t baud_rate);
can_baud_rate_t can_get_baud_rate(FDCAN_GlobalTypeDef* canbus);
void can_filter_init(FDCAN_GlobalTypeDef* canbus);
bool can_get_state();										//Returns true if FDCAN is initialized and running, false if deinitialized.
void can_tick_inc(uint8_t time_ms);
void can_processes();

/*TX control.*/
int8_t can_add_tx_buffer(FDCAN_GlobalTypeDef* canbus, can_tx_buffer_entry_t* new_message, uint32_t interval, bool overwrite);
int8_t can_activate_tx(FDCAN_GlobalTypeDef* canbus, can_tx_buffer_entry_t* message);
int8_t can_deactivate_tx(FDCAN_GlobalTypeDef* canbus, can_tx_buffer_entry_t* message);
void  can_deactivate_all_tx(FDCAN_GlobalTypeDef* canbus);
int8_t can_remove_tx_buffer(FDCAN_GlobalTypeDef* canbus, can_tx_buffer_entry_t* message);

/*RX control.*/
void can_assign_rx_rf0n_cb(FDCAN_GlobalTypeDef* canbus, void (*func)());		//New RX in FIFO0 interrupt.
void can_enable_rx_rf0n_interrupt(FDCAN_GlobalTypeDef* canbus);
void can_assign_rx_rf0f_cb(FDCAN_GlobalTypeDef* canbus, void (*func)());		//RX FIFO0 full interrupt.
uint8_t can_read_from_fifo0(FDCAN_GlobalTypeDef* canbus, can_rx_buffer_entry_t* message);

/*Extended ID filter control*/
int8_t can_add_ext_id_filter(FDCAN_GlobalTypeDef* canbus, uint32_t id, bool overwrite);		//returns 1 if list is full, 2 if ID is not a valid ID, 0 if no error.
int8_t can_remove_ext_id_filter(FDCAN_GlobalTypeDef* canbus, uint32_t id);
void can_remove_all_ext_id_filters(FDCAN_GlobalTypeDef* canbus);



#endif /* SRC_DRIVERS_STM32_CANBUS_H_ */
