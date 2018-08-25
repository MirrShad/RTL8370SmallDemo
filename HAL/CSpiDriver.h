/**
  ******************************************************************************
  * @file    CSpi.h
  * @author  Wuzeling
  * @version V1.0
  * @date    2018-04-15
  * @brief   This file defines the CSpi classes that simulates LINUX.
  ******************************************************************************/

#ifndef __CSPI_DRIVER_H
#define __CSPI_DRIVER_H

#include "fixed_vector.h"
#include "ringque.h"
#include "stm32f4xx.h"
class CSpi_master;
class CSpi_device;


enum Data_Size
{
	DATASIZE_8b = 0,
	DATASIZE_16b
};


typedef void(*CompleteHandler)();
class CSpi_message
{
	friend class CSpi_device;
	friend class CSpi_master;
public:
	CSpi_message(Data_Size = DATASIZE_8b);
	int addTxData(uint16_t);
	int addTxData(uint8_t* txMsg,int len);//remember to *2 when converting uint16_t* to uint8_t*
	int getRxData(uint16_t*);
	int getRxData(uint8_t* rxMsg,int len);	

	//getTxData should only be used by Spi_Master
	int getTxData(uint16_t& txMsg);
	//getRxData should only be used by Spi_Master
	int addRxData(uint16_t data);

	void setCompleteHandler(CompleteHandler handler);
	int dataInRxQue() const;
	int dataInTxQue() const;
	
	CSpi_message& operator=(const CSpi_message& str);
private:
	CSpi_device* spiDevice_;
	Data_Size datasize_;
	ringque<uint8_t, 100> txQue_;
	ringque<uint8_t, 100> rxQue_;	
	void (*complete)();  
};

class CSpi_device
{
	friend class CSpi_driver;
	friend class CSpi_master;
public:
	Data_Size getDataSize();
	int AsyncWrite(CSpi_message& msg);
	int AsyncRead(CSpi_message& msg,int len);
	int SyncWrite(CSpi_message& msg);
	int SyncRead(CSpi_message& msg,int len);
	
	void chip_select();
	void chip_reject();
private:
	//uint8_t chip_select_
	CSpi_master* master;
	uint8_t chip_select_pin;
	GPIO_TypeDef * chip_select_port;
};

class CSpi_driver
{
public:
	int probe(CSpi_device*);
};

class CSpi_master
{
	friend class CSpi_device;
public:
	enum IOGroup_Type
	{
		GROUP_A5A6A7 = 0,
		GROUP_B3B4B5,
		GROUP_B13B14B15
	};
	enum CLK_Mode //this mode simulates spi mode in LINUX spi driver
	{
		MODE_0 = 0,	//SPI_MODE_0  (0|0)  
		MODE_1,			//SPI_MODE_1  (0|SPI_CPHA)  
		MODE_2,			//SPI_MODE_2  (SPI_CPOL|0)  
		MODE_3			//SPI_MODE_3  (SPI_CPOL|SPI_CPHA)  
	};
	enum{MAX_DEVICE_NUM = 3};
	
	CSpi_master(SPI_TypeDef * SPIx,CSpi_driver* driver,Data_Size datasize,CLK_Mode clkmode);
	
	//BSP related
	void InitSpiGpio();
	void InitSpi();
	static IOGroup_Type BspIOGroup(SPI_TypeDef *);
	void setCRC(uint16_t crc){crc_ = crc;};
	void setSoftwareTrig(){bHardWareTrig_ = false;};
	void setNotCtrlCLK(){bCtrlCLK_ = false;};//determine whether the clk is sent by us or external device
	
	//LINUX format related
	void doRun();
	void addDevice(CSpi_device*);
	bool isAsync(){return bIsAysn;};
	Data_Size getDataSize(){return dataSize_;};
	uint16_t sendMsg(uint16_t);//Temporary fucntion which will be deleted after OS is installed
private:
	SPI_TypeDef* SPIx_;
	bool bCtrlCLK_;
	bool bHardWareTrig_;
	Data_Size dataSize_;
	CLK_Mode clkMode_;
	uint16_t crc_;

	ringque<CSpi_message, 20> msgQue_;
	fixed_vector<CSpi_device*, MAX_DEVICE_NUM> deviceTab_;
	bool bIsSending;
	bool bIsAysn;
	CSpi_driver* driver_;
	
	int addTxMsg(const CSpi_message&);	
};


	

#endif
