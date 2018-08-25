#include "CCan.h"
#include "F4Kxx_BSPmacro.h"
#include "stm32f4xx_can.h"

static void empty_putMsgEntryHook(CCanRouter*, CanTxMsg& msgRef);
/**
  * @brief  Constructor
	* @param  None
  * @retval None
  */
	
/*********************************************************
****      this part is for CCanRxMailbox_base        *****
*********************************************************/
CCanRxMailbox_base::CCanRxMailbox_base(ringque_base<MsgData>& rxQue_ref)
	:rxQue_(rxQue_ref),
	rxOverflowCount_(0),
	isAttached_(0)
{
	
}

/**
  * @brief  set mailbox id as std
	* @param  standard id value
  * @retval None
  */
void CCanRxMailbox_base::setStdId(uint16_t stdId)
{
	stdId_ = stdId;
	IDE_ = CAN_Id_Standard;
}

/**
  * @brief  set mailbox id as ext
	* @param  extended id value
  * @retval None
  */
void CCanRxMailbox_base::setExtId(uint32_t extId)
{
	extId_ = extId;
	IDE_ = CAN_Id_Extended;
}

/**
  * @brief  judge if mailbox id is equal to message
	* @param  const reference of message
  * @retval ture of false
  */
bool CCanRxMailbox_base::operator == (const CanRxMsg& rhs_msg)
{
	if(rhs_msg.IDE != IDE_) return false;
	if(((IDE_ == CAN_Id_Standard) && (stdId_ == rhs_msg.StdId))
		||((IDE_ == CAN_Id_Extended) && (extId_ == rhs_msg.ExtId)))
		return true;
	else
		return false;
}

/**
  * @brief  judge if mailbox id is equal to message
	* @param  const reference of message
  * @retval ture of false
  */
bool CCanRxMailbox_base::isIdEqual(const CanRxMsg& rhs_msg)
{
	if(rhs_msg.IDE != IDE_) return false;
	if(((IDE_ == CAN_Id_Standard) && (stdId_ == rhs_msg.StdId))
		||((IDE_ == CAN_Id_Extended) && (extId_ == rhs_msg.ExtId)))
		return true;
	else
		return false;
}

///**
//  * @brief  judge if mailbox id is equal to message
//	* @param  const reference of message
//  * @retval ture of false
//  */
//bool CCanRxMailbox_base::isIdEqual(CCanRxMailbox_base* pMailbox)
//{
//	if(pMailbox->IDE_ != IDE_) return false;
//	if(((IDE_ == CAN_Id_Standard) && (stdId_ == pMailbox->stdId_))
//		||((IDE_ == CAN_Id_Extended) && (extId_ == pMailbox->extId_)))
//		return true;
//	else
//		return false;
//}
 
/**
  * @brief  return how many messages in rx queue
	* @param  None
  * @retval quantity of messages
  */
uint16_t CCanRxMailbox_base::msgsInQue() const
{
	return rxQue_.elemsInQue();
}

/**
  * @brief  get a message from rx queue
	* @Note 	Only copy for one time. We recommand to use this.
	* @param  Pointer for received message
  * @retval None
  */
void CCanRxMailbox_base::getMsg(CanRxMsg* pCanRxMsg)
{
	if(rxQue_.elemsInQue() == 0) return ;
	
	pCanRxMsg->RTR = CAN_RTR_Data;
	pCanRxMsg->IDE = IDE_;
	pCanRxMsg->ExtId = extId_;
	pCanRxMsg->StdId = stdId_;
	
	memcpy(&pCanRxMsg->DLC, &rxQue_.front().DLC, sizeof(MsgData));
	rxQue_.pop();
}

/**
  * @brief  get message data and DLC from rx queue
	* @param  Pointer for received message data structre
  * @retval None
  */
void CCanRxMailbox_base::getMsgData(MsgData* pMsgData)
{
	if(rxQue_.elemsInQue() == 0) return ;
	*pMsgData = rxQue_.front();
	rxQue_.pop();
}

/**
  * @brief  push back a message into queue
	* @param  Pointer for received message
  * @retval None
  */
void CCanRxMailbox_base::pushMsg(const CanRxMsg& msg)
{
	//copy DLC and Data[0..7]
	if(!rxQue_.push(*((MsgData*)&msg.DLC))) 
		rxOverflowCount_++;
}

/**
  * @brief  attach the mailbox to a CanRouter
  * @param  reference of target router
  * @retval if attach succeeded
  */
bool CCanRxMailbox_base::attachToRouter(CCanRouter& refRouter)
{
	bool ret = refRouter.attachMailbox(this);
	if(ret) isAttached_++;
	return ret;
}

