/**
  ******************************************************************************
  * @file    CSpi.cpp
  * @author  Chenxx
  * @version V1.1
  * @date    2015-10-17
  * @brief   This file provides all the Spi functions.
  ******************************************************************************/
/*******************************************************************************
* release information
*******************************************************************************/	
#include "stm32f4xx.h"
#include "CSpi.h"
#include <string.h>

#define assert(val)	seer_assert(val)

#ifndef assert
#define assert(val) while(1)
#endif

/*End of File*/
//
//constructor
//
CSpi::CSpi(SPI_TypeDef * SPIx_):
		SPIx_(SPIx_), 
		DmaTxStream_(TxDMA(SPIx_)), 
		DmaRxStream_(RxDMA(SPIx_)),
		TXDMA_IFCR_(0),
		RXDMA_IFCR_(0),
		RXDMA_ISR_(0),
		TXDMA_IFCR_TCIF_MASK(0),
		RXDMA_IFCR_TCIF_MASK(0)
{
	uint32_t*& TXDMA_IFCR_ref = const_cast<uint32_t*&>(TXDMA_IFCR_);
	uint32_t*& RXDMA_IFCR_ref = const_cast<uint32_t*&>(RXDMA_IFCR_);
	uint32_t*& RXDMA_ISR_ref = const_cast<uint32_t*&>(RXDMA_ISR_);
	uint32_t& TXDMA_IFCR_TCIF_MASK_ref = const_cast<uint32_t&>(TXDMA_IFCR_TCIF_MASK);
	uint32_t& RXDMA_IFCR_TCIF_MASK_ref = const_cast<uint32_t&>(RXDMA_IFCR_TCIF_MASK);
	
	uint32_t streamx = (((uint32_t)DmaTxStream_&0xFF) - 0x10)/0x18;	// 0-7
	DMA_TypeDef * DMAx = ((DMA_TypeDef *)((uint32_t)DmaTxStream_&0xFFFFFC00));
	
	if(streamx < 4) TXDMA_IFCR_ref = (uint32_t*)&(DMAx->LIFCR);
	else TXDMA_IFCR_ref = (uint32_t*)&(DMAx->HIFCR);
	switch(streamx % 4)
	{
		case 0:
			TXDMA_IFCR_TCIF_MASK_ref = 1<<5;
			break;
		case 1:
			TXDMA_IFCR_TCIF_MASK_ref = 1<<11;
			break;
		case 2:
			TXDMA_IFCR_TCIF_MASK_ref = 1<<21;
			break;
		case 3:
			TXDMA_IFCR_TCIF_MASK_ref = 1<<27;
			break;
	}
	
	streamx = (((uint32_t)DmaRxStream_&0xFF) - 0x10)/0x18;
	
	if(streamx < 4) RXDMA_IFCR_ref = (uint32_t*)&(DMAx->LIFCR);
	else RXDMA_IFCR_ref = (uint32_t*)&(DMAx->HIFCR);
	switch(streamx % 4)
	{
		case 0:
			RXDMA_IFCR_TCIF_MASK_ref = 1<<5;
			break;
		case 1:
			RXDMA_IFCR_TCIF_MASK_ref = 1<<11;
			break;
		case 2:
			RXDMA_IFCR_TCIF_MASK_ref = 1<<21;
			break;
		case 3:
			RXDMA_IFCR_TCIF_MASK_ref = 1<<27;
			break;
	}
	
	RXDMA_ISR_ref = RXDMA_IFCR_ - 2;
}

