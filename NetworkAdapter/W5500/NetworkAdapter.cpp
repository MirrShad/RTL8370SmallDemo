#include "NetworkAdapter.h"
#include "W5500Device.h"

CNetworkAdapter::CNetworkAdapter():_periodTimer(this, &CNetworkAdapter::periodHandler, 100)
{

}

int CNetworkAdapter::doInit()
{
	_periodTimer.enable();
	return W5500Device::Instance()->doInit();
}

int CNetworkAdapter::doRun()
{
	return 0;
}

int CNetworkAdapter::queryIpInfoHandler(uint8_t* msg, uint16_t len)
{
	return W5500Device::Instance()->queryIpInfoHandler(msg, len);
}

int CNetworkAdapter::setIpHandler(uint8_t* pbData, uint16_t len)
{
	return W5500Device::Instance()->setIpHandler(pbData, len);
}

int CNetworkAdapter::RstCommandHandler(uint8_t* argv, uint16_t argc)
{
	return W5500Device::Instance()->w5500RstCommandHandler(argv, argc);
}

uint8_t* CNetworkAdapter::IP()
{
	return W5500Device::Instance()->IP();
}

uint8_t* CNetworkAdapter::NetMask()
{
	return W5500Device::Instance()->NetMask();
}

void CNetworkAdapter::setOpen(bool val)
{
	W5500Device::Instance()->setOpen(val);
}

bool CNetworkAdapter::isOpen()
{
	return W5500Device::Instance()->isOpen();
}

void CNetworkAdapter::periodHandler()
{
	W5500Device::Instance()->doRun();
}

//end of file
