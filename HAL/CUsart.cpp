#include "CUsart.h"
#include "string.h"

//
//constructor
//
CUsart::CUsart(USART_TypeDef * USARTx,
	uint8_t* rxBuff,
	uint16_t rxBuffSize,
	uint32_t BaudRate):
		USARTx_(USARTx),
		DmaTxStream_(TxDMA(USARTx_)),
		DmaRxStream_(RxDMA(USARTx_)),
		baudRate_(BaudRate),
		parity_(USART_Parity_No),
		rxOverflowCount_(0),
		rxBuff_(rxBuff),
		rxBuffSize_(rxBuffSize),
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

/**
  * @brief  send char array with length.
	* @param  buf: pointer of char buffer array.
	* @param  size: length of array.
	* @retval true:	 send succeeded.
	* @retval false: DMA is working with last send and sending failed.
  */
uint16_t CUsart::send_Array(uint8_t* buf, uint16_t size)
{
	/* judge whether DMA is sending */
	if(DmaTxStream_->NDTR != 0) return 0;

	DMA_Cmd(DmaTxStream_, DISABLE);

	*TXDMA_IFCR_ = TXDMA_IFCR_TCIF_MASK;
	DmaTxStream_->NDTR = size;
	DmaTxStream_->M0AR = (uint32_t)buf;

	DMA_Cmd(DmaTxStream_, ENABLE);
	return 1;
}

//
//uint8_t CUsart::read_RxFifo(uint8_t* buf)
//
uint16_t CUsart::read_RxFifo(uint8_t* buf)
{
	uint8_t rxDataSize;
	DMA_Cmd(DmaRxStream_, DISABLE);
	*RXDMA_IFCR_ = RXDMA_IFCR_TCIF_MASK;

	rxDataSize = rxBuffSize_ - DmaRxStream_->NDTR;
	if(rxDataSize == 0)
	{
		DMA_Cmd(DmaRxStream_, ENABLE);
		return 0;
	}

	/* check for overflow */
	if(rxDataSize > 10) rxOverflowCount_++;

	memcpy(buf, (uint8_t*)(DmaRxStream_->M0AR), rxDataSize);

	DmaRxStream_->NDTR = rxBuffSize_;
	DMA_Cmd(DmaRxStream_, ENABLE);

	/* enable stream until it was enabled */
	while((DmaRxStream_->CR&0x01) != 1)
	{
		*RXDMA_IFCR_ = RXDMA_IFCR_TCIF_MASK;
		DMA_Cmd(DmaRxStream_, ENABLE);
	}
	return rxDataSize;
}

//
//uint8_t CUsart::get_BytesInRxFifo()
//
uint16_t CUsart::get_BytesInRxFifo()
{
	uint8_t size = rxBuffSize_ - DmaRxStream_->NDTR;

	return size;
}

//
//uint16_t CUsart::get_BytesInTxFifo()
//
uint16_t CUsart::get_BytesInTxFifo()
{
	uint8_t size = DmaTxStream_->NDTR;
	return size;
}

/**
  * @brief  set the Baudrate of UART
	* @param  None
  * @retval None
  */
void CUsart::clear_rxFifo()
{
	uint8_t rxDataSize;
	DMA_Cmd(DmaRxStream_, DISABLE);
	*RXDMA_IFCR_ = RXDMA_IFCR_TCIF_MASK;

	rxDataSize = rxBuffSize_ - DmaRxStream_->NDTR;
	if(rxDataSize == 0)
	{
		DMA_Cmd(DmaRxStream_, ENABLE);
		return ;
	}

	DmaRxStream_->NDTR = rxBuffSize_;
	DMA_Cmd(DmaRxStream_, ENABLE);

	forceEnableRxDma();
}