//
//void CSpi::InitSpiGpio()
//
void CSpi::InitSpiGpio()
{
	GPIO_InitTypeDef GPIO_InitStructure; 
	uint32_t RCC_AHB1Periph_GPIOx;
	uint8_t GPIO_PinSource_Clk;
	uint8_t GPIO_PinSource_Rx;
	uint8_t GPIO_PinSource_Tx;
	uint8_t GPIO_AF_SPIx;
	GPIO_TypeDef *GPIOx_Clk;
	GPIO_TypeDef *GPIOx_Tx;
	GPIO_TypeDef *GPIOx_Rx;
	
	IOGroup_Type IOGroup = BspIOGroup(SPIx_);
	
	if(IOGroup == GROUP_A5A6A7)
	{
		RCC_AHB1Periph_GPIOx = RCC_AHB1Periph_GPIOA;
		GPIOx_Clk = GPIOx_Tx = GPIOx_Rx = GPIOA;
		GPIO_PinSource_Clk = GPIO_PinSource5;
		GPIO_PinSource_Rx = GPIO_PinSource6;
		GPIO_PinSource_Tx = GPIO_PinSource7;
	}	
	else if(IOGroup == GROUP_B3B4B5)
	{
		RCC_AHB1Periph_GPIOx = RCC_AHB1Periph_GPIOB;
		GPIOx_Clk = GPIOx_Tx = GPIOx_Rx = GPIOB;
		GPIO_PinSource_Clk = GPIO_PinSource3;
		GPIO_PinSource_Rx = GPIO_PinSource4;
		GPIO_PinSource_Tx = GPIO_PinSource5;
	}
	else if(IOGroup == 2)
	{
#ifndef RUN_FOR_SRC_2000
		RCC_AHB1Periph_GPIOx = RCC_AHB1Periph_GPIOB;
		GPIOx_Clk = GPIOx_Tx = GPIOx_Rx = GPIOB;
		GPIO_PinSource_Clk = GPIO_PinSource13;
		GPIO_PinSource_Rx = GPIO_PinSource14;
		GPIO_PinSource_Tx = GPIO_PinSource15;
#else
		RCC_AHB1Periph_GPIOx = RCC_AHB1Periph_GPIOB;
		GPIOx_Clk = GPIOx_Tx = GPIOx_Rx = GPIOB;
		GPIO_PinSource_Clk = GPIO_PinSource10;
		GPIO_PinSource_Rx = GPIO_PinSource14;
		GPIO_PinSource_Tx = GPIO_PinSource15;
#endif
	}
	else assert(false); //undefined!
	
	if(SPIx_ == SPI1)	GPIO_AF_SPIx = GPIO_AF_SPI1;
	else if(SPIx_ == SPI2)	GPIO_AF_SPIx = GPIO_AF_SPI2;
	else if(SPIx_ == SPI3)	GPIO_AF_SPIx = GPIO_AF_SPI3;
	else assert(false); //undefined!

	/* open clock of GPIO */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOx, ENABLE);
	
	/* Config Pin: TXD RXD*/
	GPIO_PinAFConfig(GPIOx_Clk, GPIO_PinSource_Clk, GPIO_AF_SPIx);
	GPIO_PinAFConfig(GPIOx_Rx, GPIO_PinSource_Rx, GPIO_AF_SPIx);
	GPIO_PinAFConfig(GPIOx_Tx, GPIO_PinSource_Tx, GPIO_AF_SPIx);
	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0<< GPIO_PinSource_Clk;
	GPIO_Init(GPIOx_Clk, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0<< GPIO_PinSource_Rx;
	GPIO_Init(GPIOx_Rx, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0<< GPIO_PinSource_Tx;
	GPIO_Init(GPIOx_Tx, &GPIO_InitStructure);
	
}

/**
  * @brief  Initialize the SPI
  * @retval None
  */
void CSpi::InitSpi()
{
	/* init clock of SPI */
	if(SPIx_ == SPI1) 
	{
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
		RCC_AHB2PeriphResetCmd(RCC_APB2Periph_SPI1, ENABLE);
		RCC_AHB2PeriphResetCmd(RCC_APB2Periph_SPI1, DISABLE);
	}
	else if(SPIx_ == SPI2)
	{
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
		RCC_AHB1PeriphResetCmd(RCC_APB1Periph_SPI2, ENABLE);
		RCC_AHB1PeriphResetCmd(RCC_APB1Periph_SPI2, DISABLE);
	}
	else if(SPIx_ == SPI3) 
	{
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE);
		RCC_AHB1PeriphResetCmd(RCC_APB1Periph_SPI3, ENABLE);
		RCC_AHB1PeriphResetCmd(RCC_APB1Periph_SPI3, DISABLE);
	}
	else assert(false);
	
	/* Deinitializes the SPIx */
	SPI_DeInit(SPIx_);
	
/* Config SPI */
	SPI_InitTypeDef SPI_InitStructure;

	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;	//Full Duplex
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;										//
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;													//keep SCK high when free
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;			                 	//
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;                          	//NSS by software
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;	//baud rate
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;                 	//Endian
	SPI_InitStructure.SPI_CRCPolynomial = 7;                            //CRC polygon
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;												//master/slave mode
	SPI_Init(SPIx_, &SPI_InitStructure);
	
	SPI_Cmd(SPIx_, DISABLE); 
	SPI_Init(SPIx_, &SPI_InitStructure); 
	SPI_Cmd(SPIx_, ENABLE); 
	if(SPIx_ == SPI1)
	{
	SPI_DMACmd(SPIx_, SPI_DMAReq_Tx, DISABLE);
	SPI_DMACmd(SPIx_, SPI_DMAReq_Rx, DISABLE);
	
	/* DMA Clock Config */
	uint32_t RCC_AHB1Periph;
	DMA_TypeDef * DMAx = ((DMA_TypeDef *)((uint32_t)DmaTxStream_&0xFFFFFC00));
	
	if(DMAx == DMA1) RCC_AHB1Periph = RCC_AHB1Periph_DMA1;
	else if(DMAx == DMA2) RCC_AHB1Periph = RCC_AHB1Periph_DMA2;
	else assert(false); //error
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph, ENABLE);
	
	DMA_DeInit(DmaTxStream_);
	DMA_DeInit(DmaRxStream_);
	
	DMA_InitTypeDef DMA_InitStructure;
	
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull; 
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single; 
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	if(SPIx_ == SPI1)
		DMA_InitStructure.DMA_Channel = DMA_Channel_3; //attention!
	else
		DMA_InitStructure.DMA_Channel = DMA_Channel_0; //attention!
	
	/* TX DMA Config */	
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(SPIx_->DR);
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)"0";
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
	DMA_InitStructure.DMA_BufferSize = 0;
	
	DMA_Cmd(DmaTxStream_, DISABLE);
	*TXDMA_IFCR_ = TXDMA_IFCR_TCIF_MASK;
	DMA_Init(DmaTxStream_, &DMA_InitStructure);
	
	/* RX DMA Config */
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(SPIx_->DR);
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)0;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
	DMA_InitStructure.DMA_BufferSize = 0;
	
	DMA_Cmd(DmaRxStream_, DISABLE);
	*RXDMA_IFCR_ = RXDMA_IFCR_TCIF_MASK;
	DMA_Init(DmaRxStream_, &DMA_InitStructure);
	
	SPI_DMACmd(SPIx_, SPI_DMAReq_Tx, ENABLE);
	SPI_DMACmd(SPIx_, SPI_DMAReq_Rx, ENABLE);
	}
	SPI_I2S_GetFlagStatus(SPIx_, SPI_I2S_FLAG_TXE);
	SPI_I2S_GetFlagStatus(SPIx_, SPI_I2S_FLAG_RXNE);
}


