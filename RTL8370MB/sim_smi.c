#include "sim_smi.h"


void sim_smi_setDirection(int pin_num,int dir)
{
	static bool bFirstIn = true;
	if(bFirstIn)
	{
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA|RCC_AHB1Periph_GPIOC, ENABLE);
		bFirstIn = false;
	}
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ; 
	if(1 == dir)
	{
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	}else if(0 == dir)
	{
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	}
	if(1 == pin_num)
	{
		GPIO_InitStructure.GPIO_Pin = SIM_SMI_MDC_PIN;
		GPIO_Init(SIM_SMI_MDC_PORT, &GPIO_InitStructure);
	}else if(2 == pin_num)
	{
		GPIO_InitStructure.GPIO_Pin = SIM_SMI_MDIO_PIN;
		GPIO_Init(SIM_SMI_MDIO_PORT, &GPIO_InitStructure);
	} 
}
void sim_smi_setData(int pin_num, int data)
{
	if(pin_num == 1)
	{
		if(1==data)
			GPIO_SetBits(SIM_SMI_MDC_PORT,SIM_SMI_MDC_PIN);
		else if(0==data)
			GPIO_ResetBits(SIM_SMI_MDC_PORT,SIM_SMI_MDC_PIN);
	}else if(pin_num == 2)
	{
		if(1==data)
			GPIO_SetBits(SIM_SMI_MDIO_PORT,SIM_SMI_MDIO_PIN);
		else if(0==data)
			GPIO_ResetBits(SIM_SMI_MDIO_PORT,SIM_SMI_MDIO_PIN);
	}
	
}
void sim_smi_getData(int pin_num, int* pData)
{
	if(pin_num == 1)
		*pData = GPIO_ReadInputDataBit(SIM_SMI_MDC_PORT,SIM_SMI_MDC_PIN);
	else if(pin_num == 2)
		*pData = GPIO_ReadInputDataBit(SIM_SMI_MDIO_PORT,SIM_SMI_MDIO_PIN);
}