/**
  * @brief  detach the mailbox from CanRouter
	* @param  None
  * @retval if detach succeeded
  */
bool CCanRxMailbox_base::detachFromRouter(CCanRouter& refRouter)
{
	bool ret = refRouter.detachMailbox(this);
	if(ret) isAttached_--;
	return ret;
}
/*********************************************************
****     this part is for CCanRxMailbox_static       *****
*********************************************************/
/**
  * @brief  constructor
	* @param  
  * @retval None
  */
CCanRxMailbox_static::CCanRxMailbox_static(CCanRxMailbox_base::MsgData* msgArray, uint16_t rxQueSize)
	:CCanRxMailbox_base(rxQue_), rxQue_(msgArray, rxQueSize)
{
	
}

/*********************************************************
*********     this part is for CCanRouter       **********
*********************************************************/
/**
  * @brief  constructor
	* @param  
  * @retval None
  */
CCanRouter::CCanRouter(CAN_TypeDef* CANx, 
	CanTxMsg* txQueBuf, 
	uint16_t txQueSize,
	uint32_t BaudRate)
:CANx_(CANx),
	baudRate_(BaudRate),
	txOverflowCount_(0),
	isGpioInitialized_(false),
	isCanInitialized_(false),
	pPutMsgEntryHook_(empty_putMsgEntryHook),
	txQue_(txQueBuf, txQueSize),
	prevTotalErrCount_(0),
	lastErrCode_(0x00),
	errCheckTimer_(this, &CCanRouter::errCheckCallback, 50)
{
	if(CANx_ == CAN1) 
		CAN_Filter_FIFO_  = CAN_Filter_FIFO0;
	else if(CANx_ == CAN2) 
		CAN_Filter_FIFO_  = CAN_Filter_FIFO1;
	else
	{
		Console::Instance()->postErr("CANx_ fault.");
		seer_assert(false);
	}
	
}

/**
  * @brief  Initialize CAN periperial
	* @param  
  * @retval None
  */
void CCanRouter::InitCan()
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);//CAN1 must be open when CAN2 used
	if(CANx_ == CAN2) 
	{
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN2, ENABLE);
	}else if(CANx_ == CAN1)
	{}
	
	//CAN
	CAN_InitTypeDef CAN_InitStructure;
	CAN_InitStructure.CAN_TTCM = DISABLE;	
	CAN_InitStructure.CAN_ABOM = ENABLE;	
	CAN_InitStructure.CAN_AWUM = DISABLE;
	CAN_InitStructure.CAN_NART = DISABLE;
	CAN_InitStructure.CAN_RFLM = DISABLE;
	CAN_InitStructure.CAN_TXFP = ENABLE;
	CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;	

	CAN_InitStructure.CAN_SJW = CAN_SJW_3tq;
	CAN_InitStructure.CAN_BS1 = CAN_BS1_8tq;
	CAN_InitStructure.CAN_BS2 = CAN_BS2_3tq;		
	
	if(baudRate_ == 250000)
	{
		CAN_InitStructure.CAN_Prescaler = 13;
	}
	else if(baudRate_ == 500000)
	{
		Console::Instance()->postErr("500K baudrate not configured");
		seer_assert(false);

	}else if(baudRate_ == 1000000)
	{
		CAN_InitStructure.CAN_SJW = CAN_SJW_3tq;
		CAN_InitStructure.CAN_BS1 = CAN_BS1_9tq;
		CAN_InitStructure.CAN_BS2 = CAN_BS2_3tq;
		CAN_InitStructure.CAN_Prescaler = 3;
	}else
	{
		Console::Instance()->printf("Unknow CAN baudrate %d\r\n", baudRate_);
	}

	CAN_Init(CANx_, &CAN_InitStructure);
	
	//config the filter
	CAN_FilterInitTypeDef  CAN_FilterInitStructure;
	
	CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask; //mask mode
	CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;//32bits
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000;
	CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0x0000;
	CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
	
	if(CAN1 == CANx_)
	{
		CAN_FilterInitStructure.CAN_FilterNumber = 0;	  //must < 14
	}
	else if(CAN2 == CANx_)
	{
		CAN_FilterInitStructure.CAN_FilterNumber = 15;	  //must >= 14
	}
	CAN_FilterInitStructure.CAN_FilterIdHigh = (0x00<<5);								
	CAN_FilterInitStructure.CAN_FilterIdLow = CAN_RTR_DATA|CAN_ID_STD;
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_Filter_FIFO_;
	CAN_FilterInit(&CAN_FilterInitStructure);
	for(int i = 0; i < 3; i++)
		CAN_FIFORelease(CANx_, CAN_Filter_FIFO_);
	isCanInitialized_ = true;
	
	errCheckTimer_.enable();
	
	
	
	uint32_t RCC_AHB1Periphx;
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHB1Periphx	= RCC_AHB1Periph_GPIOF;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8; 
	GPIO_Init(GPIOF, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; 
	GPIO_Init(GPIOF, &GPIO_InitStructure);
	
	GPIO_SetBits(GPIOF,GPIO_Pin_8);
	GPIO_SetBits(GPIOF,GPIO_Pin_9);
}

