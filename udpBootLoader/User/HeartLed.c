#include "stm32f4xx.h"
#include "HeartLed.h"

void Initial_HeartLED(void)
{
	uint32_t RCC_AHB1Periphx;
	GPIO_InitTypeDef GPIO_InitStructure;
	
	/* LED config */
	RCC_AHB1Periphx	= RCC_AHB1Periph_GPIOA<< (((uint32_t)HEART_LED_GPIO-(uint32_t)GPIOA)/0x400);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periphx, ENABLE);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Pin = HEART_LED_PIN; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(HEART_LED_GPIO, &GPIO_InitStructure);
	
#ifndef RUN_FOR_SRC_2000
	RCC_AHB1Periphx	= RCC_AHB1Periph_GPIOA<< (((uint32_t)LINK_LED_GPIO-(uint32_t)GPIOA)/0x400);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periphx, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = LINK_LED_PIN; 
	GPIO_Init(LINK_LED_GPIO, &GPIO_InitStructure);
	GPIO_ResetBits(LINK_LED_GPIO, LINK_LED_PIN);
#endif
}

void HeartLed_Run()
{
	static uint32_t cnt = 0;
	static bool LedOn = true;
	if(++cnt > HEART_LED_PERIOD)
	{
		--cnt;
		cnt = 0;
		LedOn = !LedOn;
	}
	
	if(LedOn)
		GPIO_SetBits(HEART_LED_GPIO, HEART_LED_PIN);
	else
		GPIO_ResetBits(HEART_LED_GPIO, HEART_LED_PIN);
}
