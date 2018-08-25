/**
  ******************************************************************************
  * @file    CSpi.h
  * @author  Chenxx
  * @version V1.2
  * @date    2015-09-05
  * @brief   This file defines the CSpi class.
  ******************************************************************************/
/*******************************************************************************
* release information
* 2016/01/02 chenxx add DmaSendArray();
*******************************************************************************/	
#ifndef __CSPI_H
#define __CSPI_H

#include "stm32f4xx.h"
#include "F4Kxx_BSPmacro.h"

class CSpi
{
public:
	CSpi(SPI_TypeDef *);
	enum IOGroup_Type
	{
		GROUP_A5A6A7 = 0,
		GROUP_B3B4B5,
#ifndef RUN_FOR_SRC_2000
		GROUP_B13B14B15
#else
		GROUP_B10B14B15
#endif
		
	};
	
	void InitSpiGpio();
	void InitSpi();
	void InitIRQ();
	uint8_t SyncPutc(uint8_t);
	uint8_t SyncGetc();
	int SyncWriteThenWrite(uint8_t*, uint16_t, uint8_t*, uint16_t);
	int SyncWriteThenRead(uint8_t*, uint16_t, uint8_t*, uint16_t);
	
	static DMA_Stream_TypeDef * RxDMA(SPI_TypeDef *);
	static DMA_Stream_TypeDef * TxDMA(SPI_TypeDef *);
	static IOGroup_Type BspIOGroup(SPI_TypeDef *);

	int SyncWrite(uint8_t*, uint16_t);
	int SyncRead(uint8_t*, uint16_t);
	
private:
	SPI_TypeDef* SPIx_;
	DMA_Stream_TypeDef * DmaTxStream_;
	DMA_Stream_TypeDef * DmaRxStream_;

	/* interrupt flag clear register */
	uint32_t* const TXDMA_IFCR_;
	uint32_t* const RXDMA_IFCR_;
	uint32_t* const RXDMA_ISR_;
	const uint32_t TXDMA_IFCR_TCIF_MASK;
	const uint32_t RXDMA_IFCR_TCIF_MASK;
};
#endif
/*End of File*/