void CSpi::InitIRQ()
{
	NVIC_InitTypeDef NVIC_InitStructure;
	
	SPI_Cmd(SPIx_,DISABLE);
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

	if(SPIx_==SPI1) NVIC_InitStructure.NVIC_IRQChannel = SPI1_IRQn;
	else if(SPIx_==SPI2) NVIC_InitStructure.NVIC_IRQChannel = SPI2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	SPI_I2S_ITConfig(SPIx_,SPI_I2S_IT_RXNE,ENABLE);
	SPI_Cmd(SPIx_,ENABLE);
}

/**
  * @brief  TxDMA()
	* @param  None
  * @retval Tx Dma Channel of the Usart
  */
DMA_Stream_TypeDef * CSpi::TxDMA(SPI_TypeDef * targetSpi)
{
	if(SPI1 == targetSpi)
		return DMA2_Stream3;
	else
		assert(false);
	
	//program should not come here
	return DMA2_Stream3;
}

/**
  * @brief  RxDMA()
	* @param  None
  * @retval Rx Dma Channel of the Usart
  */
DMA_Stream_TypeDef * CSpi::RxDMA(SPI_TypeDef * targetSpi)
{
	if(SPI1 == targetSpi)
		return DMA2_Stream0;
	else
		assert(false);
	
	//program should not come here
	return DMA2_Stream0;
}

