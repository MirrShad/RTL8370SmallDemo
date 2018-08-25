#ifndef _FW_CCAN_H
#define _FW_CCAN_H
#include "stdint.h"
#include "ringque.h"
#include "stm32f4xx.h"
#include "fixed_vector.h"
#include "Timer.h"
#include "Console.h"

const uint8_t CAN_ROUTER_NUM = 2;

class CCanRouter;

class CCanRxMailbox_base
{
public:
	typedef struct
	{
			uint8_t DLC;
			uint8_t Data[8];
	}MsgData;
	CCanRxMailbox_base(ringque_base<MsgData>& rxQue_ref = (ringque_base<MsgData>&)fakeRxQue_);
	~CCanRxMailbox_base(){}
	void setStdId(uint16_t);
	void setExtId(uint32_t);
	uint16_t msgsInQue() const;
	void getMsg(CanRxMsg*);
	void getMsgData(MsgData*);
	virtual void pushMsg(const CanRxMsg&);
	void clear() {rxQue_.clear();}	
		
	bool attachToRouter(CCanRouter&);
	bool detachFromRouter(CCanRouter&);
	bool operator == (const CanRxMsg&);
	virtual bool isIdEqual(const CanRxMsg&);
	bool IDE() {return IDE_;}
	uint32_t stdId() {return stdId_;}
	uint32_t extId() {return extId_;}
	uint8_t getRxOverflowcount(){return rxOverflowCount_;}

private:
	uint32_t stdId_;
	uint32_t extId_;
	uint8_t IDE_;
	ringque_base<MsgData>& rxQue_;
	uint8_t rxOverflowCount_;
	uint8_t isAttached_;
	static ringque<CCanRxMailbox_base::MsgData, 1> fakeRxQue_;
};

/*********************************************************
****     this part is for CCanRxMailbox_fixed        *****
*********************************************************/
template<uint16_t N>
class CCanRxMailbox_fixed
	:public CCanRxMailbox_base
{
public:
	CCanRxMailbox_fixed():CCanRxMailbox_base(rxQue_)
	{}
	~CCanRxMailbox_fixed(){}

private:
	ringque<CCanRxMailbox_base::MsgData, N> rxQue_;
};

/*********************************************************
****     this part is for CCanRxMailbox_static       *****
*********************************************************/
class CCanRxMailbox_static
	:public CCanRxMailbox_base
{
public:

	CCanRxMailbox_static(CCanRxMailbox_base::MsgData*, uint16_t rxQueSize);
	~CCanRxMailbox_static(){}

private:
	ringque<CCanRxMailbox_base::MsgData> rxQue_;
};

typedef CCanRxMailbox_static CCanRxMailbox;

/*********************************************************
****     this part is for CCanPdoMailbox_base        *****
*********************************************************/
class CCanPdoMailbox_base
	:private CCanRxMailbox_base
{
	public:
		CCanPdoMailbox_base( int32_t period)
		:_watchDog(this, &CCanPdoMailbox_base::timeoutCallback, period)
		{
		}
		
		bool attachToRouter(CCanRouter& refRouter)
		{
			_watchDog.enable();
			return CCanRxMailbox_base::attachToRouter(refRouter);
		}
		
		virtual void newMsgCallback(const CanRxMsg&) = 0;
		
		virtual void timeoutCallback()
		{
			Console::Instance()->printf("PDO mailbox 0x%08X timeout!\r\n", (IDE() ? extId() : stdId()));
		}
		~CCanPdoMailbox_base();
	
	private:
		virtual void pushMsg(const CanRxMsg& canMsg)
		{
			_watchDog.reset();
			newMsgCallback(canMsg);
		}
		
		CAsyncTimer<CCanPdoMailbox_base> _watchDog;
};
/*********************************************************
*********     this part is for CCanRouter       **********
*********************************************************/
class CCanRouter
{
	friend class CCanRxMailbox_base;
public:
	
	CCanRouter(CAN_TypeDef* CANx, 
		CanTxMsg* txQueBuf, 
		uint16_t txQueSize,
		uint32_t BaudRate = 250000);
	~CCanRouter(){}
	void InitCanGpio(int IOGroup);
	void InitCan();
	void setBaudrate(uint32_t);
	bool isInitialized(){return (isGpioInitialized_&&isCanInitialized_);}
	
	uint8_t getTxOverflowcount(){return txOverflowCount_;}

	enum IOGroup_Type
	{
		GROUP_B12 = 0,
		GROUP_A11,
		GROUP_D0,
		GROUP_B5,
		GROUP_B8
	};
	
	enum{MAX_MAILBOX_NUM = 32};
	void runTransmitter();
	void runReceiver();
	void putMsg(CanTxMsg&);
	uint16_t getTxQueFreeSize() {return txQue_.emptyElemsInQue();}
	uint16_t getMsgsInTxQue() {return txQue_.elemsInQue();}
	bool setPutMsgHook(void (*pHook)(CCanRouter*, CanTxMsg&));

private:
	bool attachMailbox(CCanRxMailbox_base* pMailbox);
	bool detachMailbox(CCanRxMailbox_base* pMailbox);
	CAN_TypeDef * CANx_;
	uint32_t baudRate_;
	uint8_t CAN_Filter_FIFO_;
	uint8_t txOverflowCount_;
	bool isGpioInitialized_;
	bool isCanInitialized_;
	void (*pPutMsgEntryHook_)(CCanRouter*, CanTxMsg& );

private:
	ringque<CanTxMsg> txQue_;
	fixed_vector<CCanRxMailbox_base*, MAX_MAILBOX_NUM> mailboxTab_;
	uint8_t prevTotalErrCount_;
	uint8_t lastErrCode_;
	CAsyncTimer<CCanRouter> errCheckTimer_;
	void errCheckCallback();

private:
};

extern CCanRouter CanRouter1;
extern CCanRouter CanRouter2;
CCanRouter& CanRouter(uint8_t idx);

#endif
//end of file
