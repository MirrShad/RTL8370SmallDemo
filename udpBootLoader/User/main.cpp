/********************************************************************************
* @file    udpBootloader
* @author  Chenxx
* @version V1.0
* @date    2016-04-21
* @brief   
* release information: 
*	V1.0: Add bootloader update function.
* V1.1: Add flash read function. 
* V1.2: Disable interrupt before jump to bootloader.
********************************************************************************/
#include "stm32f4xx.h"
#include "Console.h"
#include "CommonConfig.h"
#include "CSpi.h"
//#include "w5500.h"
#include "socket.h"
#include "udpDevice.h"
#include "iap.h"
#include "Timer.h"
#include "stmflash.h"
#include "CCan.h"
#include "NetworkAdapter.h"
#include "network_conf.h"
#include "powerupOption.h"
#include "HeartLed.h"

#ifdef RUN_FOR_SRC_2000

#include "RTL8370MB.h"

#endif
uint8_t tx_mem_conf[8] = {8,8,8,8,8,8,8,8};	// for setting TMSR regsiter
uint8_t rx_mem_conf[8] = {8,8,8,8,8,8,8,8};   // for setting RMSR regsiter
uint8_t w5500_ip[4] = IPV4_ADDR;            // for setting SIP register
uint8_t gw[4] = IPV4_GATEWAY;              // for setting GAR register
uint8_t sn[4] = IPV4_SUBNETMASK;              // for setting SUBR register
uint8_t mac[6] = MAC_ADDR;   // for setting SHAR register
uint8_t test_buf[10] = {0,0,0,0,0,0,0,0,0,0};

extern const uint8_t FIRMWARE_VERSION = 0x14;
extern CUdpDevice udpConsole;
// <<< Use Configuration Wizard in Context Menu >>>
// <e> RUN_IN_APP
// 	<i>Default: 0
#define RUN_IN_APP 0
// <<< end of configuration section >>>
int main(void)
{ 
#if RUN_IN_APP
	SCB->VTOR = FLASH_APP1_ADDR;
#else
	SCB->VTOR = 0x08000000;
#endif
	NVIC_CONFIG();
	BaseTimer::Instance()->initialize();
	
	Initial_HeartLED();
	GPIO_SetBits(HEART_LED_GPIO, HEART_LED_PIN);
	
	if(BOOT_PARAM_BL != pvf::read(pvf::VAR_BOOT_OPTI) && BOOT_PARAM_APP != pvf::read(pvf::VAR_BOOT_OPTI) && BOOT_PARAM_LOADING != pvf::read(pvf::VAR_BOOT_OPTI))
		pvf::write(pvf::VAR_BOOT_OPTI, BOOT_PARAM_BL);
	
	#ifdef RUN_FOR_SRC_2000
	if(BOOT_PARAM_LOADING != pvf::read(pvf::VAR_BOOT_OPTI))
		RTL8370MB_SWITCH_Init();
	#endif
	NetworkAdapter::Instance()->doInit();
	
	iapUdpDevice.open();
	BaseTimer::Instance()->delay_ms(50);
	iapUdpDevice.runReceiver();
#if !RUN_IN_APP
	if(iapUdpDevice.data_in_read_buf() > 1)
	{
		iapUdpDevice.runReceiver();
		iapUdpDevice.read(test_buf, 2);
		if(test_buf[0] == 0x7F && test_buf[1] == 0x7F)
		{
			Console::Instance()->printf("Get into bootloader by emergency mode.\r\n");
//			write_boot_parameter(BOOT_PARAM_IAP);
			pvf::write(pvf::VAR_BOOT_OPTI, BOOT_PARAM_BL);
		}else if(BOOT_PARAM_APP == pvf::read(pvf::VAR_BOOT_OPTI))
		{
			Console::Instance()->printf("in BootLoader 2\r\n");
			iap_load_app(FLASH_APP1_ADDR);
		}
	}else if(BOOT_PARAM_APP == pvf::read(pvf::VAR_BOOT_OPTI))
	{
		iap_load_app(FLASH_APP1_ADDR);
		//pvf::write(pvf::VAR_BOOT_OPTI, BOOT_PARAM_BL);
	}
	
#endif

#if RUN_IN_APP
	BaseTimer::Instance()->delay_ms(1000);
	Console::Instance()->printf("\r\n Bootloader flasher start!\r\n");
#else	
	if(BOOT_PARAM_BL == pvf::read(pvf::VAR_BOOT_OPTI) || BOOT_PARAM_LOADING == pvf::read(pvf::VAR_BOOT_OPTI))
	{
		Console::Instance()->printf("\r\n Bootloader start!\r\n");
	}else
	{
		Console::Instance()->printf("\r\n Boot paramter error: 0x%X, keep at bootloader\r\n",
			pvf::read(pvf::VAR_BOOT_OPTI));
	}
#endif
	Console::Instance()->printf("Firmware virsion: V%d.%d\r\n", FIRMWARE_VERSION>>4, FIRMWARE_VERSION&0xF);
	InitWatchDog(2000);
	while(1)
	{
		iapUdpDevice.run();
		iap_run();
		Console::Instance()->runTransmitter();
		HeartLed_Run();
		ServiceDog();
	}
}

#if  1

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
			Console::Instance()->printf("Wrong parameters value: file %s on line %d\r\n", file, line);
		}
  }
}
#endif

namespace
{
	const uint8_t TX_QUEBUF_SIZE = 64;
	CanTxMsg router1_txQueBuf[TX_QUEBUF_SIZE];
	CanTxMsg router2_txQueBuf[TX_QUEBUF_SIZE];
};
CCanRouter CanRouter1(CAN1, router1_txQueBuf, TX_QUEBUF_SIZE, 250000);
CCanRouter CanRouter2(CAN2, router2_txQueBuf, TX_QUEBUF_SIZE, 250000);


