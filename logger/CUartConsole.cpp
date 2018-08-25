/********************************************************************************
* @file    CUartConsole.cpp
* @author  Chenxx
* @version V1.0
* @date    2016-04-21
* @brief   this file defines the Console function that can printf with uart.
*	   This is for STM32F4.
********************************************************************************/
#include "CUartConsole.h"
#include <string.h>

uint8_t CUartConsole::TxDmaBuf_[TX_DMA_SIZE];	//for txDma
/* emulate dsp sci FIFO buffer */
const uint8_t CONSOLE_RX_BUF_LEN = 20;
uint8_t console_rxbuf[CONSOLE_RX_BUF_LEN];
CUsart consoleUsart(CONSOLE_UART, console_rxbuf, CONSOLE_RX_BUF_LEN);	

/**
  * @brief  Constructor
	* @param  None
  * @retval None
  */
CUartConsole::CUartConsole()
	:overflowCounter_(0)
{
	
}

/**
  * @brief  run UART transmitter, in another word TXD
  * @param  None
  * @retval None
  */
void CUartConsole::runTransmitter()
{
//	if(0 != DMA_GetCurrDataCounter(CUsart::TxDMA(CONSOLE_UART)))
//		return;

//	uint8_t sendLen = txQueue_.pop_array(TxDmaBuf_, TX_DMA_SIZE);
//	consoleUsart.send_Array((uint8_t*)TxDmaBuf_, sendLen);
}

/**
  * @brief  write data to UartConsole device
  * @param  srcBuf
	* @param  len
  * @retval bytes that actually wrote into it
  */
uint16_t CUartConsole::write(uint8_t* srcBuf, uint16_t srcLen)
{
	if(0 != DMA_GetCurrDataCounter(CUsart::TxDMA(CONSOLE_UART)))
		return 0;
	
	memcpy(TxDmaBuf_, srcBuf, srcLen);
	return consoleUsart.send_Array(TxDmaBuf_, srcLen);
}

/**
  * @brief  Initialize the Usart of console
  * @param  None
  * @retval None
  */
bool CUartConsole::open()
{
	consoleUsart.InitSciGpio();
	consoleUsart.InitSci();
	return true;
}

/**
* @brief  Is Transmitter idel or not
* @param  None
* @retval If is idel
*/
bool CUartConsole::isIdle()
{
	runTransmitter();
	return (0 == DMA_GetCurrDataCounter(CUsart::TxDMA(CONSOLE_UART))
		&& 1 == USART_GetFlagStatus(CONSOLE_UART, USART_FLAG_TXE));
}

/**
* @brief  Is Transmitter idel or not
* @param  None
* @retval If is idel
*/
uint16_t CUartConsole::getFreeSize() 
{
	if(0 != DMA_GetCurrDataCounter(CUsart::TxDMA(CONSOLE_UART)))
		return 0;
	else 
		return TX_DMA_SIZE;
}

//end of file
