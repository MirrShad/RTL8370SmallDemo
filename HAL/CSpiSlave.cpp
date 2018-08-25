#include "CSpiSlave.h"



CSpiSlave::CSpiSlave(SPI_TypeDef * SPIx_)
					:SPIx_(SPIx_)
				,_rcvTimeout(this,&CSpiSlave::rcvTimeourCallback,100)
				,usingDMA(false)
{}

uint8_t CSpiSlave::DMA_ConvertedValue[4];
void CSpiSlave::InitSpiGpio()
{
  GPIO_InitTypeDef GPIO_InitStructure; 
	uint32_t RCC_AHB1Periph_GPIOx;
	uint8_t GPIO_PinSource_Clk;
	uint8_t GPIO_PinSource_Rx;
	//uint8_t GPIO_PinSource_Tx;
  uint8_t GPIO_PinSource_CS;
	uint8_t GPIO_AF_SPIx;
	GPIO_TypeDef *GPIOx_Clk;
	//GPIO_TypeDef *GPIOx_Tx;
	GPIO_TypeDef *GPIOx_Rx;
  GPIO_TypeDef *GPIOx_CS;
	
	/* open clock of GPIO */
	GPIOx_Clk  = GPIOx_Rx = GPIOB;//= GPIOx_Tx
  RCC_AHB1Periph_GPIOx = RCC_AHB1Periph_GPIOA<< (((uint32_t)GPIOx_Clk-(uint32_t)GPIOA)/0x400);;
	GPIO_PinSource_Clk = GPIO_PinSource3;//todo:use IO group to init
	//GPIO_PinSource_Tx = GPIO_PinSource4;//todo: reuse when new version of SRC2000 is used
	GPIO_PinSource_Rx = GPIO_PinSource5;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOx, ENABLE);
  GPIOx_CS = GPIOA;
  RCC_AHB1Periph_GPIOx = RCC_AHB1Periph_GPIOA<< (((uint32_t)GPIOx_CS-(uint32_t)GPIOA)/0x400);;
  GPIO_PinSource_CS = GPIO_PinSource15;
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOx, ENABLE);

	
	if(SPIx_ == SPI1)	GPIO_AF_SPIx = GPIO_AF_SPI1;
	else if(SPIx_ == SPI2)	GPIO_AF_SPIx = GPIO_AF_SPI2;
	else if(SPIx_ == SPI3)	GPIO_AF_SPIx = GPIO_AF_SPI3;
	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0<< GPIO_PinSource_Clk;
	GPIO_Init(GPIOx_Clk, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0<< GPIO_PinSource_Rx;
	GPIO_Init(GPIOx_Rx, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0<< GPIO_PinSource_CS;
	GPIO_Init(GPIOx_CS, &GPIO_InitStructure);
	
	//GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0<< GPIO_PinSource_Tx;
	//GPIO_Init(GPIOx_Tx, &GPIO_InitStructure);
	
	/* Config Pin: TXD RXD*/
	GPIO_PinAFConfig(GPIOx_Clk, GPIO_PinSource_Clk, GPIO_AF_SPIx);
	GPIO_PinAFConfig(GPIOx_Rx, GPIO_PinSource_Rx, GPIO_AF_SPIx);
	//GPIO_PinAFConfig(GPIOx_Tx, GPIO_PinSource_Tx, GPIO_AF_SPIx);
  GPIO_PinAFConfig(GPIOx_CS, GPIO_PinSource_CS, GPIO_AF_SPIx);
	
	
	
	_rcvTimeout.enable();
}



void CSpiSlave::InitSpi()
{
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
	
	/* Deinitializes the SPIx */
	SPI_DeInit(SPIx_);
	
/* Config SPI */
	SPI_InitTypeDef SPI_InitStructure;

	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_RxOnly;	//Full Duplex
	SPI_InitStructure.SPI_Mode = SPI_Mode_Slave;												//master/slave mode
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;										//
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;													//keep SCK high when free
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;			                 	//
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;                          	//NSS by software
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;	//baud rate
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;                 	//Endian
	SPI_InitStructure.SPI_CRCPolynomial = 0x31;                            //CRC polygon
	
	SPI_Init(SPIx_, &SPI_InitStructure);
	
	SPI_Cmd(SPIx_, DISABLE); 
	SPI_Init(SPIx_, &SPI_InitStructure); 
	SPI_CalculateCRC(SPIx_,ENABLE);
	SPI_Cmd(SPIx_, ENABLE); 
	SPI_I2S_ClearITPendingBit(SPIx_, SPI_I2S_IT_RXNE);
	SPI_NSSInternalSoftwareConfig(SPI3,SPI_NSSInternalSoft_Set);
}

void CSpiSlave::InitDMA(void)
{
	DMA_InitTypeDef  DMA_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1,ENABLE);
	DMA_DeInit(DMA1_Stream0);
	
	DMA_InitStructure.DMA_Channel = DMA_Channel_0;  //通道选择
  DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&SPI3->DR;//DMA外设地址
  DMA_InitStructure.DMA_Memory0BaseAddr = (u32)DMA_ConvertedValue;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
	DMA_InitStructure.DMA_BufferSize = 1;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;//中等优先级
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;//存储器突发单次传输
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;//外设突发单次传输
	DMA_Init(DMA1_Stream0, &DMA_InitStructure);
	
	DMA_SetCurrDataCounter(DMA1_Stream0,4);
	DMA_Cmd (DMA1_Stream0,ENABLE); 
	
	SPI_I2S_DMACmd(SPI3,SPI_I2S_DMAReq_Rx,ENABLE);
	usingDMA = true;
}	

void CSpiSlave::rcvTimeourCallback()
{
	
}

uint16_t CSpiSlave::getNum()
{
	//if(!usingDMA)
		//_rcvTimeout.reset();
		if(SPI_I2S_GetFlagStatus(SPIx_,SPI_I2S_FLAG_RXNE)==RESET) return 0x0000;
			//if(_rcvTimeout.isAbsoluteTimeUp()) 
				
		return SPI_ReceiveData(SPIx_);
	
	
	//return DMA_ConvertedValue[id];
}
