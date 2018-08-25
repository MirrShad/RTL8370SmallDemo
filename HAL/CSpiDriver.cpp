#include "CSpiDriver.h"

#define assert(val)	seer_assert(val)

#ifndef assert
#define assert(val) while(1)
#endif

namespace{
	
}
/*********************************************************
****      this part is for CSpi_device        *****
*********************************************************/
/**
* @brief  send msg which may stop here
* @param  msg: message to be sent
* @retval error num
* @note we only allow async or sync for one master currently
*/
int CSpi_device::SyncWrite(CSpi_message& msg)
{
	//todo:: when using OS,change another way to syncwrite.
	//we now open a function to write temporary, laterly we will all combine into addTxMsg
	if(master->isAsync())
		return -1;
	
	if(this->master->getDataSize()!=msg.datasize_)
		return -2;
	
	msg.spiDevice_ =  this;
	if(DATASIZE_8b == this->master->getDataSize())
	{
		while(0 != msg.txQue_.elemsInQue())
		{
			msg.rxQue_.push(this->master->sendMsg(msg.txQue_.front()));
			msg.txQue_.pop();
		}
	}else if(DATASIZE_16b == this->master->getDataSize())
	{
		while(0 != msg.txQue_.elemsInQue())
		{
			uint16_t temp = msg.txQue_.front()<<8;
			msg.txQue_.pop();
			temp += msg.txQue_.front();
			msg.txQue_.pop();
			temp = this->master->sendMsg(temp);
			msg.rxQue_.push(uint8_t(temp));
			msg.rxQue_.push(temp>>8);
		}
	}
	else
		assert(false);	

	if(msg.complete!=NULL)
		msg.complete();
	
	return 0;
}
	
/**
* @brief  read msg which may stop here
* @param  msg: message containing the read msg
* @param  len: message want to read
* @retval error num
* @note we only allow async or sync for one master currently
*/
int CSpi_device::SyncRead(CSpi_message& msg,int len)
{
	if(master->isAsync())
		return -1;
	
	if(this->master->getDataSize()!=msg.datasize_)
		return -2;
	
	msg.spiDevice_ =  this;
	if(DATASIZE_8b == this->master->getDataSize())
		while(len-->0)
		{
			msg.addTxData(0xFF);
		}
	else if(DATASIZE_16b == this->master->getDataSize())
		while(len-->0)
		{
			msg.addTxData(0xFFFF);
		}
	else
		assert(false);	
	
	return SyncWrite(msg);
}

/**
* @brief  send msg won't stop here
* @param  msg: message to be sent
* @retval error num
* @note we only allow async or sync for one master currently
*/
int CSpi_device::AsyncWrite(CSpi_message& msg)
{
	if(!master->isAsync())
		return -1;
	
	if(this->master->getDataSize()!=msg.datasize_)
		return -2;
	
	msg.spiDevice_ =  this;
	this->master->addTxMsg(msg);
	return 0;
}


int CSpi_device::AsyncRead(CSpi_message& msg,int len)
{
	if(!master->isAsync())
		return -1;
	
	if(this->master->getDataSize()!=msg.datasize_)
		return -2;
	
	msg.spiDevice_ =  this;
	if(DATASIZE_8b == this->master->getDataSize())
		while(len-->0)
		{
			msg.addTxData(0xFF);
		}
	else if(DATASIZE_16b == this->master->getDataSize())
		while(len-->0)
		{
			msg.addTxData(0xFFFF);
		}
	else
		assert(false);	
	
	return AsyncWrite(msg);
}

Data_Size CSpi_device::getDataSize(){return master->getDataSize();};
/*********************************************************
****      this part is for CSpi_driver        *****
*********************************************************/
int CSpi_driver::probe(CSpi_device* device)
{
	uint32_t RCC_AHB1Periphx;
	GPIO_InitTypeDef  GPIO_InitStructure;
	
	RCC_AHB1Periphx	= RCC_AHB1Periph_GPIOA<< (((uint32_t)(device->chip_select_port)-(uint32_t)GPIOA)/0x400);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periphx, ENABLE);
	GPIO_InitStructure.GPIO_Pin = device->chip_select_pin;//PB3~5复用功能输出	
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//复用功能
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;//上拉
  GPIO_Init(device->chip_select_port, &GPIO_InitStructure);//初始化
	
	return 0;
}

/*********************************************************
****      this part is for CSpi_master        *****
*********************************************************/
CSpi_master::CSpi_master(SPI_TypeDef * SPIx,CSpi_driver* driver,Data_Size datasize,CLK_Mode clkmode):
	SPIx_(SPIx),
	driver_(driver),
	bCtrlCLK_(true),
	bHardWareTrig_(true),
	dataSize_(datasize),
	clkMode_(clkmode),
	crc_(0),
	bIsSending(false),
	bIsAysn(false)
{
}

void CSpi_master::InitSpiGpio()
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

CSpi_master::IOGroup_Type CSpi_master::BspIOGroup(SPI_TypeDef * targetSpi)
{
	if(SPI1 == targetSpi)
		return GROUP_A5A6A7;
	else if(SPI3 == targetSpi)
		return GROUP_B3B4B5;
	else
		assert(false);
	
	//program should not come here
	return GROUP_A5A6A7;
}

