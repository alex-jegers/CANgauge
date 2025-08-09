/*
 * stm32_fmc.c
 *
 *  Created on: Feb 27, 2024
 *      Author: awjpp
 */
#ifdef CORE_CM7

#include "stm32_fmc.h"
#include "stm32_io.h"
#include "stm32_rcc.h"
#include "stm32_timer.h"
#include "string.h"			//TODO: for test, remove.

#define TEST_ADDR				*(uint32_t*)0xD0000000




/**************STATIC FUNCTIONS******************/
void fmc_init_sdram()
{
	/*Initialize the IO pins needed for SDRAM.*/
	fmc_init_gpio_sdram();
	//fmc_init_io_test();
	/*Enable RCC AHB3 clock for the FMC.*/
	RCC->AHB3ENR |= RCC_AHB3ENR_FMCEN;

	/*Select and enable the kernel clock.*/
	RCC->D1CCIPR |= RCC_D1CCIPR_FMCSEL_PLL2R;
	rcc_enable_pll2r();

#ifdef TARGET_HARDWARE_STM32H745DISCO

	const uint32_t fmc_sdcr1_msk = (FMC_SDCRx_RBURST_Msk)
								|	(FMC_SDCR_SDCLK_DIV_2_Val << FMC_SDCRx_SDCLK_Pos)	//clock div 2. 232MHz in SDRAM_SDCLK is 116MHz.
								|	(FMC_SDCRx_WP_Msk);
	
	const uint32_t fmc_sdcr2_msk = (FMC_SDCR_NR_12_BITS_Val << FMC_SDCRx_NR_Pos)
							| (FMC_SDCR_MWID_16_BITS_Val << FMC_SDCRx_MWID_Pos)
							| (FMC_SDCR_NB_4_BANKS_Val << FMC_SDCRx_NB_Pos)
							| (FMC_SDCR_CAS_3_CYCLES_Val << FMC_SDCRx_CAS_Pos);

	const uint32_t fmc_sdtr_msk = (FMC_SDTR_TMRD_Val << FMC_SDTRx_TMRD_Pos)
							| (FMC_SDTR_TXSR_Val << FMC_SDTRx_TXSR_Pos)
							| (FMC_SDTR_TRAS_Val << FMC_SDTRx_TRAS_Pos)
							| (FMC_SDTR_TRC_Val << FMC_SDTRx_TRC_Pos)
							| (FMC_SDTR_TWR_Val << FMC_SDTRx_TWR_Pos)
							| (FMC_SDTR_TRP_Val << FMC_SDTRx_TRP_Pos)
							| (FMC_SDTR_TRCD_Val << FMC_SDTRx_TRCD_Pos);


	const uint32_t fmc_sdcmr_msk = (FMC_SDCMR_MRD_Val << FMC_SDCMR_MRD_Pos)
							| (FMC_SDCMR_NRFS_Val << FMC_SDCMR_NRFS_Pos)
							| (FMC_SDCMR_CTB1_Msk)
							| (FMC_SDCMR_CTB2_Msk);

	const uint32_t fmc_sdrtr_msk = (FMC_SDRTR_COUNT_Val << FMC_SDRTR_COUNT_Pos);

#endif //TARGET_HARDWARE == STM32H745DISCO





#ifdef TARGET_HARDWARE_CANGAUGE

	const uint32_t fmc_sdcr1_msk = (FMC_SDCRx_RBURST_Msk)
								| (FMC_SDCR_SDCLK_DIV_3_Val << FMC_SDCRx_SDCLK_Pos)	//clock div 3. 232MHz in SDRAM_SDCLK is 77.33MHz.
								| (FMC_SDCRx_WP_Msk)
								| (FMC_SDCR_NR_12_BITS_Val << FMC_SDCRx_NR_Pos)
								| (FMC_SDCR_NC_9_BITS_Val << FMC_SDCRx_NC_Pos)
								| (FMC_SDCR_MWID_16_BITS_Val << FMC_SDCRx_MWID_Pos)
								| (FMC_SDCR_NB_4_BANKS_Val << FMC_SDCRx_NB_Pos)
								| (FMC_SDCR_CAS_3_CYCLES_Val << FMC_SDCRx_CAS_Pos);
	
	const uint32_t fmc_sdcr2_msk = (FMC_SDCR_NR_12_BITS_Val << FMC_SDCRx_NR_Pos)
							| (FMC_SDCR_NC_9_BITS_Val << FMC_SDCRx_NC_Pos)
							| (FMC_SDCR_MWID_16_BITS_Val << FMC_SDCRx_MWID_Pos)
							| (FMC_SDCR_NB_4_BANKS_Val << FMC_SDCRx_NB_Pos)
							| (FMC_SDCR_CAS_3_CYCLES_Val << FMC_SDCRx_CAS_Pos);

	const uint32_t fmc_sdtr_msk = (FMC_SDTR_TMRD_Val << FMC_SDTRx_TMRD_Pos)
							| (FMC_SDTR_TXSR_Val << FMC_SDTRx_TXSR_Pos)
							| (FMC_SDTR_TRAS_Val << FMC_SDTRx_TRAS_Pos)
							| (FMC_SDTR_TRC_Val << FMC_SDTRx_TRC_Pos)
							| (FMC_SDTR_TWR_Val << FMC_SDTRx_TWR_Pos)
							| (FMC_SDTR_TRP_Val << FMC_SDTRx_TRP_Pos)
							| (FMC_SDTR_TRCD_Val << FMC_SDTRx_TRCD_Pos);

	const uint32_t fmc_sdcmr_msk = (FMC_SDCMR_MRD_Val << FMC_SDCMR_MRD_Pos)
							| (FMC_SDCMR_NRFS_Val << FMC_SDCMR_NRFS_Pos)
							| (FMC_SDCMR_CTB1_Msk)
							| (FMC_SDCMR_CTB2_Msk);

	const uint32_t fmc_sdrtr_msk = (FMC_SDRTR_COUNT_Val << FMC_SDRTR_COUNT_Pos);


#endif //TARGET_HARDWARE == CANGAUGE

	FMC_Bank1_R->BTCR[0] |= FMC_BCR1_FMCEN;

	FMC_Bank5_6_R->SDCR[0] = fmc_sdcr1_msk;
	FMC_Bank5_6_R->SDCR[1] = fmc_sdcr2_msk;

	FMC_Bank5_6_R->SDTR[0] = fmc_sdtr_msk;
	FMC_Bank5_6_R->SDTR[1] = fmc_sdtr_msk;

	/*All of the command register writes are tested and work.*/
	/*Clock enable. Tested*/
	FMC_Bank5_6_R->SDCMR = (FMC_SDCMR_MODE_CLK_CONFIG_Val << FMC_SDCMR_MODE_Pos)
						| (FMC_SDCMR_CTB1_Msk)
						| (FMC_SDCMR_CTB2_Msk);
	timer_delay_ms(1);

	/*Precharge command. Tested.*/
	FMC_Bank5_6_R->SDCMR = (FMC_SDCMR_MODE_PALL_Val << FMC_SDCMR_MODE_Pos)
						| (FMC_SDCMR_CTB1_Msk)
						| (FMC_SDCMR_CTB2_Msk);
	timer_delay_ms(1);

	/*Refresh command. Tested.*/
	FMC_Bank5_6_R->SDCMR = (FMC_SDCMR_MODE_AUTO_RFSH_Val << FMC_SDCMR_MODE_Pos)
						| (FMC_SDCMR_NRFS_Val << FMC_SDCMR_NRFS_Pos)
						| (FMC_SDCMR_CTB1_Msk)
						| (FMC_SDCMR_CTB2_Msk);
	timer_delay_ms(1);

	/*Program mode register. Tested.*/
	FMC_Bank5_6_R->SDCMR = (FMC_SDCMR_MODE_LOAD_MODE_REG_Val << FMC_SDCMR_MODE_Pos)
						| (FMC_SDCMR_MRD_Val << FMC_SDCMR_MRD_Pos)
						| (FMC_SDCMR_CTB1_Msk)
						| (FMC_SDCMR_CTB2_Msk);
	timer_delay_ms(1);

	/*Set the refresh rate counter. Tested.*/
	FMC_Bank5_6_R->SDRTR = 156;

	timer_delay_ms(1);

	//TODO: Read back the memory and add error handling for if it reads back incorrectly.
	for (uint32_t addr = 0xd0000000; addr < 0xD017E800; addr += 4)
	{
		*(uint32_t*)addr = 0xffeb2fa6;
	}

	for (uint32_t addr = 0xd0000000; addr < 0xD017E800; addr += 4)
	{
		uint32_t data = *(uint32_t*)addr;
		if (data != 0xffeb2fa6)
		{
			while (1)
			{
				io_test_led_on();
				timer_delay_ms(500);
				io_test_led_off();
				timer_delay_ms(500);
			}

		}
	}

}


