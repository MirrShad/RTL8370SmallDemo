#ifndef _RTL_8370_MB_H_
#define _RTL_8370_MB_H_


#include "stm32f4x7_rtl8370mb_eth.h"
#include "rtk_types.h"
#include "rtk_error.h"

#define RTL8370MB_PHY_ADDRESS  	0x00				//PHY芯片地址.
#define RTL8370MB_RST_PORT 		  GPIOD 			//复位引脚
#define RTL8370MB_RST_PIN				GPIO_Pin_3

#ifdef __cplusplus
extern "C" {
#endif
 
extern __align(4)  ETH_DMADESCTypeDef DMARxDscrTab[ETH_RXBUFNB];			//以太网DMA接收描述符数据结构体指针
extern __align(4)  ETH_DMADESCTypeDef DMATxDscrTab[ETH_TXBUFNB];			//以太网DMA发送描述符数据结构体指针 
extern __align(4)  uint8_t Rx_Buff[ETH_RX_BUF_SIZE*ETH_RXBUFNB]; 							//以太网底层驱动接收buffers指针 
extern __align(4)  uint8_t Tx_Buff[ETH_TX_BUF_SIZE*ETH_TXBUFNB]; 							//以太网底层驱动发送buffers指针
extern ETH_DMADESCTypeDef  *DMATxDescToSet;			//DMA发送描述符追踪指针
extern ETH_DMADESCTypeDef  *DMARxDescToGet; 		//DMA接收描述符追踪指针 
extern ETH_DMA_Rx_Frame_infos *DMA_RX_FRAME_infos;	//DMA最后接收到的帧信息指针	
	
u8 RTL8370MB_SWITCH_Init(void);
u8 RTL8370MB_PHY_Init(void);
u8 RTL8370MB_SWITCH_CheatInit(void);
u8 RTL8370MB_Get_Speed(void);
u8 ETH_MACDMA_Config(void);
FrameTypeDef ETH_Rx_Packet(void);
u8 ETH_Tx_Packet(u16 FrameLength);
u32 ETH_GetCurrentTxBuffer(void);
u8 ETH_Mem_Malloc(void);
void ETH_Mem_Free(void);	
	
#ifdef __cplusplus
}
#endif


#endif // _RTL_8370_MB_H_