void CSpi_master::InitSpi()
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
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	
	if(bCtrlCLK_) SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	else SPI_InitStructure.SPI_Mode = SPI_Mode_Slave;

	uint16_t temp = dataSize_<<10;
	SPI_InitStructure.SPI_DataSize = temp;
	
	switch(clkMode_)
	{
		case MODE_0:
			SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;													//keep SCK high when free
			SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
			break;
		case MODE_1:
			SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;													//keep SCK high when free
			SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
			break;
		case MODE_2:
			SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;													//keep SCK high when free
			SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
			break;
		case MODE_3:
			SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;													//keep SCK high when free
			SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;	
			break;
		default:
			assert(false);
	}
	
	if(bHardWareTrig_) SPI_InitStructure.SPI_NSS = SPI_NSS_Hard;    
	else SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB; 
	
	if(0==crc_) SPI_InitStructure.SPI_CRCPolynomial = 7;     
	else SPI_InitStructure.SPI_CRCPolynomial = crc_;
	
	SPI_Cmd(SPIx_, DISABLE); 
	SPI_Init(SPIx_, &SPI_InitStructure); 
	if(0!=crc_) SPI_CalculateCRC(SPIx_,ENABLE);
	SPI_Cmd(SPIx_, ENABLE); 
	
	SPI_I2S_ClearITPendingBit(SPIx_, SPI_I2S_IT_RXNE);
	if(!bHardWareTrig_)SPI_NSSInternalSoftwareConfig(SPI3,SPI_NSSInternalSoft_Set);
}

uint16_t CSpi_master::sendMsg(uint16_t msg)
{
	SPI_I2S_ReceiveData(SPIx_);
	while (SPI_I2S_GetFlagStatus(SPIx_, SPI_I2S_FLAG_TXE) == RESET);
	SPI_I2S_SendData(SPIx_, msg);

  while (SPI_I2S_GetFlagStatus(SPIx_, SPI_I2S_FLAG_RXNE) == RESET);
	return SPI_I2S_ReceiveData(SPIx_);
}

int CSpi_master::addTxMsg(const CSpi_message& msg)
{
	msgQue_.push(msg);
	return 0;
}

void CSpi_master::doRun()
{
	if(!bIsAysn)
		return;
	
	//receive
	if(bIsSending && (SPI_I2S_GetFlagStatus(SPIx_, SPI_I2S_FLAG_RXNE) != RESET))
	{
		bIsSending = false;
		msgQue_.front().addRxData(SPI_I2S_ReceiveData(SPIx_));
		if(0==msgQue_.front().dataInTxQue())
		{
			msgQue_.front().spiDevice_->chip_reject();
			msgQue_.front().complete();
			msgQue_.pop();
		}			
	}
	
	//send
	if(bIsSending) return;
	if(msgQue_.empty()) return;	
	uint16_t Txdata;
	msgQue_.front().getTxData(Txdata);
	
	bIsSending = true;
	SPI_I2S_ReceiveData(SPIx_);
	msgQue_.front().spiDevice_->chip_select();
	SPI_I2S_SendData(SPIx_, Txdata);
	
}

void CSpi_master::addDevice(CSpi_device* device)
{
	driver_->probe(device);
}
/*********************************************************
****      this part is for CSpi_message        *****
*********************************************************/
CSpi_message::CSpi_message(Data_Size size)
{
	datasize_ = size;
	complete = NULL;
}

int CSpi_message::addTxData(uint16_t data)
{
	if(DATASIZE_8b == datasize_)
	{
		txQue_.push(data);
	}else if(DATASIZE_16b == datasize_)
	{
		txQue_.push(data>>8);
		txQue_.push(data);
	}
	else assert(false);
	
	return 0;
}

/**
* @brief  push array into queue
* @param  buf: pointer of the array
* @param  len: length of the array
* @retval the length actually pushed
*/
int CSpi_message::addTxData(uint8_t* txMsg,int len)//remember to *2 when converting uint16_t* to uint8_t*
{
	return txQue_.push_array(txMsg,len);
}

/**
* @brief  BspIOGroup
* @param  data: the data in the top of rxQue
* @retval the number of data is pop from rxQue_
*/
int CSpi_message::getRxData(uint16_t* data)
{
	if(rxQue_.empty()) return 0;
	if(DATASIZE_8b == datasize_)
	{
		*data = rxQue_.front();
		rxQue_.pop();
	}
	else if(DATASIZE_16b == datasize_)
	{
		*data = rxQue_.front()<<8;
		rxQue_.pop();
		*data += rxQue_.front();
		rxQue_.pop();
	}
	
	return 1;
}
	
int CSpi_message::addRxData(uint16_t data)
{
	if(DATASIZE_8b == datasize_)
	{
		return rxQue_.push(data);
	}
	else if(DATASIZE_16b == datasize_)
	{
		rxQue_.push(data>>8);
		return rxQue_.push(data);
	}
	return 0;
}

/**
* @brief  pop array from rxQue
* @param  rxMsg: pointer of the array
* @retval the length actually get
*/
int CSpi_message::getTxData(uint16_t& data)
{
	if(txQue_.empty()) return 0;
	data = txQue_.front();
	txQue_.pop();
	return 1;
}

int CSpi_message::dataInRxQue() const
{
	return rxQue_.elemsInQue();
}

int CSpi_message::dataInTxQue() const
{
	return txQue_.elemsInQue();
}

CSpi_message& CSpi_message::operator=(const CSpi_message& str)
{
	spiDevice_ = str.spiDevice_;
	datasize_ = str.datasize_;
	txQue_ = str.txQue_;
	rxQue_ = str.rxQue_;
}
