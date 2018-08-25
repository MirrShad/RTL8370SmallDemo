#ifndef _PULSER_H_
#define _PULSER_H_
#include "stm32f4xx.h"
class CPulser
{
	public:
		enum PulserEnum
		{
			EPulserE14
		};
		
		CPulser(PulserEnum pulser);
		void InitPulserGpio();
		void InitPulser();
		
		PulserEnum pulser() {return pulserx_;}
		void setFreq(uint16_t freq_hz){freq_hz_ = freq_hz;}
		void enablePulser(uint16_t pulseNum);
		
		TIM_TypeDef* masterTIM() {return bspMasterTIM(pulserx_);}
		TIM_TypeDef* slaverTIM() {return bspSlaverTIM(pulserx_);}
		
		static int bspIO(
				PulserEnum Pulserx,
				uint32_t* pRCC_AHB1Periph_GPIOx, 
				uint8_t* pGPIO_PinSourcex,
				uint8_t* pGPIO_AF_TIMx,
				GPIO_TypeDef ** pGPIOx);
		
		static TIM_TypeDef* bspMasterTIM(PulserEnum pulser);
		static uint8_t bspTimChn(PulserEnum pulser);
		static TIM_TypeDef* bspSlaverTIM(PulserEnum pulser);
		static uint16_t bspTriggerSource(TIM_TypeDef* mst, TIM_TypeDef* slv);
		
	private:
		PulserEnum pulserx_;
		uint32_t freq_hz_;
};
#endif
//end of file
