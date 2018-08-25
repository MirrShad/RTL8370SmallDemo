#ifndef _FW_CUSART_H
#define _FW_CUSART_H
#include "stdint.h"
#include "stm32f4xx.h"
#include "F4Kxx_BSPmacro.h"
class CUsart
{
public:		
	CUsart(USART_TypeDef *,
		uint8_t* rxBuff,
		uint16_t rxBuffSize,
		uint32_t BaudRate = 115200);
	~CUsart();

	uint16_t send_Array(uint8_t*, uint16_t);
	uint16_t read_RxFifo(uint8_t*);
	uint16_t get_BytesInTxFifo();
	uint16_t get_BytesInRxFifo();
	void clear_rxFifo();

	void forceEnableRxDma();
	void InitSciGpio();
	void InitSci();
	void setBaudrate(uint32_t);
	void setParity(uint16_t);
	uint8_t getRxOverflowCount();
	void clearErr();
	bool isDataFlowBreak();
	
	enum IOGroup_Type
	{
		GROUP_A2A3 = 0,
		GROUP_A9A10,
		GROUP_B10B11,
		GROUP_C12D2,
		GROUP_C10C11,
		GROUP_C6C7,
		GROUP_D5D6,
		GROUP_D8D9
	};
	static DMA_Stream_TypeDef * RxDMA(USART_TypeDef *);
	static DMA_Stream_TypeDef * TxDMA(USART_TypeDef *);
	static IOGroup_Type BspIOGroup(USART_TypeDef *);
	static bool try_reg_UART(USART_TypeDef *, CUsart*);
	static void force_reg_UART(USART_TypeDef *, CUsart*);
	static bool cancel_UART(USART_TypeDef *);

private:
	USART_TypeDef * USARTx_;
	IOGroup_Type IOGroup_;
	DMA_Stream_TypeDef * DmaTxStream_;
	DMA_Stream_TypeDef * DmaRxStream_;
	uint32_t baudRate_;
	uint16_t parity_;
	uint8_t rxOverflowCount_;
	uint8_t *rxBuff_;
	uint16_t rxBuffSize_;

	/* interrupt flag clear register */
	uint32_t* const TXDMA_IFCR_;
	uint32_t* const RXDMA_IFCR_;
	uint32_t* const RXDMA_ISR_;
	const uint32_t TXDMA_IFCR_TCIF_MASK;
	const uint32_t RXDMA_IFCR_TCIF_MASK;
	
};

#endif
//end of file
