#include "stm32f4xx.h"
#include "CommonConfig.h"
#include <stdint.h>
#include "TaskManager.h"
#include "Timer.h"
#include "NetworkAdapter.h"
#include "MessageTask.h"
#include "GlobalData.h"
#include "DIO.h"
#include "TemperatureHumidityTask.h"
#include "UISampling.h"
#include "stmflash.h"

#include "F4Kxx_BSPmacro.h"
#include "powerupOption.h"
#include "CommandDispatchTask.h"
#include "sntpSynchr.h"

////////////////////////********** TODO TO BE DELETED!!! **********////////////////////////
#include "AbsoluteEncoderSickAHx36.h"

////////////////////////********** TODO TO BE DELETED!!! **********////////////////////////

int main(void)
{
	SCB->VTOR = FLASH_APP1_ADDR;
	NVIC_CONFIG();
	
	BaseTimer::Instance()->initialize();

	NetworkAdapter::Instance()->doInit();
	
	pvf::initInApp();
	
	SntpSynchr::Instance()->doInit();
	TaskManager::Instance()->addTask(NAMECODE_ConfigTask);
	TaskManager::Instance()->addTask(NAMECODE_CommandDispatchTask);
	TaskManager::Instance()->addTask(NAMECODE_MessageTask);
	TaskManager::Instance()->addTask(NAMECODE_CanRouterTask);
	Message::Instance()->postMsg("Reboot Now");
	//UISampling::Instance()->doInit();
	
	//dio init process
	DIO::Instance()->doInit();
	TaskManager::Instance()->addTask(NAMECODE_DIOTask);
	GlobalData::RptDat.ioDat.setEnable(true);
	
	#ifdef RUN_FOR_INT_BOARD
		TaskManager::Instance()->addTask(NAMECODE_IntBrdTask);
	#endif

	#ifdef RUN_FOR_SRC_2000
		TaskManager::Instance()->addTask(NAMECODE_SelfInsTask);
		TaskManager::Instance()->addTask(NAMECODE_UARTControlTask);
		//TaskManager::Instance()->addTask(NAMECODE_PowerLogicTask);
	#endif

	//to be moved to ConfigTask!!!!!!!!!!!!!!!!
	CommandDispatchMailbox::Instance()->attachToRouter(CanRouter1);
	CommandDispatchMailbox::Instance()->attachToRouter(CanRouter2);

	#ifdef RUN_FOR_SRC_2000
	#endif
//	InitWatchDog(2000);

	////////////////////////********** TODO TO BE DELETED!!! **********////////////////////////
//	EncSickAHx36::Instance()->enableAbsEncHandler((uint8_t*)"\x05\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00", 12);

	////////////////////////********** TODO TO BE DELETED!!! **********////////////////////////

	while(1)
	{
		TaskManager::Instance()->runTasks();
		ServiceDog();
		BaseTimer::Instance()->doRun();
	}
}

#if 1

/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
	Timer loopTimer(1000,1000);

  /* Infinite loop */
  while (1)
  {
		Console::Instance()->runTransmitter();
		if(loopTimer.isAbsoluteTimeUp())
		{
			Message::Instance()->postMsg("Wrong parameters value: file %s on line %d", file, line);
		}
  }
}
#endif

