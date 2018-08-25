/********************************************************************************
* @file    CUdpConsole.h
* @author  Chenxx
* @version V1.0
* @date    2017-04-8
* @brief   this file defines the Console function that can printf with Udp.
*	   This is for STM32F4.
* release information: 
*	2017/04/08: create.
********************************************************************************/
// <<< Use Configuration Wizard in Context Menu >>>
#ifndef _CUDPCONSOLE_H_
#define _CUDPCONSOLE_H_
#include "stm32f4xx.h"
#include "Console.h"
#include "socket.h"

#define CONSOLE_UDP_REMOTE_PORT 4999

class CUdpConsole
	:public CConsole::COstreamDev
{
public:
	CUdpConsole();
	virtual ~CUdpConsole(){close();}

	virtual uint16_t write(uint8_t*, uint16_t);
//	virtual uint16_t write(ringque_base<char>& ref_que);
	virtual bool open();
	virtual void runTransmitter();
	virtual uint16_t getFreeSize();
	virtual bool isIdle();
	virtual int getDevType() const { return CConsole::UDP_DEV; }
	virtual bool isOpen();

private:
	uint8_t remote_ip_[4];
	uint16_t remote_port_;
	int socket_n_;
	uint16_t overflowCounter_;
	sockaddr_in remoteaddr_;

};


#endif
// <<< end of configuration section >>>