void fmc_init_gpio_sdram()
{
	io_init();

	io_set_pin_mux(SDRAM_D0		, GPIO_AFR_AF12);
	io_set_pin_mux(SDRAM_D1		, GPIO_AFR_AF12);
	io_set_pin_mux(SDRAM_D2		, GPIO_AFR_AF12);
	io_set_pin_mux(SDRAM_D3		, GPIO_AFR_AF12);
	io_set_pin_mux(SDRAM_D4		, GPIO_AFR_AF12);
	io_set_pin_mux(SDRAM_D5		, GPIO_AFR_AF12);
	io_set_pin_mux(SDRAM_D6		, GPIO_AFR_AF12);
	io_set_pin_mux(SDRAM_D7		, GPIO_AFR_AF12);
	io_set_pin_mux(SDRAM_D8		, GPIO_AFR_AF12);
	io_set_pin_mux(SDRAM_D9		, GPIO_AFR_AF12);
	io_set_pin_mux(SDRAM_D10	, GPIO_AFR_AF12);
	io_set_pin_mux(SDRAM_D11	, GPIO_AFR_AF12);
	io_set_pin_mux(SDRAM_D12	, GPIO_AFR_AF12);
	io_set_pin_mux(SDRAM_D13	, GPIO_AFR_AF12);
	io_set_pin_mux(SDRAM_D14	, GPIO_AFR_AF12);
	io_set_pin_mux(SDRAM_D15	, GPIO_AFR_AF12);
	io_set_pin_mux(SDRAM_A0		, GPIO_AFR_AF12);
	io_set_pin_mux(SDRAM_A1		, GPIO_AFR_AF12);
	io_set_pin_mux(SDRAM_A2		, GPIO_AFR_AF12);
	io_set_pin_mux(SDRAM_A3		, GPIO_AFR_AF12);
	io_set_pin_mux(SDRAM_A4		, GPIO_AFR_AF12);
	io_set_pin_mux(SDRAM_A5		, GPIO_AFR_AF12);
	io_set_pin_mux(SDRAM_A6		, GPIO_AFR_AF12);
	io_set_pin_mux(SDRAM_A7		, GPIO_AFR_AF12);
	io_set_pin_mux(SDRAM_A8		, GPIO_AFR_AF12);
	io_set_pin_mux(SDRAM_A9		, GPIO_AFR_AF12);
	io_set_pin_mux(SDRAM_A10	, GPIO_AFR_AF12);
	io_set_pin_mux(SDRAM_A11	, GPIO_AFR_AF12);
	io_set_pin_mux(SDRAM_BA0	, GPIO_AFR_AF12);
	io_set_pin_mux(SDRAM_BA1	, GPIO_AFR_AF12);
	io_set_pin_mux(SDRAM_NBL0	, GPIO_AFR_AF12);
	io_set_pin_mux(SDRAM_NBL1	, GPIO_AFR_AF12);
	io_set_pin_mux(SDRAM_CLK	, GPIO_AFR_AF12);
	io_set_pin_mux(SDRAM_NCAS	, GPIO_AFR_AF12);
	io_set_pin_mux(SDRAM_NRAS	, GPIO_AFR_AF12);
	io_set_pin_mux(SDRAM_NWE	, GPIO_AFR_AF12);

	/*TODO: Clean up after test.*/
	io_set_pin_mux(SDRAM_NE1	, GPIO_AFR_AF12);
	io_set_pin_mux(SDRAM_CKE1	, GPIO_AFR_AF12);
	/*****************************/


	io_set_output_speed(SDRAM_D0	, GPIO_OSPEEDR_VERY_HIGH);
	io_set_output_speed(SDRAM_D1	, GPIO_OSPEEDR_VERY_HIGH);
	io_set_output_speed(SDRAM_D2	, GPIO_OSPEEDR_VERY_HIGH);
	io_set_output_speed(SDRAM_D3	, GPIO_OSPEEDR_VERY_HIGH);
	io_set_output_speed(SDRAM_D4	, GPIO_OSPEEDR_VERY_HIGH);
	io_set_output_speed(SDRAM_D5	, GPIO_OSPEEDR_VERY_HIGH);
	io_set_output_speed(SDRAM_D6	, GPIO_OSPEEDR_VERY_HIGH);
	io_set_output_speed(SDRAM_D7	, GPIO_OSPEEDR_VERY_HIGH);
	io_set_output_speed(SDRAM_D8	, GPIO_OSPEEDR_VERY_HIGH);
	io_set_output_speed(SDRAM_D9	, GPIO_OSPEEDR_VERY_HIGH);
	io_set_output_speed(SDRAM_D10	, GPIO_OSPEEDR_VERY_HIGH);
	io_set_output_speed(SDRAM_D11	, GPIO_OSPEEDR_VERY_HIGH);
	io_set_output_speed(SDRAM_D12	, GPIO_OSPEEDR_VERY_HIGH);
	io_set_output_speed(SDRAM_D13	, GPIO_OSPEEDR_VERY_HIGH);
	io_set_output_speed(SDRAM_D14	, GPIO_OSPEEDR_VERY_HIGH);
	io_set_output_speed(SDRAM_D15	, GPIO_OSPEEDR_VERY_HIGH);
	io_set_output_speed(SDRAM_A0	, GPIO_OSPEEDR_VERY_HIGH);
	io_set_output_speed(SDRAM_A1	, GPIO_OSPEEDR_VERY_HIGH);
	io_set_output_speed(SDRAM_A2	, GPIO_OSPEEDR_VERY_HIGH);
	io_set_output_speed(SDRAM_A3	, GPIO_OSPEEDR_VERY_HIGH);
	io_set_output_speed(SDRAM_A4	, GPIO_OSPEEDR_VERY_HIGH);
	io_set_output_speed(SDRAM_A5	, GPIO_OSPEEDR_VERY_HIGH);
	io_set_output_speed(SDRAM_A6	, GPIO_OSPEEDR_VERY_HIGH);
	io_set_output_speed(SDRAM_A7	, GPIO_OSPEEDR_VERY_HIGH);
	io_set_output_speed(SDRAM_A8	, GPIO_OSPEEDR_VERY_HIGH);
	io_set_output_speed(SDRAM_A9	, GPIO_OSPEEDR_VERY_HIGH);
	io_set_output_speed(SDRAM_A10	, GPIO_OSPEEDR_VERY_HIGH);
	io_set_output_speed(SDRAM_A11	, GPIO_OSPEEDR_VERY_HIGH);
	io_set_output_speed(SDRAM_BA0	, GPIO_OSPEEDR_VERY_HIGH);
	io_set_output_speed(SDRAM_BA1	, GPIO_OSPEEDR_VERY_HIGH);
	io_set_output_speed(SDRAM_NBL0	, GPIO_OSPEEDR_VERY_HIGH);
	io_set_output_speed(SDRAM_NBL1	, GPIO_OSPEEDR_VERY_HIGH);
	io_set_output_speed(SDRAM_CLK	, GPIO_OSPEEDR_VERY_HIGH);
	io_set_output_speed(SDRAM_NCAS	, GPIO_OSPEEDR_VERY_HIGH);
	io_set_output_speed(SDRAM_NRAS	, GPIO_OSPEEDR_VERY_HIGH);
	io_set_output_speed(SDRAM_NWE	, GPIO_OSPEEDR_VERY_HIGH);

	/*TODO: Cleanup after test.*/
	io_set_output_speed(SDRAM_NE1	, GPIO_OSPEEDR_VERY_HIGH);
	io_set_output_speed(SDRAM_CKE1	, GPIO_OSPEEDR_VERY_HIGH);

	/****************************/


#ifdef TARGET_HARDWARE_CANGAUGE

	io_set_pin_mux(SDRAM_A12, GPIO_AFR_AF12);	
	io_set_output_speed(SDRAM_A12, GPIO_OSPEEDR_VERY_HIGH);

#endif //TARGET_HARDWARE == CANGAUGE

}

