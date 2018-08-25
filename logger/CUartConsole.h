/********************************************************************************
* @file    CUartConsole.h
* @author  Chenxx
* @version V1.0
* @date    2016-04-21
* @brief   this file defines the Console function that can printf with uart.
*	   This is for STM32F4.
* release information: 
*	2016/07/01: chenxx add noneDMA mode.
* 2016/07/24: chenxx fixed the bug in printf: stop sending when overflow
* 2016/08/26: chenxx completed the TX DMA mode.
********************************************************************************/
// <<< Use Configuration Wizard in Context Menu >>>
#ifndef _CUARTCONSOLE_H_
#define _CUARTCONSOLE_H_
#include "stm32f4xx.h"
#include "Console.h"
#include "CUsart.h"

// <o> Console on USART: <1=>USART 1 <2=>USART 2 <3=> USART 3 
// <4=> USART 4 <5=> USART 5 <6=> USART 6
// 	<i>Default: 1
#define STM32_CONSOLE_USART 5

/* Macros for Board Support */

#if STM32_CONSOLE_USART == 1
	#define CONSOLE_UART				USART1
	
#elif STM32_CONSOLE_USART == 2
	#define CONSOLE_UART				USART2

#elif STM32_CONSOLE_USART == 3
	#define CONSOLE_UART				USART3
	
#elif STM32_CONSOLE_USART == 4
	#define CONSOLE_UART				UART4
	
#elif STM32_CONSOLE_USART == 5
	#define CONSOLE_UART				UART5
	
#elif STM32_CONSOLE_USART == 6
	#define CONSOLE_UART				USART6
	
#else
	#error
#endif

class CUartConsole
	:public CConsole::COstreamDev
{
public:
	CUartConsole();
	virtual ~CUartConsole(){close();}

	virtual uint16_t write(uint8_t*, uint16_t);
	virtual bool open();
	virtual void runTransmitter();
	virtual uint16_t getFreeSize();
	virtual bool isIdle();
	virtual int getDevType() const { return CConsole::UART_DEV; }

private:
	enum{
		TX_DMA_SIZE = 32
	};
	static uint8_t TxDmaBuf_[TX_DMA_SIZE];	//for txDma
	uint16_t overflowCounter_;

};


#endif
// <<< end of configuration section >>>
