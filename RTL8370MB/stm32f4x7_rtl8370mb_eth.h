#ifndef _STM32F4X7_RTL8270MB_ETH_H_
#define _STM32F4X7_RTL8270MB_ETH_H_

#ifdef __cplusplus
extern "C" {
#endif
 

#include "stm32f4x7_eth_conf.h"
#include "stm32f4x7_eth.h"
#include "stm32f4xx_rcc.h"
#include "rtk_types.h"
#include "rtk_error.h"
#include "port.h"

	
uint32_t ETH_RTL8370MB_Init(ETH_InitTypeDef* ETH_InitStruct, uint16_t PHYAddress);
	
#ifdef __cplusplus
}
#endif	
	
#endif // _STM32F4X7_RTL8270MB_ETH_H_
