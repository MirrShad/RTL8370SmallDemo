#ifndef __SIM_SMI_H__
#define __SIM_SMI_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx.h"	
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"	
	
#define SIM_SMI_MDC_PORT	GPIOC
#define SIM_SMI_MDC_PIN		GPIO_Pin_1
#define SIM_SMI_MDIO_PORT	GPIOA
#define SIM_SMI_MDIO_PIN	GPIO_Pin_2
	
void sim_smi_setDirection(int pin_num,int dir);
void sim_smi_setData(int pin_num, int data);
void sim_smi_getData(int pin_num, int* pData);

	
#ifdef __cplusplus
}
#endif


	
#endif 
