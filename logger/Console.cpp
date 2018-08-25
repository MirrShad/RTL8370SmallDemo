/********************************************************************************
* @file    CConsole.h
* @author  Chenxx
* @version V1.0
* @date    2016-04-21
* @brief   this file defines the Console function. Adjusted from CConsole
*	   This is for STM32F4.
* release information:
*	2016/07/01: chenxx add noneDMA mode.
* 2016/07/24: chenxx fixed the bug in printf: stop sending when overflow
* 2016/08/26: chenxx completed the TX DMA mode.
********************************************************************************/
#include "Console.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "ConsoleConfig.h"

#if USE_MINI_PRINT
#	include "rtt_vsnprintf.h"
#else
#	include <stdio.h>
#endif

#if ENABLE_UART_CONSOLE
#	include "CUartConsole.h"
#endif

#if ENABLE_UDP_CONSOLE
#	include "CUdpConsole.h"
#endif

#if ENABLE_CAN_CONSOLE
#	include "CCanConsole.h"
#endif

#if ENABLE_RTT_CONSOLE
#	include "CRttConsole.h"
#endif

ringque<char, CConsole::TXBUF_SIZE> CConsole::txQueue_;
char CConsole::vsnprintfBuf_[VSPRINT_SIZE];	//for sprintf
/**
  * @brief  Constructor
	* @param  None
  * @retval None
  */
CConsole::CConsole()
	:overflowCounter_(0),
	ConsoleDev_(NULL)
{
	initDev((OstreamDevEnum)DEFAULT_CONSOLE);
}

/**
  * @brief  send array
  * @param  None
  * @retval char get
  */
uint16_t CConsole::send_array(char* buf, uint16_t len)
{
	uint16_t res;
	//TODO add mutex lock here
	res = txQueue_.push_array(buf, len);
	//TODO release mutex lock here
	runTransmitter();
	return res;
}

/**
  * @brief  printf a string without DMA controller.
	*					User should call the CConsole::run()
  * @param  None
  * @retval number of bytes were sent
  */
int CConsole::printf(const char* fmt, ...)
{
	va_list args;
	int n;
	int ret;

	//TODO lock vsnprintf mutex
	va_start(args, fmt);
	n = vsnprintf(vsnprintfBuf_, TXBUF_SIZE, fmt, args);
	va_end(args);
	if(n > TXBUF_SIZE) n = TXBUF_SIZE;

	ret = send_array(vsnprintfBuf_, n);
	//TODO release vsnprintf mutex
	return ret;
}

/**
  * @brief  for None Dma Mode
  * @param  char to send
  * @retval None
  */
void CConsole::putc(const char c)
{
	//TODO add mutex lock here
	txQueue_.push(c);
	//TODO release mutex lock here
	runTransmitter();
}

/**
  * @brief  for None Dma Mode
  * @param  string to send
  * @retval None
  */
void CConsole::puts(const char* s)
{
	//TODO add mutex lock here
	txQueue_.push_array((char*)s, strlen(s));
	//TODO release mutex lock here
	runTransmitter();
}

/**
  * @brief  run UART transmitter
  * @param  None
  * @retval None
  */
void CConsole::runTransmitter()
{
	if(NULL == ConsoleDev_)
		return;
	
	if(!ConsoleDev_->isOpen())
	{
		ConsoleDev_->open();
		return ;
	}

	if(!ConsoleDev_->isIdle())
		return;

	const uint16_t BUFF_SIZE = 256;
	uint8_t tempBuff[BUFF_SIZE];
	uint16_t copyLen = txQueue_.elemsInQue();

	//find the minium between: txQue.elems, ConsoleDev.freesize, BUFF_SIZE
	ConsoleDev_->runTransmitter();
	if(ConsoleDev_->getFreeSize() < copyLen)
	{
		//blockDevice = i;
		copyLen = ConsoleDev_->getFreeSize();
	}

	if(0 == copyLen) return;
	if(copyLen > BUFF_SIZE) copyLen = BUFF_SIZE;

	txQueue_.pop_array((char*)tempBuff, copyLen);
	//write data into dev
	ConsoleDev_->write(tempBuff, copyLen);
}

/**
  * @brief  confirm if all transmitter is Idel.
  * @param  None
  * @retval None
  */
bool CConsole::isIdle()
{
	if(ConsoleDev_ != NULL && !ConsoleDev_->isIdle())
	{
		return false;
	}
	return true;
}

/**
  * @brief  setDev
  * @param  cmd pointer
	* @param  len: command length
	* @retval 0: ok
	* @retval -1: initDev failed
	* @note   cmd:{devtype, portParam}
  */
int CConsole::setDev(uint8_t* cmd, uint16_t len)
{
	int dev = *(int*)cmd;
	int ret = -1;

	ret = initDev((OstreamDevEnum)dev);
	return ret;
//	if(0 != ret || NULL == ConsoleDev_)
//	{
//		return ret;
//	}

}

/**
  * @brief  initDev
  * @param  None
  * @retval None
  */
int CConsole::initDev(OstreamDevEnum dev)
{
	delete ConsoleDev_;
	ConsoleDev_ = NULL;
	
	switch(dev)
	{
#		if ENABLE_UART_CONSOLE
		case UART_DEV:
		{
			ConsoleDev_ = new CUartConsole;
			break;
		}
#		endif
#		if ENABLE_CAN_CONSOLE		
		case CAN_DEV:
		{
			ConsoleDev_ = new CCanConsole;
			break;
		}
#		endif
#		if ENABLE_RTT_CONSOLE	
		case RTT_DEV:
		{
			ConsoleDev_ = new CRttConsole;
			break;
		}
#		endif
#		if ENABLE_UDP_CONSOLE	
		case UDP_DEV:
		{
			ConsoleDev_ = new CUdpConsole;
			break;
		}
#		endif
		default:
			return -1;
	}
	
	return ConsoleDev_->open();
}


extern "C"
{
	int fputc(int ch, FILE* stream)
	{
		Console::Instance()->putc(ch);
		return ch;
	}
}

//end of file
