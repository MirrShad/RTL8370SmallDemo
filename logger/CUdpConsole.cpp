/********************************************************************************
* @file    CUdpConsole.cpp
* @author  Chenxx
* @version V1.0
* @date    2016-04-21
* @brief   this file defines the Console function that can printf with can.
*	   This is for STM32F4.
********************************************************************************/
#include "CUdpConsole.h"
//#include "socket.h"
#include "NetworkAdapter.h"
#include <stdio.h>

#define UDP_CONSOLE_PORT 6666

/**
  * @brief  Constructor
	* @param  None
  * @retval None
  */
CUdpConsole::CUdpConsole()
	:remote_port_(CONSOLE_UDP_REMOTE_PORT),
	overflowCounter_(0),
	socket_n_(-1)
{
	const uint8_t broadcast_ip[4] = {192, 168, 192, 255};
	const uint8_t silent_ip[4] = {0, 0, 0, 0};
	
	if(memcmp(NetworkAdapter::Instance()->IP(), broadcast_ip, 3) == 0)
	{
		memcpy(remote_ip_, broadcast_ip, 4);
	}else
	{
		memcpy(remote_ip_, silent_ip, 4);
	}
	
	remoteaddr_.sin_family = AF_INET;
	remoteaddr_.sin_port = htons(CONSOLE_UDP_REMOTE_PORT);
	remoteaddr_.sin_addr.s_addr = byteston(remote_ip_);

}

/**
  * @brief  run UART transmitter, in another word TXD
  * @param  None
  * @retval None
  */
void CUdpConsole::runTransmitter()
{

}

/**
  * @brief  get free size
  * @param  None
  * @retval None
  */
uint16_t CUdpConsole::getFreeSize()
{
//	return getSn_TX_FSR(socket_n_);
	return 100;
}

/**
  * @brief  write data to UartConsole device
  * @param  srcBuf
	* @param  len
  * @retval bytes that actually wrote into it
  */
uint16_t CUdpConsole::write(uint8_t* srcBuf, uint16_t srcLen)
{
	int32_t ret = net::sendto(socket_n_, srcBuf, srcLen, MSG_DONTWAIT, (sockaddr *)&remoteaddr_, sizeof(remoteaddr_));
	if(ret < 0)
	{
		open();
		ret = 0;
	}
	return ret;
}

/**
  * @brief  Initialize the Usart of console
  * @param  None
  * @retval None
  */
bool CUdpConsole::open()
{
	if(!NetworkAdapter::Instance()->isOpen())
		return false;
	
	sockaddr_in localaddr;

	localaddr.sin_family = AF_INET;
	localaddr.sin_port = htons(UDP_CONSOLE_PORT);
	localaddr.sin_addr.s_addr = INADDR_ANY;

	net::close(socket_n_);	
	socket_n_ = net::socket(AF_INET, SOCK_DGRAM, 0);
	net::bind(socket_n_, (sockaddr *)&localaddr,  sizeof(localaddr));

	uint8_t printmsg[40];
	int len = snprintf((char*)printmsg, 40, "socket[%d] init for udpconsole...\r\n", socket_n_);
	int ret = net::sendto(socket_n_, printmsg, len, MSG_DONTWAIT, (sockaddr *)&remoteaddr_, sizeof(remoteaddr_));
	ret += 3;
	return true;
}

/**
* @brief  Is Transmitter idel or not
* @param  None
* @retval If is idel
*/
bool CUdpConsole::isIdle()
{
//	uint32_t byteInTxBuf = 0;
//	byteInTxBuf = getSn_TXBUF_SIZE(socket_n_) * 1024 - getFreeSize();
//	
//	return (0 == byteInTxBuf);
	return true;
}

bool CUdpConsole::isOpen()
{
	return (NetworkAdapter::Instance()->isOpen() && socket_n_ >= 0);
}
//end of file
