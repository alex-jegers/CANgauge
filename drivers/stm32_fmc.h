/*
 * stm32_fmc.h
 *
 *  Created on: Feb 27, 2024
 *      Author: awjpp
 */

#ifndef DRIVERS_STM32_FMC_H_
#define DRIVERS_STM32_FMC_H_

#include "stm32h745xx.h"


#define FMC_SDCR_NC_8_BITS_Val						0x0
#define FMC_SDCR_NC_9_BITS_Val						0x1
#define FMC_SDCR_NR_11_BITS_Val						0x0
#define FMC_SDCR_NR_12_BITS_Val						0x1
#define FMC_SDCR_NR_13_BITS_Val						0x2
#define FMC_SDCR_MWID_32_BITS_Val					0x2
#define FMC_SDCR_MWID_16_BITS_Val					0x1
#define FMC_SDCR_NB_4_BANKS_Val						0x1
#define FMC_SDCR_CAS_3_CYCLES_Val					0x3
#define FMC_SDCR_CAS_2_CYCLES_Val					0x2
#define FMC_SDCR_CAS_1_CYCLES_Val					0x1
#define FMC_SDCR_WP_EN_Val							0x0
#define FMC_SDCR_WP_DIS_Val							0x1
#define FMC_SDCR_SDCLK_DIV_2_Val					0x2
#define FMC_SDCR_SDCLK_DIV_3_Val					0x3
#define FMC_SDCR_RPIPE_0_DELAY_Val					0x0

/*SDRAM clock is 8.6ns = 116MHz*/
#define FMC_SDTR_TMRD_Val							15		//-6 = 12ns, -7 = 14ns
#define FMC_SDTR_TXSR_Val							15		//-6 = 66ns, -7 = 70ns
#define FMC_SDTR_TRAS_Val							3
#define FMC_SDTR_TRC_Val							6
#define FMC_SDTR_TWR_Val							15		//Could maybe be 4.
#define FMC_SDTR_TRP_Val							1
#define FMC_SDTR_TRCD_Val							1

#define FMC_SDCMR_NRFS_Val							(8 - 1)	//Might need to be more.
#define FMC_SDCMR_MODE_NORMAL_Val					0x0
#define FMC_SDCMR_MODE_CLK_CONFIG_Val				0x1
#define FMC_SDCMR_MODE_PALL_Val						0x2
#define FMC_SDCMR_MODE_AUTO_RFSH_Val				0x3
#define FMC_SDCMR_MODE_LOAD_MODE_REG_Val			0x4
#define FMC_SDCMR_MODE_POWER_DOWN_Val				0x5
#define FMC_SDCMR_MRD_Val							(SDRAM_BURST_LENGTH_1_Msk 			\
													| SDRAM_BURST_TYPE_SEQUENTIAL_Msk 	\
													| SDRAM_CAS_MODE_3_Msk				\
													| SDRAM_WRITE_BURST_TYPE_SINGLE_Msk)

#define SDRAM_BURST_LENGTH_1_Msk					0x0
#define SDRAM_BURST_TYPE_SEQUENTIAL_Msk				0x0
#define SDRAM_BURST_TYPE_INTERLEAVED_Msk			0x0
#define SDRAM_CAS_MODE_2_Msk						(0x2 << 4)
#define SDRAM_CAS_MODE_3_Msk						(0x3 << 4)
#define SDRAM_WRITE_BURST_TYPE_PROGRAM_Msk			(0x0 << 9)
#define SDRAM_WRITE_BURST_TYPE_SINGLE_Msk			(0x1 << 9)

#define FMC_SDRTR_COUNT_Val							1539

#ifdef TARGET_HARDWARE_STM32H745DISCO

#define SDRAM_D2				GPIOD, GPIO_PIN0_Msk	
#define SDRAM_D3				GPIOD, GPIO_PIN1_Msk	
#define SDRAM_D13				GPIOD, GPIO_PIN8_Msk	
#define SDRAM_D14				GPIOD, GPIO_PIN9_Msk	
#define SDRAM_D15				GPIOD, GPIO_PIN10_Msk	
#define SDRAM_D0				GPIOD, GPIO_PIN14_Msk	
#define SDRAM_D1				GPIOD, GPIO_PIN15_Msk	

#define SDRAM_NBL0				GPIOE, GPIO_PIN0_Msk	
#define SDRAM_NBL1				GPIOE, GPIO_PIN1_Msk	
#define SDRAM_D4				GPIOE, GPIO_PIN7_Msk	
#define SDRAM_D5				GPIOE, GPIO_PIN8_Msk	
#define SDRAM_D6				GPIOE, GPIO_PIN9_Msk	
#define SDRAM_D7				GPIOE, GPIO_PIN10_Msk	
#define SDRAM_D8				GPIOE, GPIO_PIN11_Msk	
#define SDRAM_D9				GPIOE, GPIO_PIN12_Msk	
#define SDRAM_D10				GPIOE, GPIO_PIN13_Msk	
#define SDRAM_D11				GPIOE, GPIO_PIN14_Msk	
#define SDRAM_D12				GPIOE, GPIO_PIN15_Msk	


