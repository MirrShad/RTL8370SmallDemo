#include "watchDog.h"
void InitWatchDog(uint16_t time_ms)
{
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
  IWDG_SetPrescaler(IWDG_Prescaler_32);
  IWDG_SetReload(time_ms);
  IWDG_Enable();
	IWDG_ReloadCounter();
}
//end of file
