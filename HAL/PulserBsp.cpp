#include "Pulser.h"
#include "Console.h"
#include "stm32f4xx.h"

int CPulser::bspIO(PulserEnum Pulserx,
				uint32_t* pRCC_AHB1Periph_GPIOx, 
				uint8_t* pGPIO_PinSourcex,
				uint8_t* pGPIO_AF_TIMx,
				GPIO_TypeDef ** pGPIOx)
{
		if(CPulser::EPulserE14 == Pulserx)
		{
			*pGPIO_AF_TIMx = GPIO_AF_TIM1;
			*pRCC_AHB1Periph_GPIOx = RCC_AHB1Periph_GPIOE;
			*pGPIO_PinSourcex = GPIO_PinSource14;
			*pGPIOx = GPIOE;
			return 0;
		}
		else 
			return -1;
}

TIM_TypeDef* CPulser::bspMasterTIM(PulserEnum pulser)
{
	if(CPulser::EPulserE14 == pulser)
		return TIM1;
	else
		Console::Instance()->printf("Unknow pulser: %d\r\n", pulser);
	return TIM1;
}
TIM_TypeDef* CPulser::bspSlaverTIM(PulserEnum pulser)
{
	if(CPulser::EPulserE14 == pulser)
		return TIM3;
	else
		Console::Instance()->printf("Unknow pulser: %d\r\n", pulser);
	return TIM3;
}

uint8_t CPulser::bspTimChn(PulserEnum pulser)
{
	if(CPulser::EPulserE14 == pulser)
		return 4;
	else
		Console::Instance()->printf("Unknow pulser: %d\r\n", pulser);
	return 0;
}

uint16_t CPulser::bspTriggerSource(TIM_TypeDef* mst, TIM_TypeDef* slv)
{
	if(TIM1 == mst && TIM3 == slv)
		return TIM_TS_ITR0;
	else
		Console::Instance()->printf("Don't support this trigger source\r\n");
	return 0;
}

//end of file