/**
  * @brief  Initialize CAN IO port
	* @param  
  * @retval None
  */
void CCanRouter::InitCanGpio(int IOGroup)
{
	GPIO_InitTypeDef GPIO_InitStructure; 
	uint32_t RCC_AHB1Periph_GPIOx;
	uint8_t GPIO_PinSource_BASE;
	uint8_t GPIO_AF_CANx;
	GPIO_TypeDef *GPIOx;
	
	if(IOGroup == GROUP_B12)
	{
		RCC_AHB1Periph_GPIOx = RCC_AHB1Periph_GPIOB;
		GPIOx = GPIOB;
		GPIO_PinSource_BASE = GPIO_PinSource12;
	}	
	else if(IOGroup == GROUP_A11)
	{
		RCC_AHB1Periph_GPIOx = RCC_AHB1Periph_GPIOA;
		GPIOx = GPIOA;
		GPIO_PinSource_BASE = GPIO_PinSource11;
	}
	else seer_assert(false); //undefined!
	
	if(CANx_ == CAN1)	GPIO_AF_CANx = GPIO_AF_CAN1;
	else if(CANx_ == CAN2)	GPIO_AF_CANx = GPIO_AF_CAN2;
	else seer_assert(false); //undefined!
	
	/* open clock of MOSI MISO SCK nCS */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOx, ENABLE);
	
	/* Config Pin: MOSI MISO SCK */
	GPIO_PinAFConfig(GPIOx, GPIO_PinSource_BASE, GPIO_AF_CANx);
	GPIO_PinAFConfig(GPIOx, GPIO_PinSource_BASE + 1, GPIO_AF_CANx);	
	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Pin = (GPIO_Pin_0|GPIO_Pin_1) << GPIO_PinSource_BASE;
	GPIO_Init(GPIOx, &GPIO_InitStructure);
	
	isGpioInitialized_ = true;
}

/**
  * @brief  set baudrate of CAN
	* @param  baudrate value
  * @retval None
  */
void CCanRouter::setBaudrate(uint32_t value)
{
	baudRate_ = value;
	InitCan();
}

/**
  * @brief  run CAN transmitter for one time.
	*					Send until txMailbox full or txQue_ is empty
	* @param  None
  * @retval None
  */
void CCanRouter::runTransmitter()
{
	if(txQue_.elemsInQue() == 0) return;
	
	uint8_t temp_mbox = 0xFF;
	temp_mbox = CAN_Transmit(CANx_, &txQue_.front());
	while(CAN_TxStatus_NoMailBox != temp_mbox)
	{
		txQue_.pop();
		if(txQue_.elemsInQue() == 0) break;
		temp_mbox = CAN_Transmit(CANx_, &txQue_.front());
	}
	
	if(0 != CAN_GetLastErrorCode(CANx_))
		lastErrCode_ = CAN_GetLastErrorCode(CANx_);
}

/**
  * @brief  Run CAN receiver for one time. Until FIFO is empty
	* @param  None
  * @retval None
  */
void CCanRouter::runReceiver()
{
	if(CAN_MessagePending(CANx_, CAN_Filter_FIFO_) == 0) return;
	CanRxMsg RxMessage;
	while(CAN_MessagePending(CANx_, CAN_Filter_FIFO_) != 0)
	{
		CAN_Receive(CANx_, CAN_Filter_FIFO_, &RxMessage);
		for(int i = 0; i < mailboxTab_.size(); i++)
		{
			if(mailboxTab_.at(i)->isIdEqual(RxMessage))
			{
				mailboxTab_.at(i)->pushMsg(RxMessage);
			}
		}
		//if program goes here, means the router can't find the mailbox for this id.
#if 0	//no mailbox message report
		Console::Instance()->printf("CAN%d cannot find the rx mailbox ", CANx_ == CAN1 ? 1 : 2);
		if(RxMessage.IDE == CAN_Id_Standard)
		{
			Console::Instance()->printf("STD 0x%X\r\n", RxMessage.StdId);
		}else
		{
			Console::Instance()->printf("EXT 0x%X\r\n", RxMessage.ExtId);
		}
#endif
		
	}
	
	if(0 != CAN_GetLastErrorCode(CANx_))
		lastErrCode_ = CAN_GetLastErrorCode(CANx_);
}

