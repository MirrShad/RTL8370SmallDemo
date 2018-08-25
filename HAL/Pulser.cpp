#include "Pulser.h"
#include "Console.h"

CPulser::CPulser(PulserEnum pulser):freq_hz_(10000)
{

}
	
void CPulser::InitPulserGpio()
{
	GPIO_InitTypeDef GPIO_InitStructure; 
	uint32_t RCC_AHB1Periph_GPIOx;
	uint8_t GPIO_PinSource_;
	uint8_t GPIO_AF_TIMx;
	GPIO_TypeDef *GPIOx_;
	
	bspIO(pulserx_, &RCC_AHB1Periph_GPIOx, &GPIO_PinSource_, &GPIO_AF_TIMx, &GPIOx_);
	
	/* open clock of GPIO */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOx, ENABLE);
	
	GPIO_PinAFConfig(GPIOx_, GPIO_PinSource_, GPIO_AF_TIMx);

	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 << GPIO_PinSource_;
	GPIO_Init(GPIOx_, &GPIO_InitStructure);
}


void CPulser::InitPulser()
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	
	//
	// Initialize master timer
	//
	
	if(TIM1 == masterTIM())
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1,ENABLE);
	else if(TIM2 == masterTIM())
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);
	else if(TIM3 == masterTIM())
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);
	else if(TIM4 == masterTIM())
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);
	else if(TIM5 == masterTIM())
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5,ENABLE);
	else if(TIM8 == masterTIM())
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8,ENABLE);
	else
	{
		Console::Instance()->printf("Do not support master timer: 0x%08X\r\n", (uint32_t)masterTIM());
	}
	
	RCC_ClocksTypeDef RCC_ClocksStatus;
	RCC_GetClocksFreq(&RCC_ClocksStatus);
	uint32_t PRESCAL_FREQ = 1000000;
	
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; 
	TIM_TimeBaseInitStructure.TIM_Prescaler = RCC_ClocksStatus.SYSCLK_Frequency / PRESCAL_FREQ - 1;   //Timer clock = 1M
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInitStructure.TIM_Period = PRESCAL_FREQ / freq_hz_ - 1;    //Period = (TIM counter clock / TIM output clock) - 1 = 20K
	TIM_TimeBaseInit(masterTIM(), &TIM_TimeBaseInitStructure);
	
	TIM_OCInitTypeDef TIM_OCInitStructure;
	
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable;
	TIM_OCInitStructure.TIM_Pulse = (PRESCAL_FREQ / freq_hz_ / 2);
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_Low;
	TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;
	TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCNIdleState_Reset;
	
	switch(bspTimChn(pulserx_))
	{
		case 1:
			TIM_OC1Init(masterTIM(),&TIM_OCInitStructure);
			break;
		
		case 2:
			TIM_OC2Init(masterTIM(),&TIM_OCInitStructure);
			break;
		
		case 3:
			TIM_OC3Init(masterTIM(),&TIM_OCInitStructure);
			break;
		
		case 4:
			TIM_OC4Init(masterTIM(),&TIM_OCInitStructure);
			break;
		
		default:
			Console::Instance()->printf("Unknow channel\r\n");
			break;
	}
	
	TIM_SelectOutputTrigger(masterTIM(), TIM_TRGOSource_Update);
	TIM_SelectMasterSlaveMode(masterTIM(), TIM_MasterSlaveMode_Enable);
	TIM_Cmd(masterTIM(), DISABLE);
	
	//
	// Initialize slaver timer
	//
	NVIC_InitTypeDef NVIC_InitStructure;
	
	if(TIM2 == slaverTIM())
	{
		NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);
	}
	else if(TIM3 == slaverTIM())
	{
		NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);
	}
	else if(TIM4 == slaverTIM())
	{
		NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);
	}
	else if(TIM5 == slaverTIM())
	{
		NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5,ENABLE);
	}
	else
		Console::Instance()->printf("Unsupport slaver timer\r\n");
	
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	TIM_Cmd(slaverTIM(), DISABLE);
	TIM_ITConfig(slaverTIM(), TIM_IT_Update, DISABLE);
	
//	TIM_SelectInputTrigger(slaverTIM(), bspTriggerSource(masterTIM(), slaverTIM()));

	
	TIM_TimeBaseInitStructure.TIM_Period = 10000;
	TIM_TimeBaseInitStructure.TIM_Prescaler = 15500;
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(slaverTIM(), &TIM_TimeBaseInitStructure);
	
	TIM_SelectSlaveMode(slaverTIM(), TIM_SlaveMode_Trigger);  
	TIM_SelectInputTrigger(slaverTIM(), TIM_TS_ITR0);
	
	TIM_ClearFlag(slaverTIM(),TIM_FLAG_Update);	
	TIM_ITConfig(slaverTIM(), TIM_IT_Update, ENABLE);
	TIM_Cmd(slaverTIM(), ENABLE);
}

void CPulser::enablePulser(uint16_t pulseNum)
{
//	TIM_Cmd(masterTIM(),ENABLE);
	TIM_CtrlPWMOutputs(masterTIM(),ENABLE);
}

extern "C"
{
	
void TIM3_IRQHandler()
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
	{
		TIM_Cmd(TIM1, DISABLE);
		Console::Instance()->printf("interrupt!!\r\n");
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
	}
}	

}