//
//void CUsart::InitSciGpio(int IOGroup)
//
void CUsart::InitSciGpio()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	uint32_t RCC_AHB1Periph_GPIOx;
	uint8_t GPIO_PinSource_Tx;
	uint8_t GPIO_PinSource_Rx;
	uint8_t GPIO_AF_USARTx;
	GPIO_TypeDef *GPIOx_Tx;
	GPIO_TypeDef *GPIOx_Rx;
	IOGroup_Type IOGroup = BspIOGroup(USARTx_);

	if(IOGroup == GROUP_A2A3)
	{
		RCC_AHB1Periph_GPIOx = RCC_AHB1Periph_GPIOA;
		GPIOx_Tx = GPIOx_Rx = GPIOA;
		GPIO_PinSource_Tx = GPIO_PinSource2;
		GPIO_PinSource_Rx = GPIO_PinSource3;
	}
	else if(IOGroup == GROUP_A9A10)
	{
		RCC_AHB1Periph_GPIOx = RCC_AHB1Periph_GPIOA;
		GPIOx_Tx = GPIOx_Rx = GPIOA;
		GPIO_PinSource_Tx = GPIO_PinSource9;
		GPIO_PinSource_Rx = GPIO_PinSource10;
	}
	else if(IOGroup == GROUP_B10B11)
	{
		RCC_AHB1Periph_GPIOx = RCC_AHB1Periph_GPIOB;
		GPIOx_Tx = GPIOx_Rx = GPIOB;
		GPIO_PinSource_Tx = GPIO_PinSource10;
		GPIO_PinSource_Rx = GPIO_PinSource11;
	}
	else if(IOGroup == GROUP_C10C11)
	{
		RCC_AHB1Periph_GPIOx = RCC_AHB1Periph_GPIOC;
		GPIOx_Tx = GPIOx_Rx = GPIOC;
		GPIO_PinSource_Tx = GPIO_PinSource10;
		GPIO_PinSource_Rx = GPIO_PinSource11;
	}
	else if(IOGroup == GROUP_C12D2)
	{
		RCC_AHB1Periph_GPIOx = RCC_AHB1Periph_GPIOD|RCC_AHB1Periph_GPIOC;
		GPIOx_Tx = GPIOC;
		GPIOx_Rx = GPIOD;
		GPIO_PinSource_Tx = GPIO_PinSource12;
		GPIO_PinSource_Rx = GPIO_PinSource2;
	}
	else if(IOGroup == GROUP_C6C7)
	{
		RCC_AHB1Periph_GPIOx = RCC_AHB1Periph_GPIOC;
		GPIOx_Tx = GPIOx_Rx = GPIOC;
		GPIO_PinSource_Tx = GPIO_PinSource6;
		GPIO_PinSource_Rx = GPIO_PinSource7;
	}
	else if(IOGroup == GROUP_D5D6)
	{
		RCC_AHB1Periph_GPIOx = RCC_AHB1Periph_GPIOD;
		GPIOx_Tx = GPIOx_Rx = GPIOD;
		GPIO_PinSource_Tx = GPIO_PinSource5;
		GPIO_PinSource_Rx = GPIO_PinSource6;
	}
	else if(IOGroup == GROUP_D8D9)
	{
		RCC_AHB1Periph_GPIOx = RCC_AHB1Periph_GPIOD;
		GPIOx_Tx = GPIOx_Rx = GPIOD;
		GPIO_PinSource_Tx = GPIO_PinSource8;
		GPIO_PinSource_Rx = GPIO_PinSource9;
	}
	else seer_assert(false); //undefined!

	if(USARTx_ == USART1)	GPIO_AF_USARTx = GPIO_AF_USART1;
	else if(USARTx_ == USART2)	GPIO_AF_USARTx = GPIO_AF_USART2;
	else if(USARTx_ == USART3)	GPIO_AF_USARTx = GPIO_AF_USART3;
	else if(USARTx_ == UART4)	GPIO_AF_USARTx = GPIO_AF_UART4;
	else if(USARTx_ == UART5)	GPIO_AF_USARTx = GPIO_AF_UART5;
	else if(USARTx_ == USART6)	GPIO_AF_USARTx = GPIO_AF_USART6;
	else seer_assert(false); //undefined!

	/* open clock of GPIO */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOx, ENABLE);

	/* Config Pin: TXD RXD*/
	GPIO_PinAFConfig(GPIOx_Tx, GPIO_PinSource_Tx, GPIO_AF_USARTx);
	GPIO_PinAFConfig(GPIOx_Rx, GPIO_PinSource_Rx, GPIO_AF_USARTx);

	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0<< GPIO_PinSource_Tx;
	GPIO_Init(GPIOx_Tx, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0<< GPIO_PinSource_Rx;
	GPIO_Init(GPIOx_Rx, &GPIO_InitStructure);
}

/**
  * @brief  Initialize the USART, which is called SCI(serial communication interface) in DSP
	* @param  rxBuf: rxBuf address
  * @retval None
  */
