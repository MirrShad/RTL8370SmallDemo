#include "Timer.h"
#include "fixed_vector.h"
#include "Console.h"

namespace
{
	const uint8_t MAX_ASYNC_TIME_NUM = 32;
	fixed_vector<CTimer_base*, MAX_ASYNC_TIME_NUM>& asyncTimerTab()
	{
		static fixed_vector<CTimer_base*, MAX_ASYNC_TIME_NUM> asyncTimerTab_;
		return asyncTimerTab_;
	}
};

/**
  * @brief  system core clock initialize
  * @param  None
  * @retval None
  */
CBaseTimer::CBaseTimer()
:TICK_VAL_NS_FAC_(),msFlag_(false)
{	
	_baseTimer = 0;	
	SystemCoreClockUpdate();
}

/**
  * @brief  initialize Systick as base timer
  * @param  None
  * @retval None
  */
void CBaseTimer::initialize()
{	
	RCC_ClocksTypeDef RCC_ClocksStatus;
	RCC_GetClocksFreq(&RCC_ClocksStatus);
	
	TICK_VAL_NS_FAC_ = (RCC_ClocksStatus.SYSCLK_Frequency / 8000000ul);	/*SYSCLK_Frequency = 8MHz, provide by HSE_VALUE*/
	SYSTICK_PERIOD_ = RCC_ClocksStatus.SYSCLK_Frequency / 8 / 1000ul;	/*SYSCLK_Frequency = 8MHz, provide by HSE_VALUE*/
	
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8); 				
	SysTick_Config(SYSTICK_PERIOD_);
	start();
}

/**
  * @brief  when an systick interrupt occurs, call this function
  * @param  None
  * @retval None
  */
void CBaseTimer::doUpdate()
{
	++_baseTimer;
	_baseTimer &= 0x7FFFFFFF;		//prevent overflow
	msFlag_ = true;
}

void CBaseTimer::doRun()
{
	if(!msFlag_)
		return ;
	msFlag_ = false;
	
	for(int i = 0; i < asyncTimerTab().size(); i++)
	{
		if(asyncTimerTab().at(i)->isAbsoluteTimeUp())
		{
			asyncTimerTab().at(i)->callback();
		}
	}
}

int CBaseTimer::attach(CTimer_base* p)
{
	if(asyncTimerTab().end() != asyncTimerTab().find(p)) 
	{
//		Console::Instance()->printf("timer 0x%08X already attached\r\n", (uint32_t)p);
		return 0;
	}
	
	if(!asyncTimerTab().push_back(p))
	{
		Console::Instance()->printf("Attach timer 0x%08X failed, timer pool full\r\n", (uint32_t)p);
		return -1;
	}
	
	return 0;
}

int CBaseTimer::detach(CTimer_base* p)
{
	if(asyncTimerTab().erase(asyncTimerTab().find(p)))
		return 0;
	return -1;
}

void CBaseTimer::delay_ms(uint16_t value)
{
	int32_t inTime = _baseTimer;
	while(_baseTimer - inTime < value);
}

//Timer::Timer(int32_t delay,int32_t period)
//{
//	_timer = BaseTimer::Instance()->getTime() - period + delay;
//	_counter = delay;
//	_period = period;
//}

//void Timer::reset()
//{
//	_timer = BaseTimer::Instance()->getTime();
//	_counter = _period;
//}

//bool Timer::isTimeUp()
//{
//	int32_t tempBaseTimer = BaseTimer::Instance()->getTime();
//	if (_timer != tempBaseTimer)//be triggered
//	{
//		_timer = tempBaseTimer;
//		if (--_counter <= 0)
//		{
//			_counter = _period;
//			return true;
//		}
//	}
//	return false;	
//}

//bool Timer::isAbsoluteTimeUp()
//{
//	int32_t tempBaseTimer = BaseTimer::Instance()->getTime();
//	int32_t diff = tempBaseTimer - _timer;
//	if ( diff < 0 )
//	{
//		diff &= 0x7FFFFFFF;
//	}
//	if (diff >= _period)
//	{
//		_timer = tempBaseTimer;
//		return true;
//	}
//	return false;
//}

void CPUTIMER0_ISR()
{
	BaseTimer::Instance()->doUpdate();
	SCB->ICSR = SCB_ICSR_PENDSTCLR_Msk;
}
