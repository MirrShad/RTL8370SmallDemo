/********************************************************************************
* @file    CCanConsole.h
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
#ifndef _CCANCONSOLE_H_
#define _CCANCONSOLE_H_
#include "stm32f4xx.h"
#include "Console.h"
#include "CCan.h"

// <o> Console on CAN: <1=>CAN1 <2=>CAN2
// 	<i>Default: 1
#define STM32_CONSOLE_CAN 2

/* Macros for Board Support */

#if STM32_CONSOLE_CAN == 1
	#define CONSOLE_CAN						CAN1
	#define CONSOLE_CAN_IOGROUP		CCanRouter::GROUP_A11
	#define CONSOLE_USE_CAN1 	1
	
#elif STM32_CONSOLE_CAN == 2
	#define CONSOLE_CAN						CAN2
	#define CONSOLE_CAN_IOGROUP		CCanRouter::GROUP_B12
	#define CONSOLE_USE_CAN2 	1
	
#else
	#error
#endif

class CCanConsole
	:public CConsole::COstreamDev
{
public:
	CCanConsole();
	virtual ~CCanConsole(){close();}

	virtual uint16_t write(uint8_t*, uint16_t);
	virtual bool open();
	virtual void runTransmitter();
	virtual uint16_t getFreeSize();
	virtual bool isIdle();
	virtual int getDevType() const { return CConsole::CAN_DEV; }

private:
	CCanRouter& canBaseRouter_;
	uint16_t overflowCounter_;

};


#endif
// <<< end of configuration section >>>