void CUsart::InitSci()
{
	/* init clock of USART */
	if(USARTx_ == USART1) RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	else if(USARTx_ == USART2) RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	else if(USARTx_ == USART3) RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
	else if(USARTx_ == UART4) RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);
	else if(USARTx_ == UART5) RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);
	else if(USARTx_ == USART6) RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART6, ENABLE);
	else seer_assert(false);

	/* Deinitializes the USARTx */
	USART_DeInit(USARTx_);

	USART_InitTypeDef USART_InitStructure;

	USART_InitStructure.USART_BaudRate = baudRate_;
	if(USART_Parity_No == parity_)
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	else
		USART_InitStructure.USART_WordLength = USART_WordLength_9b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = parity_;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;

	USART_Cmd(USARTx_, DISABLE);
	USART_Init(USARTx_, &USART_InitStructure);
	USART_Cmd(USARTx_, ENABLE);

	USART_DMACmd(USARTx_, USART_DMAReq_Tx, DISABLE);
	USART_DMACmd(USARTx_, USART_DMAReq_Rx, DISABLE);

	/* DMA Clock Config */
	uint32_t RCC_AHB1Periph;
	DMA_TypeDef * DMAx = ((DMA_TypeDef *)((uint32_t)DmaTxStream_&0xFFFFFC00));

	if(DMAx == DMA1) RCC_AHB1Periph = RCC_AHB1Periph_DMA1;
	else if(DMAx == DMA2) RCC_AHB1Periph = RCC_AHB1Periph_DMA2;
	else seer_assert(false); //error
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
	if(USARTx_ == USART6)
		DMA_InitStructure.DMA_Channel = DMA_Channel_5; //attention!
	else
		DMA_InitStructure.DMA_Channel = DMA_Channel_4; //attention!

	/* TX DMA Config */
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(USARTx_->DR);
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)"0";
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
	DMA_InitStructure.DMA_BufferSize = 0;

	DMA_Cmd(DmaTxStream_, DISABLE);
	*TXDMA_IFCR_ = TXDMA_IFCR_TCIF_MASK;
	DMA_Init(DmaTxStream_, &DMA_InitStructure);

	/* RX DMA Config */
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(USARTx_->DR);
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)rxBuff_;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
	DMA_InitStructure.DMA_BufferSize = rxBuffSize_;

	DMA_Cmd(DmaRxStream_, DISABLE);
	*RXDMA_IFCR_ = RXDMA_IFCR_TCIF_MASK;
	DMA_Init(DmaRxStream_, &DMA_InitStructure);
	DMA_Cmd(DmaRxStream_, ENABLE);

	USART_DMACmd(USARTx_, USART_DMAReq_Tx, ENABLE);
	USART_DMACmd(USARTx_, USART_DMAReq_Rx, ENABLE);
}

/**
  * @brief  set the Baudrate of UART
	* @param  None
  * @retval None
  */
void CUsart::setBaudrate(uint32_t BaudRate = 0)
{
	uint32_t tmpreg = 0x00, apbclock = 0x00;
  uint32_t integerdivider = 0x00;
  uint32_t fractionaldivider = 0x00;
  RCC_ClocksTypeDef RCC_ClocksStatus;

	/* update baudrate_ if BaudRate is effective*/
	if(BaudRate > 0) baudRate_ = BaudRate;

	/* check if the Usart is opened */
	bool isUsartOpen = (USARTx_->CR1 & USART_CR1_UE);

	/* close the USART if it is opened */
	if(isUsartOpen) USART_Cmd(USARTx_, DISABLE);

	assert_param(IS_USART_BAUDRATE(baudRate_));
	/* Configure the USART Baud Rate */
  RCC_GetClocksFreq(&RCC_ClocksStatus);

  if ((USARTx_ == USART1) || (USARTx_ == USART6))
  {
    apbclock = RCC_ClocksStatus.PCLK2_Frequency;
  }
  else
  {
    apbclock = RCC_ClocksStatus.PCLK1_Frequency;
  }

  /* Determine the integer part */
  if ((USARTx_->CR1 & USART_CR1_OVER8) != 0)
  {
    /* Integer part computing in case Oversampling mode is 8 Samples */
    integerdivider = ((25 * apbclock) / (2 * baudRate_));
  }
  else /* if ((USARTx_->CR1 & USART_CR1_OVER8) == 0) */
  {
    /* Integer part computing in case Oversampling mode is 16 Samples */
    integerdivider = ((25 * apbclock) / (4 * baudRate_));
  }
  tmpreg = (integerdivider / 100) << 4;

  /* Determine the fractional part */
  fractionaldivider = integerdivider - (100 * (tmpreg >> 4));

  /* Implement the fractional part in the register */
  if ((USARTx_->CR1 & USART_CR1_OVER8) != 0)
  {
    tmpreg |= ((((fractionaldivider * 8) + 50) / 100)) & ((uint8_t)0x07);
  }
  else /* if ((USARTx_->CR1 & USART_CR1_OVER8) == 0) */
  {
    tmpreg |= ((((fractionaldivider * 16) + 50) / 100)) & ((uint8_t)0x0F);
  }

  /* Write to USART BRR register */
  USARTx_->BRR = (uint16_t)tmpreg;

	/* recover the state of USART */
	if(isUsartOpen) USART_Cmd(USARTx_, ENABLE);
}