void fmc_init_io_test()
{

	io_init();

	io_set_pin_dir_out(SDRAM_D0	);
	io_set_pin_dir_out(SDRAM_D1	);
	io_set_pin_dir_out(SDRAM_D2	);
	io_set_pin_dir_out(SDRAM_D3	);
	io_set_pin_dir_out(SDRAM_D4	);
	io_set_pin_dir_out(SDRAM_D5	);
	io_set_pin_dir_out(SDRAM_D6	);
	io_set_pin_dir_out(SDRAM_D7	);
	io_set_pin_dir_out(SDRAM_D8	);
	io_set_pin_dir_out(SDRAM_D9	);
	io_set_pin_dir_out(SDRAM_D10);
	io_set_pin_dir_out(SDRAM_D11);
	io_set_pin_dir_out(SDRAM_D12);
	io_set_pin_dir_out(SDRAM_D13);
	io_set_pin_dir_out(SDRAM_D14);
	io_set_pin_dir_out(SDRAM_D15);
	io_set_pin_dir_out(SDRAM_A0	);
	io_set_pin_dir_out(SDRAM_A1	);
	io_set_pin_dir_out(SDRAM_A2	);
	io_set_pin_dir_out(SDRAM_A3	);
	io_set_pin_dir_out(SDRAM_A4	);
	io_set_pin_dir_out(SDRAM_A5	);
	io_set_pin_dir_out(SDRAM_A6	);
	io_set_pin_dir_out(SDRAM_A7	);
	io_set_pin_dir_out(SDRAM_A8	);
	io_set_pin_dir_out(SDRAM_A9	);
	io_set_pin_dir_out(SDRAM_A10);
	io_set_pin_dir_out(SDRAM_A11);
	io_set_pin_dir_out(SDRAM_BA0);
	io_set_pin_dir_out(SDRAM_BA1);
	io_set_pin_dir_out(SDRAM_NBL0);
	io_set_pin_dir_out(SDRAM_NBL1);
	io_set_pin_dir_out(SDRAM_CLK);
	io_set_pin_dir_out(SDRAM_NCAS);
	io_set_pin_dir_out(SDRAM_NRAS);
	io_set_pin_dir_out(SDRAM_NWE);
	io_set_pin_dir_out(SDRAM_NE1);
	io_set_pin_dir_out(SDRAM_CKE1);

	io_pin_out_set(SDRAM_D0	);
	io_pin_out_set(SDRAM_D1	);
	io_pin_out_set(SDRAM_D2	);
	io_pin_out_set(SDRAM_D3	);
	io_pin_out_set(SDRAM_D4	);
	io_pin_out_set(SDRAM_D5	);
	io_pin_out_set(SDRAM_D6	);
	io_pin_out_set(SDRAM_D7	);
	io_pin_out_set(SDRAM_D8	);
	io_pin_out_set(SDRAM_D9	);
	io_pin_out_set(SDRAM_D10);
	io_pin_out_set(SDRAM_D11);
	io_pin_out_set(SDRAM_D12);
	io_pin_out_set(SDRAM_D13);
	io_pin_out_set(SDRAM_D14);
	io_pin_out_set(SDRAM_D15);
	io_pin_out_set(SDRAM_A0	);
	io_pin_out_set(SDRAM_A1	);
	io_pin_out_set(SDRAM_A2	);
	io_pin_out_set(SDRAM_A3	);
	io_pin_out_set(SDRAM_A4	);
	io_pin_out_set(SDRAM_A5	);
	io_pin_out_set(SDRAM_A6	);
	io_pin_out_set(SDRAM_A7	);
	io_pin_out_set(SDRAM_A8	);
	io_pin_out_set(SDRAM_A9	);
	io_pin_out_set(SDRAM_A10);
	io_pin_out_set(SDRAM_A11);
	io_pin_out_set(SDRAM_BA0);
	io_pin_out_set(SDRAM_BA1);
	io_pin_out_set(SDRAM_NBL0);
	io_pin_out_set(SDRAM_NBL1);
	io_pin_out_set(SDRAM_CLK);
	io_pin_out_set(SDRAM_NCAS);
	io_pin_out_set(SDRAM_NRAS);
	io_pin_out_set(SDRAM_NWE);
	io_pin_out_set(SDRAM_NE1);
	io_pin_out_set(SDRAM_CKE1);

	io_pin_out_clr(SDRAM_D0	);
	io_pin_out_clr(SDRAM_D1	);
	io_pin_out_clr(SDRAM_D2	);
	io_pin_out_clr(SDRAM_D3	);
	io_pin_out_clr(SDRAM_D4	);
	io_pin_out_clr(SDRAM_D5	);
	io_pin_out_clr(SDRAM_D6	);
	io_pin_out_clr(SDRAM_D7	);
	io_pin_out_clr(SDRAM_D8	);
	io_pin_out_clr(SDRAM_D9	);
	io_pin_out_clr(SDRAM_D10);
	io_pin_out_clr(SDRAM_D11);
	io_pin_out_clr(SDRAM_D12);
	io_pin_out_clr(SDRAM_D13);
	io_pin_out_clr(SDRAM_D14);
	io_pin_out_clr(SDRAM_D15);
	io_pin_out_clr(SDRAM_A0	);
	io_pin_out_clr(SDRAM_A1	);
	io_pin_out_clr(SDRAM_A2	);
	io_pin_out_clr(SDRAM_A3	);
	io_pin_out_clr(SDRAM_A4	);
	io_pin_out_clr(SDRAM_A5	);
	io_pin_out_clr(SDRAM_A6	);
	io_pin_out_clr(SDRAM_A7	);
	io_pin_out_clr(SDRAM_A8	);
	io_pin_out_clr(SDRAM_A9	);
	io_pin_out_clr(SDRAM_A10);
	io_pin_out_clr(SDRAM_A11);
	io_pin_out_clr(SDRAM_BA0);
	io_pin_out_clr(SDRAM_BA1);
	io_pin_out_clr(SDRAM_NBL0);
	io_pin_out_clr(SDRAM_NBL1);
	io_pin_out_clr(SDRAM_CLK);
	io_pin_out_clr(SDRAM_NCAS);
	io_pin_out_clr(SDRAM_NRAS);
	io_pin_out_clr(SDRAM_NWE);
	io_pin_out_clr(SDRAM_NE1);
	io_pin_out_clr(SDRAM_CKE1);
}

#endif //CORE_CM7