/**
  * @brief  BspIOGroup
	* @param  None
  * @retval Tx Dma Channel of the Usart
  */
CSpi::IOGroup_Type CSpi::BspIOGroup(SPI_TypeDef * targetSpi)
{
	if(SPI1 == targetSpi)
		return GROUP_A5A6A7;
#ifdef RUN_FOR_SRC_2000
	else if(SPI2 == targetSpi)
		return GROUP_B10B14B15;
#endif
	else
		assert(false);
	
	//program should not come here
	return GROUP_A5A6A7;
}

uint8_t CSpi::SyncPutc(uint8_t byte)
{
	SPI_I2S_ReceiveData(SPIx_);
	while (SPI_I2S_GetFlagStatus(SPIx_, SPI_I2S_FLAG_TXE) == RESET);
	SPI_I2S_SendData(SPIx_, byte);

  while (SPI_I2S_GetFlagStatus(SPIx_, SPI_I2S_FLAG_RXNE) == RESET);
	return SPI_I2S_ReceiveData(SPIx_);
}

uint8_t CSpi::SyncGetc()
{
	return SyncPutc(0xFF);
}

int CSpi::SyncWrite(uint8_t* buf, uint16_t len)
{
	/* judge whether DMA is sending */
	if(DmaTxStream_->NDTR != 0) return -1;
	
	DMA_Cmd(DmaTxStream_, DISABLE);
	
	*TXDMA_IFCR_ = TXDMA_IFCR_TCIF_MASK;
	DmaTxStream_->NDTR = len;
	DmaTxStream_->M0AR = (uint32_t)buf;
	
	DMA_Cmd(DmaTxStream_, ENABLE);
	
	//wait for transmit finished
	while(DmaTxStream_->NDTR > 0);
	DMA_Cmd(DmaTxStream_, DISABLE);
	return 0;
}

int CSpi::SyncRead(uint8_t* buf, uint16_t len)
{
	/* judge whether DMA is sending */
	if(DmaTxStream_->NDTR != 0) return -1;
	
	DMA_Cmd(DmaTxStream_, DISABLE);
	SPI_I2S_ReceiveData(SPIx_);
	
	if(DmaRxStream_->NDTR != 0)
		DMA_Cmd(DmaRxStream_, DISABLE);
	
	*TXDMA_IFCR_ = TXDMA_IFCR_TCIF_MASK;
	DmaTxStream_->NDTR = len;
	DmaTxStream_->M0AR = (uint32_t)buf;
	
	*RXDMA_IFCR_ = RXDMA_IFCR_TCIF_MASK;
	DmaRxStream_->NDTR = len;
	DmaRxStream_->M0AR = (uint32_t)buf;
	
	DMA_Cmd(DmaRxStream_, ENABLE);
	DMA_Cmd(DmaTxStream_, ENABLE);
	
	//wait for receive finished
	while(DmaRxStream_->NDTR > 0);
	DMA_Cmd(DmaTxStream_, DISABLE);
	DMA_Cmd(DmaRxStream_, DISABLE);
	return 0;
}

int CSpi::SyncWriteThenWrite(uint8_t* buf0, uint16_t len0, uint8_t* buf1, uint16_t len1)
{
	SyncWrite(buf0, len0);
	SyncWrite(buf1, len1);
	return 0;
}

int CSpi::SyncWriteThenRead(uint8_t* buf0, uint16_t len0, uint8_t* buf1, uint16_t len1)
{
	SyncWrite(buf0, len0);
	SyncRead(buf1, len1);
	return 0;
}
