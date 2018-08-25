#ifndef _NETWORK_ADAPTER_H_
#define _NETWORK_ADAPTER_H_

//#include "Device.h"
#include "Singleton.h"
#include <stdint.h>
#include "Timer.h"

class CNetworkAdapter
{
public:
	CNetworkAdapter();
	
	int doInit();
	int doRun();
	void setOpen(bool open);
	bool isOpen();
	int queryIpInfoHandler(uint8_t* msg, uint16_t len);
	int setIpHandler(uint8_t* msg, uint16_t len);
	int RstCommandHandler(uint8_t* argv=(uint8_t*)0, uint16_t argc=0);
			
	uint8_t* IP();
	uint8_t* NetMask();

private:
	
	void periodHandler();
	CAsyncTimer<CNetworkAdapter> _periodTimer;
	bool _isOpen;

};

typedef NormalSingleton<CNetworkAdapter> NetworkAdapter;
#endif
//
