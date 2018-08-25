/********************************************************************************
* @file    CCanConsole.cpp
* @author  Chenxx
* @version V1.0
* @date    2016-04-21
* @brief   this file defines the Console function that can printf with can.
*	   This is for STM32F4.
********************************************************************************/
#include "CCanConsole.h"

/**
  * @brief  Constructor
	* @param  None
  * @retval None
  */
CCanConsole::CCanConsole()
	:
#if STM32_CONSOLE_CAN == 1
	canBaseRouter_(CanRouter1),
#elif STM32_CONSOLE_CAN == 2
	canBaseRouter_(CanRouter2),
#else 
#	error
#endif
	overflowCounter_(0)
{
	
}

/**
  * @brief  run UART transmitter, in another word TXD
  * @param  None
  * @retval None
  */
void CCanConsole::runTransmitter()
{
	canBaseRouter_.runTransmitter();
}

/**
  * @brief  get free size
  * @param  None
  * @retval None
  */
uint16_t CCanConsole::getFreeSize()
{
	uint16_t freesize = canBaseRouter_.getTxQueFreeSize();
	
	// no node on CAN bus, to avoid Console blocking, act as CanConsole normal
	// actually this device is gave up during nothing ack.
	if(0 == freesize && 
			(CAN_ErrorCode_ACKErr == CAN_GetLastErrorCode(CONSOLE_CAN) || 
			CAN_ErrorCode_BitRecessiveErr == CAN_GetLastErrorCode(CONSOLE_CAN)))
		return 64;
	
	if(freesize >= 4)
		return 32;
	
	else
		return freesize*8;
}

/**
  * @brief  write data to UartConsole device
  * @param  srcBuf
	* @param  len
  * @retval bytes that actually wrote into it
  */
uint16_t CCanConsole::write(uint8_t* srcBuf, uint16_t srcLen)
{
	CanTxMsg tempMsg;
	tempMsg.IDE = CAN_Id_Extended;
	tempMsg.ExtId = 0x14999;
	tempMsg.DLC = 8;
	tempMsg.RTR = CAN_RTR_Data;
	
	uint16_t writeLen = getFreeSize();
	if(srcLen < writeLen)
		writeLen = srcLen;
	
	uint8_t* endPtr = srcBuf + writeLen; //the byte at endptr should not be used
	uint8_t* frontPtr = srcBuf;
	
	while(frontPtr + 8 < endPtr)
	{
		memcpy(tempMsg.Data, frontPtr, 8);
		frontPtr += 8;
		canBaseRouter_.putMsg(tempMsg);
	}
	tempMsg.DLC = endPtr - frontPtr;
	memcpy(tempMsg.Data, frontPtr, tempMsg.DLC);
	canBaseRouter_.putMsg(tempMsg);
	
	return writeLen;
}

/**
  * @brief  Initialize the Usart of console
  * @param  None
  * @retval None
  */
bool CCanConsole::open()
{
	if(canBaseRouter_.isInitialized())
		return false;
	
	canBaseRouter_.InitCan();
	canBaseRouter_.InitCanGpio(CONSOLE_CAN_IOGROUP);
	return true;
}

/**
* @brief  Is Transmitter idel or not
* @param  None
* @retval If is idel
*/
bool CCanConsole::isIdle()
{
	canBaseRouter_.runTransmitter();
	static uint16_t noAckCounter = 0;
	if(CAN_ErrorCode_ACKErr == CAN_GetLastErrorCode(CONSOLE_CAN) 
		|| CAN_ErrorCode_BitRecessiveErr == CAN_GetLastErrorCode(CONSOLE_CAN))
	{
		//CAN bus empty, no node reply
		if(noAckCounter++ > 3000)
			return true;
	}
	else
	{
		noAckCounter = 0;
	}
	
	
	return (0 == canBaseRouter_.getMsgsInTxQue()
		&& CAN_TxStatus_Ok == CAN_TransmitStatus(CONSOLE_CAN, 0)
		&& CAN_TxStatus_Ok == CAN_TransmitStatus(CONSOLE_CAN, 1)
		&& CAN_TxStatus_Ok == CAN_TransmitStatus(CONSOLE_CAN, 2));
}

//end of file
