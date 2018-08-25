#include "NetworkAdapter.h"
#include "LAN8720.h"
#include "Console.h"
#include "lwip_comm.h"

CNetworkAdapter::CNetworkAdapter():_periodTimer(this, &CNetworkAdapter::periodHandler, 1), _isOpen(false)
{
	
}

int CNetworkAdapter::doInit()
{
	_periodTimer.enable();
	
	if(LAN8720_Init())return -2;			//初始化LAN8720失败 
	if(lwip_comm_init()!=0)
	{
		Console::Instance()->printf("lwIP Init failed!\r\n");
		return -1;
	}
	
	uint8_t speed = LAN8720_Get_Speed();//�õ�����
	if(speed&1<<1)Console::Instance()->printf("Ethernet Speed:100M\r\n");
	else Console::Instance()->printf("Ethernet Speed:10M\r\n");
	
	setOpen(true);
	return 0;
}

int CNetworkAdapter::doRun()
{
	return 0;
}

int CNetworkAdapter::queryIpInfoHandler(uint8_t* msg, uint16_t len)
{
	return 0;
}

int CNetworkAdapter::setIpHandler(uint8_t* pbData, uint16_t len)
{
	return 0;
}

int CNetworkAdapter::RstCommandHandler(uint8_t* argv, uint16_t argc)
{
	return 0;
}

uint8_t* CNetworkAdapter::IP()
{
	return (uint8_t*)"\xC0\xA8\xC0\x04";
}

uint8_t* CNetworkAdapter::NetMask()
{
	return (uint8_t*)"\xFF\xFF\xFF\x00";
}

void CNetworkAdapter::setOpen(bool val)
{
	_isOpen = val;
}

bool CNetworkAdapter::isOpen()
{
	return _isOpen;
}

void CNetworkAdapter::periodHandler()
{
	lwip_periodic_handle();
}

//end of file
