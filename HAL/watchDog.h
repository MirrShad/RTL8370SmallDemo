#ifndef _FW_WATCHDOG_H_
#define _FW_WATCHDOG_H_
#include "stm32f4xx.h"

void InitWatchDog(uint8_t);
inline void ServiceDog()
{IWDG_ReloadCounter();}

#endif
//end of file
