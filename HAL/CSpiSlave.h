
#ifndef __CSPI_SLAVE_H
#define __CSPI_SLAVE_H

#include "stm32f4xx.h"
#include "Timer.h"
class CSpiSlave
{
public:
    CSpiSlave(SPI_TypeDef *);
    enum IOGroup_Type
	{
		GROUP_A5A6A7 = 0,
		GROUP_B3B4B5,
		GROUP_B13B14B15
	};

    void InitSpiGpio();
		void InitSpi();
		void InitDMA();
		uint16_t getNum();
private:
    SPI_TypeDef* SPIx_;
		bool usingDMA;
		static uint8_t DMA_ConvertedValue[4];
		
		CAsyncTimer<CSpiSlave> _rcvTimeout;
		void rcvTimeourCallback();
};

#endif