#define SDRAM_A0				GPIOF, GPIO_PIN0_Msk	
#define SDRAM_A1				GPIOF, GPIO_PIN1_Msk	
#define SDRAM_A2				GPIOF, GPIO_PIN2_Msk	
#define SDRAM_A3				GPIOF, GPIO_PIN3_Msk	
#define SDRAM_A4				GPIOF, GPIO_PIN4_Msk	
#define SDRAM_A5				GPIOF, GPIO_PIN5_Msk	
#define SDRAM_NRAS				GPIOF, GPIO_PIN11_Msk	
#define SDRAM_A6				GPIOF, GPIO_PIN12_Msk	
#define SDRAM_A7				GPIOF, GPIO_PIN13_Msk	
#define SDRAM_A8				GPIOF, GPIO_PIN14_Msk	
#define SDRAM_A9				GPIOF, GPIO_PIN15_Msk	

#define SDRAM_A10				GPIOG, GPIO_PIN0_Msk	
#define SDRAM_A11				GPIOG, GPIO_PIN1_Msk	
#define SDRAM_BA0				GPIOG, GPIO_PIN4_Msk	
#define SDRAM_BA1				GPIOG, GPIO_PIN5_Msk	
#define SDRAM_CLK				GPIOG, GPIO_PIN8_Msk	
#define SDRAM_NCAS				GPIOG, GPIO_PIN15_Msk	

#define SDRAM_NWE				GPIOH, GPIO_PIN5_Msk	
#define SDRAM_NE1				GPIOH, GPIO_PIN6_Msk	
#define SDRAM_CKE1				GPIOH, GPIO_PIN7_Msk	

#endif //TARGET_HARDWARE_DISCO_BOARD


#ifdef TARGET_HARDWARE_CANGAUGE

#define SDRAM_D2				GPIOD, GPIO_PIN0_Msk		
#define SDRAM_D3				GPIOD, GPIO_PIN1_Msk		
#define SDRAM_D13				GPIOD, GPIO_PIN8_Msk		
#define SDRAM_D14				GPIOD, GPIO_PIN9_Msk		
#define SDRAM_D15				GPIOD, GPIO_PIN10_Msk		
#define SDRAM_D0				GPIOD, GPIO_PIN14_Msk		
#define SDRAM_D1				GPIOD, GPIO_PIN15_Msk		

#define SDRAM_NBL0				GPIOE, GPIO_PIN0_Msk		
#define SDRAM_NBL1				GPIOE, GPIO_PIN1_Msk		
#define SDRAM_D4				GPIOE, GPIO_PIN7_Msk		
#define SDRAM_D5				GPIOE, GPIO_PIN8_Msk		
#define SDRAM_D6				GPIOE, GPIO_PIN9_Msk		
#define SDRAM_D7				GPIOE, GPIO_PIN10_Msk		
#define SDRAM_D8				GPIOE, GPIO_PIN11_Msk		
#define SDRAM_D9				GPIOE, GPIO_PIN12_Msk		
#define SDRAM_D10				GPIOE, GPIO_PIN13_Msk		
#define SDRAM_D11				GPIOE, GPIO_PIN14_Msk		
#define SDRAM_D12				GPIOE, GPIO_PIN15_Msk		


#define SDRAM_A0				GPIOF, GPIO_PIN0_Msk		
#define SDRAM_A1				GPIOF, GPIO_PIN1_Msk		
#define SDRAM_A2				GPIOF, GPIO_PIN2_Msk		
#define SDRAM_A3				GPIOF, GPIO_PIN3_Msk		
#define SDRAM_A4				GPIOF, GPIO_PIN4_Msk		
#define SDRAM_A5				GPIOF, GPIO_PIN5_Msk		
#define SDRAM_NRAS				GPIOF, GPIO_PIN11_Msk		
#define SDRAM_A6				GPIOF, GPIO_PIN12_Msk		
#define SDRAM_A7				GPIOF, GPIO_PIN13_Msk		
#define SDRAM_A8				GPIOF, GPIO_PIN14_Msk		
#define SDRAM_A9				GPIOF, GPIO_PIN15_Msk		

#define SDRAM_A10				GPIOG, GPIO_PIN0_Msk		
#define SDRAM_A11				GPIOG, GPIO_PIN1_Msk		
#define SDRAM_A12				GPIOG, GPIO_PIN2_Msk		//NC in STM32H745DISCO config.
#define SDRAM_BA0				GPIOG, GPIO_PIN4_Msk		
#define SDRAM_BA1				GPIOG, GPIO_PIN5_Msk		
#define SDRAM_CLK				GPIOG, GPIO_PIN8_Msk		
#define SDRAM_NCAS				GPIOG, GPIO_PIN15_Msk		

#define SDRAM_NWE				GPIOC, GPIO_PIN0_Msk		
#define SDRAM_NE1				GPIOB, GPIO_PIN6_Msk		//PC2_C, need to set PC2SO in SYSCFG_PMCR.
#define SDRAM_CKE1				GPIOB, GPIO_PIN5_Msk

#endif //TARGET_HARDWARE_CANGAUGE

void fmc_init_sdram();
void fmc_init_gpio_sdram();
void fmc_init_io_test();

#endif /* DRIVERS_STM32_FMC_H_ */