/**
  * @brief  set the parity of UART
	* @param  the def of parity in stm32f4xx_usart.h
  * @retval None
  */
void CUsart::setParity(uint16_t parity)
{
	if (IS_USART_PARITY(parity))
		parity_ = parity;
}

/**
  * @brief  get overflow count of Rx buffer
	* @param  None
  * @retval Number of overflow
  */
uint8_t CUsart::getRxOverflowCount()
{
	return rxOverflowCount_;
}

/**
  * @brief  judge if usart data flow is break
	* @param  None
  * @retval true or false
	* @note 	this function is fucking useful for modbus
  */
bool CUsart::isDataFlowBreak()
{
	bool ret = (SET == USART_GetFlagStatus(USARTx_, USART_FLAG_IDLE));
	
	if(!ret)
		return ret;
	
	uint8_t temp = USARTx_->DR;
	temp++;
	return ret;
}

/**
  * @brief  TxDMA()
	* @param  None
  * @retval Tx Dma Channel of the Usart
  */
DMA_Stream_TypeDef * CUsart::TxDMA(USART_TypeDef * targetUart)
{
	if(USART1 == targetUart)
		return DMA2_Stream7;
	else if(USART2 == targetUart)
		return DMA1_Stream6;
	else if(USART3 == targetUart)
		return DMA1_Stream3;
	else if(UART4 == targetUart)
		return DMA1_Stream4;
	else if(UART5 == targetUart)
		return DMA1_Stream7;
	else if(USART6 == targetUart)
		return DMA2_Stream6;
	else
		seer_assert(false);

	//program should not come here
	return DMA2_Stream7;
}

/**
  * @brief  RxDMA()
	* @param  None
  * @retval Rx Dma Channel of the Usart
  */
DMA_Stream_TypeDef * CUsart::RxDMA(USART_TypeDef * targetUart)
{
	if(USART1 == targetUart)
		return DMA2_Stream2;
	else if(USART2 == targetUart)
		return DMA1_Stream5;
	else if(USART3 == targetUart)
		return DMA1_Stream1;
	else if(UART4 == targetUart)
		return DMA1_Stream2;
	else if(UART5 == targetUart)
		return DMA1_Stream0;
	else if(USART6 == targetUart)
		return DMA2_Stream1;
	else
		seer_assert(false);

	//program should not come here
	return DMA2_Stream2;
}

/**
  * @brief  BspIOGroup
	* @param  None
  * @retval Tx Dma Channel of the Usart
  */
CUsart::IOGroup_Type CUsart::BspIOGroup(USART_TypeDef * targetUart)
{
#ifdef RUN_FOR_SRC_2000
	if(USART1 == targetUart)
		return GROUP_A9A10;
	else if(USART2 == targetUart)
		return GROUP_D5D6;
	else if(USART3 == targetUart)
		return GROUP_D8D9;
	else if(UART4 == targetUart)
		return GROUP_C10C11;
	else if(UART5 == targetUart)
		return GROUP_C12D2;
	else if(USART6 == targetUart)
		return GROUP_C6C7;
	else
		seer_assert(false);
#else
	if(USART1 == targetUart)
		return GROUP_A9A10;
	else if(USART2 == targetUart)
		return GROUP_A2A3;
	else if(USART3 == targetUart)
		return GROUP_B10B11;
	else if(UART4 == targetUart)
		return GROUP_C10C11;
	else if(UART5 == targetUart)
		return GROUP_C12D2;
	else if(USART6 == targetUart)
		return GROUP_C6C7;
	else
		seer_assert(false);
#endif
	//program should not come here
	return GROUP_A9A10;

}

/**
  * @brief  rx dma stream forceEnableRxDma
	* @param  None
  * @retval Number of overflow
  */
void CUsart::forceEnableRxDma()
{
	while((DmaRxStream_->CR&0x01) != 1)
	{
		*RXDMA_IFCR_ = RXDMA_IFCR_TCIF_MASK;
		DMA_Cmd(DmaRxStream_, ENABLE);
	}
}

/**
  * @brief  Clear error bytes of USART
	* @param  None
  * @retval Noe
	* @Note   PE (Parity error), FE (Framing error), NE (Noise error), ORE (OverRun
	*          error) and IDLE (Idle line detected) flags are cleared by software
  *          sequence: a read operation to USART_SR register (USART_GetFlagStatus())
  *          followed by a read operation to USART_DR register (USART_ReceiveData()).
  */
void CUsart::clearErr()
{
	int i = 0;

	if(0xFF == USARTx_->SR)
		i++;
	if(0xFF == USARTx_->DR)
		i++;
}

//
//destructor
//
CUsart::~CUsart()
{}
//end of file
