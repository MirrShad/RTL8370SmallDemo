#ifndef _FW_TIMER_H_
#define _FW_TIMER_H_
#include "stm32f4xx.h"
#include <stdint.h>
#include "Singleton.h"
//base timer

class CTimer_base;

class CBaseTimer
{
public:
	CBaseTimer();// need EALLOW
	~CBaseTimer(){}

	void initialize();
	void doUpdate();
	void doRun();
	int attach(CTimer_base*);
	int detach(CTimer_base*);
	int32_t getTime(){return _baseTimer;}
	inline uint32_t systickPeriod() { return SYSTICK_PERIOD_; }
	
	inline uint64_t getTime_ns()
	{
		uint32_t baseTime = _baseTimer;
		uint32_t tick = SysTick->VAL;
		uint32_t baseTime1 = _baseTimer;
		if(baseTime1 != baseTime && tick > (SYSTICK_PERIOD_ >> 1))
			++baseTime;
		
		return (baseTime * 1000000ull + (SYSTICK_PERIOD_ - tick - 1) * 1000000ull / SYSTICK_PERIOD_);
	}
	
	inline void getTime_ns(uint64_t* p) 
	{
		uint32_t baseTime = _baseTimer;
		uint32_t tick = SysTick->VAL;
		uint32_t baseTime1 = _baseTimer;
		if(baseTime1 != baseTime && tick > (SYSTICK_PERIOD_ >> 1))
			++baseTime;
		*p =  (baseTime * 1000000ull + (SYSTICK_PERIOD_ - tick - 1) * 1000000ull / SYSTICK_PERIOD_);
	}
	
	inline void start()
	{
		SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
	}
	inline void stop()
	{
		SysTick->CTRL &= ~(uint16_t)SysTick_CTRL_ENABLE_Msk;
	}
	
	inline uint32_t tickVal() {return (SYSTICK_PERIOD_ - SysTick->VAL - 1);}
	void delay_ms(uint16_t value);
	uint32_t tick_per_microsec() {return SYSTICK_PERIOD_;}
	
private:
	volatile int32_t _baseTimer;
	uint64_t TICK_VAL_NS_FAC_;
	uint32_t SYSTICK_PERIOD_;					//19375 why?
	bool msFlag_;

};

typedef NormalSingleton<CBaseTimer>	BaseTimer;

class CTimer_base
{
	public:
		CTimer_base(int32_t delay = 1,int32_t period = 1)
		{
			_starttime = BaseTimer::Instance()->getTime() - period + delay;
			_period = period;
		}

		~CTimer_base()
		{
			BaseTimer::Instance()->detach(this);
		};
		
		bool isAbsoluteTimeUp()
		{
			int32_t tempBaseTimer = BaseTimer::Instance()->getTime();
			int32_t diff = tempBaseTimer - _starttime;
			if ( diff < 0 )
			{
				diff &= 0x7FFFFFFF;
			}
			if (diff >= _period)
			{
				_starttime = tempBaseTimer;
				return true;
			}
			return false;
		}
	
		void setPeriod(int32_t period) 
		{
			_period = period;
			reset();
		}
		
		void reset() { _starttime = BaseTimer::Instance()->getTime(); }
		
		bool isTimeUp() { return isAbsoluteTimeUp(); }
		
		virtual void callback() {}
	
	private:
		int32_t _starttime;
		int32_t _period;
		
};

typedef CTimer_base Timer;

template<typename T>
class CAsyncTimer: public CTimer_base
{
	public:
		CAsyncTimer(T* pObj, void(T::*pFunc)(), int32_t period):
			CTimer_base(period, period), _pObj(pObj), _pFunc(pFunc)
		{}
		virtual void callback()
		{
			if(_pObj && _pFunc)
				(_pObj->*_pFunc)();
		}
		
		int enable() 
		{ 
			reset();
			return BaseTimer::Instance()->attach(this); 
		}
	
		int disable() { return BaseTimer::Instance()->detach(this); }
	
	private:
		T* _pObj;
		void (T::*_pFunc)();
};

template<>
class CAsyncTimer<void>: public CTimer_base
{
	public:
		CAsyncTimer( void(* pFunc)(), int32_t period):
			CTimer_base(period, period), _pFunc(pFunc)
		{}
		virtual void callback()
		{
			if(_pFunc)
				(*_pFunc)();
		}
		
		int enable() 
		{ 
			reset();
			return BaseTimer::Instance()->attach(this); 
		}
	
		int disable() { return BaseTimer::Instance()->detach(this); }
	
	private:
		void (* _pFunc)();
};

void CPUTIMER0_ISR();

#endif	//_FW_TIMER_H_
