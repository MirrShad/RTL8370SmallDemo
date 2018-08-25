/********************************************************************************
* @file    CConsole.h
* @author  Chenxx
* @version V1.0
* @date    2016-04-21
* @brief   this file defines the Console function. Only for output logs. 
*	   			No receive function. Do not need to care about hardware platform.
* release information: 
*	2016/07/01: chenxx add noneDMA mode.
* 2016/07/24: chenxx fixed the bug in printf: stop sending when overflow
* 2016/08/26: chenxx completed the TX DMA mode.
********************************************************************************/
#ifndef _CONSOLE_H_
#define _CONSOLE_H_

#include "Singleton.h"
#include "ringque.h"

class CConsole
{
public:
	enum OstreamDevEnum
	{
		NULL_DEV = 0,
		UART_DEV,
		CAN_DEV,
		RTT_DEV,
		UDP_DEV,
		NUM_OF_DEV
	};

	CConsole();
	~CConsole();
	uint16_t send_array(char*, uint16_t);
	int printf(const char* fmt, ...) __attribute__((format(printf,2,3)));
	void putc(const char c);
	void puts(const char* s);
	enum{
		TXBUF_SIZE = 512,
		VSPRINT_SIZE = 128
	};
	void postErr();
	void runTransmitter();
	void run();
	bool isIdle();
	int setDev(uint8_t* cmd, uint16_t len);
	int initDev(OstreamDevEnum dev);
	OstreamDevEnum getDevType() const { return (OstreamDevEnum)ConsoleDev_->getDevType(); }

public:
	class COstreamDev
	{
		public:
			virtual ~COstreamDev(){close();}
			virtual uint16_t write(uint8_t*, uint16_t) = 0;
			virtual bool open() = 0;
			virtual bool close() {return true;}
			virtual void runTransmitter() {}
			virtual uint16_t getFreeSize() = 0;
			virtual bool isIdle() = 0;
			virtual int getDevType() const = 0;
			virtual bool isOpen() { return true; }
	};
	
private:
	static ringque<char, TXBUF_SIZE> txQueue_;
	static char vsnprintfBuf_[VSPRINT_SIZE]; //for sprintf
	uint16_t overflowCounter_;
	COstreamDev* ConsoleDev_;
};

typedef NormalSingleton<CConsole> Console;
#define postErr(msg) printf("Error: %s(%d)-%s(): %s\r\n", __FILE__, __LINE__, __FUNCTION__, msg)

#endif
//end of file
