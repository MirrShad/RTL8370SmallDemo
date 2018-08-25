#ifndef __COMMONCONFIG_H
#define __COMMONCONFIG_H

#ifdef __cplusplus
 extern "C" {
#endif 

#include "stm32f4xx.h"

void NVIC_CONFIG(void);
void InitWatchDog(uint16_t);
	 
inline void ServiceDog()
{IWDG_ReloadCounter();}

#ifdef __cplusplus
 }
#endif 
#endif