/**
  * @brief  push message to txQue_
	* @param  None
  * @retval None
  */
void CCanRouter::putMsg(CanTxMsg& refMsg)
{
//	Console::Instance()->printf("Put message\r\n");
	(*pPutMsgEntryHook_)(this, refMsg);
	if(!txQue_.push(refMsg))
		txOverflowCount_++;
}

/**
  * @brief  attach mailbox into mailbox table
	* @param  None
  * @retval None
  */
bool CCanRouter::attachMailbox(CCanRxMailbox_base* pMailbox)
{
	if(mailboxTab_.find(pMailbox) != mailboxTab_.end())
	{
//		Console::Instance()->printf("Mailbox already attached\r\n");
		return true;
	}
	
	if(false == mailboxTab_.push_back(pMailbox))
	{
		Console::Instance()->postErr("Mailbox table overflow");
		return false;
	}
	
	Console::Instance()->printf("CAN%d attaching mailbox: %s 0x%X finished.\r\n", 
		CANx_ == CAN1 ? 1 : 2, 
		pMailbox->IDE() == CAN_Id_Standard ? "STD" : "EXT", 
		pMailbox->IDE() == CAN_Id_Standard ? pMailbox->stdId() : pMailbox->extId());
	
	return true;
}

bool CCanRouter::detachMailbox(CCanRxMailbox_base* pMailbox)
{
	auto mailboxIter = mailboxTab_.find(pMailbox);
	bool findmailbox = (mailboxIter != mailboxTab_.end());

	if(findmailbox)
	{
		mailboxTab_.erase(mailboxIter);
	}
	
	Console::Instance()->printf("CAN%d detaching mailbox: %s 0x%X %s.\r\n", 
	CANx_ == CAN1 ? 1 : 2, 
	pMailbox->IDE() == CAN_Id_Standard ? "STD" : "EXT", 
	pMailbox->IDE() == CAN_Id_Standard ? pMailbox->stdId() : pMailbox->extId(),
	findmailbox ? "finished" : "failed");
	
	return findmailbox;
	
}

bool CCanRouter::setPutMsgHook(void (*pHook)(CCanRouter*, CanTxMsg&))
{
	if(pHook != NULL)
	{
		pPutMsgEntryHook_ = pHook;
		return true;
	}else
		return false;
}

/**
  * @brief  empty entry hook
	* @param  CanTxMsg&
  * @retval None
  */
static void empty_putMsgEntryHook(CCanRouter* pRouter, CanTxMsg& refmsg)
{
//	Console::Instance()->printf("static putmsg.\r\n");
}

CCanRouter& CanRouter(uint8_t idx)
{
	if(1 == idx)
		return CanRouter1;
	else if(2 == idx)
		return CanRouter2;
	else
	{
		Console::Instance()->printf("CanRouter idx error\r\n");
		return CanRouter1;
	}
}

/**
  * @brief  Error check timer callback
	* @param  None
  * @retval None
  */
void CCanRouter::errCheckCallback()
{
	uint16_t curTotalErrCount = CAN_GetReceiveErrorCounter(CANx_) + CAN_GetLSBTransmitErrorCounter(CANx_);
	if(curTotalErrCount != prevTotalErrCount_ && curTotalErrCount != 0)
	{
		Console::Instance()->printf("CAN%d new error: 0x%02X, count: %d\r\n", CANx_ == CAN1 ? 1 : 2, lastErrCode_, curTotalErrCount);
		switch(lastErrCode_)
		{
			case CAN_ErrorCode_StuffErr:
				Console::Instance()->printf("CAN:Stuff error");
				break;
			case CAN_ErrorCode_FormErr:
				Console::Instance()->printf("CAN:Form error");
				break;
			case CAN_ErrorCode_ACKErr:
				Console::Instance()->printf("CAN:Acknowledgement error, please check connecting the right CAN router first");
				break;
			case CAN_ErrorCode_BitRecessiveErr:
				Console::Instance()->printf("CAN:Bit Recessive error,sending recessive bit but heard dominant");
				break;
			case CAN_ErrorCode_BitDominantErr:
				Console::Instance()->printf("CAN:Bit Dominant error,sending dominant bit but heard recessive");
				break;
			case CAN_ErrorCode_CRCErr:
				Console::Instance()->printf("CAN:CRC error");
			default:
				break;
		}
	}
	prevTotalErrCount_ = curTotalErrCount;
}
ringque<CCanRxMailbox_base::MsgData, 1> CCanRxMailbox_base::fakeRxQue_; 

//end of file
