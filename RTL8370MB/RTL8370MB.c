#include "RTL8370MB.h"
#include "port.h"
#include "Timer.h"
#include "rtk_switch.h"
#include "stm32f4xx_rcc.h"
#include "interrupt.h"
#include "led.h"
#include "rtl8367c_asicdrv.h"
#include "rtk_cpu.h"

__align(4) ETH_DMADESCTypeDef DMARxDscrTab[ETH_RXBUFNB];	//以太网DMA接收描述符数据结构体指针
__align(4) ETH_DMADESCTypeDef DMATxDscrTab[ETH_TXBUFNB];	//以太网DMA发送描述符数据结构体指针 
__align(4) uint8_t Rx_Buff[ETH_RX_BUF_SIZE*ETH_RXBUFNB]; 					//以太网底层驱动接收buffers指针 
__align(4) uint8_t Tx_Buff[ETH_TX_BUF_SIZE*ETH_TXBUFNB]; 					//以太网底层驱动发送buffers指针

static void ETHERNET_NVICConfiguration(void);

u8 RTL8370MB_SWITCH_Init(void)
{
	//Boot up time for RTL8370MB
	BaseTimer::Instance()->delay_ms(1000);
	
	rtk_api_ret_t ret;
	
	
	ret = rtk_switch_init();
	if(ret != RT_ERR_OK)
		return 1;
	
	rtk_port_mac_ability_t pAbility;
	rtk_mode_ext_t pMode;
	u8 rval;
	
	rval=rtk_port_macForceLinkExt_get(EXT_PORT1,&pMode,&pAbility);	
	if(rval != RT_ERR_OK)
		return 2;
	pAbility.forcemode = 1;//MAC_FORCE
	pAbility.speed = 1;//PORT_SPEED_100M;
	pAbility.duplex = 1;//PORT_FULL_DUPLEX;
	pAbility.link = 1;//PORT_LINKUP;
	pAbility.nway = 0;//DISABLED;
	pAbility.txpause = 1;//ENABLED;
	pAbility.rxpause = 1;//ENABLED;
	rval=rtk_port_macForceLinkExt_set(EXT_PORT1,MODE_EXT_RMII_PHY,&pAbility);	
	rval=rtk_port_macForceLinkExt_get(EXT_PORT1,&pMode,&pAbility);	
	if(rval != RT_ERR_OK)
		return 3;
	rval=rtk_port_macForceLinkExt_get(EXT_PORT1,&pMode,&pAbility);	
	rval=rtk_port_macForceLinkExt_get(EXT_PORT1,&pMode,&pAbility);	
	if(rval != RT_ERR_OK)
		return 4;
	
		//LED test
	{
		/*
		rtk_portmask_t portmask;
		RTK_PORTMASK_PORT_SET(portmask,UTP_PORT0);
		RTK_PORTMASK_PORT_SET(portmask,UTP_PORT1);
		RTK_PORTMASK_PORT_SET(portmask,UTP_PORT2);
		RTK_PORTMASK_PORT_SET(portmask,UTP_PORT3);
		RTK_PORTMASK_PORT_SET(portmask,UTP_PORT4);
		RTK_PORTMASK_PORT_SET(portmask,UTP_PORT5);
		RTK_PORTMASK_PORT_SET(portmask,UTP_PORT6);
		RTK_PORTMASK_PORT_SET(portmask,UTP_PORT7);
		rtk_led_serialModePortmask_set(SERIAL_LED_0_1,&portmask);
		
		rtk_led_operation_set(LED_OP_SERIAL);
		rtk_led_serialMode_set(LED_ACTIVE_HIGH);
		rtk_led_OutputEnable_set(ENABLED);
		
		rtk_led_groupConfig_set(LED_GROUP_0,LED_CONFIG_ACT);
		rtk_led_groupConfig_set(LED_GROUP_1,LED_CONFIG_LINK_ACT);
		rtk_led_groupConfig_set(LED_GROUP_2,LED_CONFIG_LEDOFF);
		rval = rtk_led_enable_set(LED_GROUP_0,&portmask);
		rval = rtk_led_enable_set(LED_GROUP_1,&portmask);
		rval = rtk_led_enable_set(LED_GROUP_2,&portmask);
		*/
		
		/*
		rtk_portmask_t portmask;
		RTK_PORTMASK_PORT_SET(portmask,UTP_PORT0);
		RTK_PORTMASK_PORT_SET(portmask,UTP_PORT1);
		RTK_PORTMASK_PORT_SET(portmask,UTP_PORT2);
		RTK_PORTMASK_PORT_SET(portmask,UTP_PORT3);
		RTK_PORTMASK_PORT_SET(portmask,UTP_PORT4);
		RTK_PORTMASK_PORT_SET(portmask,UTP_PORT5);
		//RTK_PORTMASK_PORT_SET(portmask,UTP_PORT6);
		//RTK_PORTMASK_PORT_SET(portmask,UTP_PORT7);
		rtk_led_force_mode_t mode;
		rtk_led_operation_set(LED_OP_PARALLEL);
		//rval = rtk_led_modeForce_set(UTP_PORT2,LED_GROUP_0,LED_FORCE_ON);
		//rval = rtk_led_modeForce_set(UTP_PORT2,LED_GROUP_1,LED_FORCE_ON);
		//rval = rtk_led_modeForce_get(UTP_PORT1,LED_GROUP_0,&mode);
		//rval = rtk_led_blinkRate_set(LED_BLINKRATE_1024MS);
		//rval = rtk_led_modeForce_set(UTP_POT1,LED_GROUP_0,LED_FORCE_BLINK);
		//rval = rtk_led_modeForce_set(UTP_PORT1,LED_GROUP_1,LED_FORCE_BLINK);
		//rval = rtk_led_modeForce_set(UTP_PORT2,LED_GROUP_0,LED_FORCE_OFF);
		//rval = rtk_led_modeForce_set(UTP_PORT2,LED_GROUP_1,LED_FORCE_OFF);
		rtk_led_groupConfig_set(LED_GROUP_0,LED_CONFIG_ACT);
		rtk_led_groupConfig_set(LED_GROUP_1,LED_CONFIG_LINK_ACT);
		rval = rtk_led_enable_set(LED_GROUP_0,&portmask);
		rval = rtk_led_enable_set(LED_GROUP_1,&portmask);
		*/
	}
		
	/*
	rtk_uint32 value;
	
	rval= rtl8367c_setAsicReg(0x13c3, 0x08);
	rval= rtl8367c_setAsicReg(0x13c4, 0x1075);
	rval= rtl8367c_getAsicReg(0x13c3, &value);
	rval= rtl8367c_getAsicReg(0x13c4, &value);
	rval=rtk_port_macForceLinkExt_get(EXT_PORT0,&pMode,&pAbility);*/

	/*
	rtk_uint32 value;
	rval= rtl8367c_getAsicReg(0x1b00, &value);
	rval= rtl8367c_getAsicReg(0x1b03, &value);
	*/
	return 0;
}

u8 RTL8370MB_PHY_Init(void)
{
	u8 rval=0;
	GPIO_InitTypeDef GPIO_InitStructure;
  
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA|RCC_AHB1Periph_GPIOC|RCC_AHB1Periph_GPIOD|RCC_AHB1Periph_GPIOG , ENABLE);//使能GPIO时钟 RMII接口
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);   //使能SYSCFG时钟
  
	SYSCFG_ETH_MediaInterfaceConfig(SYSCFG_ETH_MediaInterface_RMII); //MAC和PHY之间使用RMII接口

	/*网络引脚设置 RMII接口 
	  ETH_MDIO -------------------------> PA2
	  ETH_MDC --------------------------> PC1
	  ETH_RMII_REF_CLK------------------> PA1
	  ETH_RMII_CRS_DV ------------------> PA7
	  ETH_RMII_RXD0 --------------------> PC4
	  ETH_RMII_RXD1 --------------------> PC5
	  ETH_RMII_TX_EN -------------------> PG11
	  ETH_RMII_TXD0 --------------------> PG13
	  ETH_RMII_TXD1 --------------------> PG14
	  ETH_RESET-------------------------> PD3*/
	
	  //配置PA1 PA2 PA7
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;  
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource1, GPIO_AF_ETH); //引脚复用到网络接口上
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_ETH);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_ETH);

	//配置PC1,PC4 and PC5
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource1, GPIO_AF_ETH); //引脚复用到网络接口上
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource4, GPIO_AF_ETH);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource5, GPIO_AF_ETH);
                                
	//配置PG11, PG14 and PG13 
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_11 | GPIO_Pin_13 | GPIO_Pin_14;
	GPIO_Init(GPIOG, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource11, GPIO_AF_ETH);
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource13, GPIO_AF_ETH);
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource14, GPIO_AF_ETH);
	
	//配置PD3为推完输出
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	//推完输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;  
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	
	/*
	GPIO_ResetBits(RTL8370MB_RST_PORT,RTL8370MB_RST_PIN);					//硬件复位LAN8720
	BaseTimer::Instance()->delay_ms(50);
//	delay_ms(50);	
	GPIO_SetBits(RTL8370MB_RST_PORT,RTL8370MB_RST_PIN);				 	//复位结束 
	*/
	ETHERNET_NVICConfiguration();
	rval=ETH_MACDMA_Config();
	return !rval;					//ETH的规则为:0,失败;1,成功;所以要取反一下 
}

void ETHERNET_NVICConfiguration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	
	NVIC_InitStructure.NVIC_IRQChannel = ETH_IRQn;  //以太网中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0X00; //中断寄存器组2最高优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0X00;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

//得到8370的速度模式
//返回值:
//001:10M半双工
//101:10M全双工
//010:100M半双工
//110:100M全双工
//其他:错误.
u8 RTL8370MB_Get_Speed(void)
{
	u8 val = 0;
	val |= (1<<1);
	val |= (1<<2);
	return val;
	/*
	rtk_port_linkStatus_t linkStatus;
	rtk_port_speed_t speed;
	rtk_port_duplex_t duplex;
	rtk_api_ret_t ret = rtk_port_phyStatus_get(EXT_PORT1,&linkStatus,&speed,&duplex); //从RTL8370MB的port状态读取目前速度
	if(ret != RT_ERR_OK)
		return 1;
	
	u8 val = 0;
	
	if(linkStatus != PORT_LINKUP)
		return 2;
	
	switch(speed)
	{
		case PORT_SPEED_10M:
			val |= 1;
			break;
		case PORT_SPEED_100M:
			val |= (1<<1);
			break;
		default:
			return 3;
	}
	
	switch(duplex)
	{
		case PORT_HALF_DUPLEX:
			break;
		case PORT_FULL_DUPLEX:
			val |= (1<<2);
			break;
		default:
			return 4;
	}
	
	return val;
	*/
}

u8 ETH_MACDMA_Config(void)
{

	ETH_InitTypeDef ETH_InitStructure; 
	
	//使能以太网时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_ETH_MAC | RCC_AHB1Periph_ETH_MAC_Tx |RCC_AHB1Periph_ETH_MAC_Rx, ENABLE);
                        
	
	ETH_DeInit();  								//AHB总线重启以太网
	ETH_SoftwareReset();  						//软件重启网络
	while (ETH_GetSoftwareResetStatus() == SET);//等待软件重启网络完成 
	ETH_StructInit(&ETH_InitStructure); 	 	//初始化网络为默认值  

	///网络MAC参数设置 
	ETH_InitStructure.ETH_AutoNegotiation = ETH_AutoNegotiation_Enable;   			//开启网络自适应功能
	ETH_InitStructure.ETH_LoopbackMode = ETH_LoopbackMode_Disable;					//关闭反馈
	ETH_InitStructure.ETH_RetryTransmission = ETH_RetryTransmission_Disable; 		//关闭重传功能
	ETH_InitStructure.ETH_AutomaticPadCRCStrip = ETH_AutomaticPadCRCStrip_Disable; 	//关闭自动去除PDA/CRC功能 
	ETH_InitStructure.ETH_ReceiveAll = ETH_ReceiveAll_Disable;						//关闭接收所有的帧
	ETH_InitStructure.ETH_BroadcastFramesReception = ETH_BroadcastFramesReception_Enable;//允许接收所有广播帧
	ETH_InitStructure.ETH_PromiscuousMode = ETH_PromiscuousMode_Disable;			//关闭混合模式的地址过滤  
	ETH_InitStructure.ETH_MulticastFramesFilter = ETH_MulticastFramesFilter_Perfect;//对于组播地址使用完美地址过滤   
	ETH_InitStructure.ETH_UnicastFramesFilter = ETH_UnicastFramesFilter_Perfect;	//对单播地址使用完美地址过滤 
	ETH_InitStructure.ETH_InterFrameGap = ETH_InterFrameGap_40Bit;
#ifdef CHECKSUM_BY_HARDWARE
	ETH_InitStructure.ETH_ChecksumOffload = ETH_ChecksumOffload_Enable; 			//开启ipv4和TCP/UDP/ICMP的帧校验和卸载   
#endif
	//当我们使用帧校验和卸载功能的时候，一定要使能存储转发模式,存储转发模式中要保证整个帧存储在FIFO中,
	//这样MAC能插入/识别出帧校验值,当真校验正确的时候DMA就可以处理帧,否则就丢弃掉该帧
	ETH_InitStructure.ETH_DropTCPIPChecksumErrorFrame = ETH_DropTCPIPChecksumErrorFrame_Enable; //开启丢弃TCP/IP错误帧
	ETH_InitStructure.ETH_ReceiveStoreForward = ETH_ReceiveStoreForward_Enable;     //开启接收数据的存储转发模式    
	ETH_InitStructure.ETH_TransmitStoreForward = ETH_TransmitStoreForward_Enable;   //开启发送数据的存储转发模式  

	ETH_InitStructure.ETH_ForwardErrorFrames = ETH_ForwardErrorFrames_Disable;     	//禁止转发错误帧  
	ETH_InitStructure.ETH_ForwardUndersizedGoodFrames = ETH_ForwardUndersizedGoodFrames_Disable;	//不转发过小的好帧 
	ETH_InitStructure.ETH_SecondFrameOperate = ETH_SecondFrameOperate_Enable;  		//打开处理第二帧功能
	ETH_InitStructure.ETH_AddressAlignedBeats = ETH_AddressAlignedBeats_Enable;  	//开启DMA传输的地址对齐功能
	ETH_InitStructure.ETH_FixedBurst = ETH_FixedBurst_Enable;            			//开启固定突发功能    
	ETH_InitStructure.ETH_RxDMABurstLength = ETH_RxDMABurstLength_32Beat;     		//DMA发送的最大突发长度为32个节拍   
	ETH_InitStructure.ETH_TxDMABurstLength = ETH_TxDMABurstLength_32Beat;			//DMA接收的最大突发长度为32个节拍
	ETH_InitStructure.ETH_DMAArbitration = ETH_DMAArbitration_RoundRobin_RxTx_2_1;
	
	//rval=ETH_RTL8370MB_Init(&ETH_InitStructure,RTL8370MB_PHY_ADDRESS);		//配置ETH
	
	u8 rval=0;
	rval=ETH_RTL8370MB_Init(&ETH_InitStructure,RTL8370MB_PHY_ADDRESS);
	if(rval==ETH_SUCCESS)//配置成功
	{
		ETH_DMAITConfig(ETH_DMA_IT_NIS|ETH_DMA_IT_R,ENABLE);  	//使能以太网接收中断	
	}
	
	return rval;
}

extern void lwip_pkt_handle(void);		//在lwip_comm.c里面定义
//以太网中断服务函数
extern "C"{
void ETH_IRQHandler(void)
{
	while(ETH_GetRxPktSize(DMARxDescToGet)!=0) 	//检测是否收到数据包
	{ 
		lwip_pkt_handle();		
	}
	ETH_DMAClearITPendingBit(ETH_DMA_IT_R);
	ETH_DMAClearITPendingBit(ETH_DMA_IT_NIS);
}  
}

//接收一个网卡数据包
//返回值:网络数据包帧结构体
FrameTypeDef ETH_Rx_Packet(void)
{ 
	u32 framelength=0;
	FrameTypeDef frame={0,0};   
	//检查当前描述符,是否属于ETHERNET DMA(设置的时候)/CPU(复位的时候)
	if((DMARxDescToGet->Status&ETH_DMARxDesc_OWN)!=(u32)RESET)
	{	
		frame.length=ETH_ERROR; 
		if ((ETH->DMASR&ETH_DMASR_RBUS)!=(u32)RESET)  
		{ 
			ETH->DMASR = ETH_DMASR_RBUS;//清除ETH DMA的RBUS位  
			ETH->DMARPDR=0;//恢复DMA接收
		}
		return frame;//错误,OWN位被设置了
	}  
	if(((DMARxDescToGet->Status&ETH_DMARxDesc_ES)==(u32)RESET)&& 
	((DMARxDescToGet->Status & ETH_DMARxDesc_LS)!=(u32)RESET)&&  
	((DMARxDescToGet->Status & ETH_DMARxDesc_FS)!=(u32)RESET))  
	{       
		framelength=((DMARxDescToGet->Status&ETH_DMARxDesc_FL)>>ETH_DMARxDesc_FrameLengthShift)-4;//得到接收包帧长度(不包含4字节CRC)
 		frame.buffer = DMARxDescToGet->Buffer1Addr;//得到包数据所在的位置
	}else framelength=ETH_ERROR;//错误  
	frame.length=framelength; 
	frame.descriptor=DMARxDescToGet;  
	//更新ETH DMA全局Rx描述符为下一个Rx描述符
	//为下一次buffer读取设置下一个DMA Rx描述符
	DMARxDescToGet=(ETH_DMADESCTypeDef*)(DMARxDescToGet->Buffer2NextDescAddr);   
	return frame;  
}
//发送一个网卡数据包
//FrameLength:数据包长度
//返回值:ETH_ERROR,发送失败(0)
//		ETH_SUCCESS,发送成功(1)
u8 ETH_Tx_Packet(u16 FrameLength)
{   
	//检查当前描述符,是否属于ETHERNET DMA(设置的时候)/CPU(复位的时候)
	if((DMATxDescToSet->Status&ETH_DMATxDesc_OWN)!=(u32)RESET)return ETH_ERROR;//错误,OWN位被设置了 
 	DMATxDescToSet->ControlBufferSize=(FrameLength&ETH_DMATxDesc_TBS1);//设置帧长度,bits[12:0]
	DMATxDescToSet->Status|=ETH_DMATxDesc_LS|ETH_DMATxDesc_FS;//设置最后一个和第一个位段置位(1个描述符传输一帧)
  	DMATxDescToSet->Status|=ETH_DMATxDesc_OWN;//设置Tx描述符的OWN位,buffer重归ETH DMA
	if((ETH->DMASR&ETH_DMASR_TBUS)!=(u32)RESET)//当Tx Buffer不可用位(TBUS)被设置的时候,重置它.恢复传输
	{ 
		ETH->DMASR=ETH_DMASR_TBUS;//重置ETH DMA TBUS位 
		ETH->DMATPDR=0;//恢复DMA发送
	} 
	//更新ETH DMA全局Tx描述符为下一个Tx描述符
	//为下一次buffer发送设置下一个DMA Tx描述符 
	DMATxDescToSet=(ETH_DMADESCTypeDef*)(DMATxDescToSet->Buffer2NextDescAddr);    
	return ETH_SUCCESS;   
}
//得到当前描述符的Tx buffer地址
//返回值:Tx buffer地址
u32 ETH_GetCurrentTxBuffer(void)
{  
  return DMATxDescToSet->Buffer1Addr;//返回Tx buffer地址  
}

//为ETH底层驱动申请内存
//返回值:0,正常
//    其他,失败
u8 ETH_Mem_Malloc(void)
{ 
//	DMARxDscrTab = (ETH_DMADESCTypeDef*)mymalloc(SRAMIN,ETH_RXBUFNB*sizeof(ETH_DMADESCTypeDef));//申请内存
//	DMATxDscrTab = (ETH_DMADESCTypeDef*)mymalloc(SRAMIN,ETH_TXBUFNB*sizeof(ETH_DMADESCTypeDef));//申请内存  
//	Rx_Buff = (uint8_t*)mymalloc(SRAMIN,ETH_RX_BUF_SIZE*ETH_RXBUFNB);	//申请内存
//	Tx_Buff = (uint8_t*)mymalloc(SRAMIN,ETH_TX_BUF_SIZE*ETH_TXBUFNB);	//申请内存
//	if(!DMARxDscrTab||!DMATxDscrTab||!Rx_Buff||!Tx_Buff)
//	{
//		ETH_Mem_Free();
//		return 1;	//申请失败
//	}	
	return 0;		//申请成功
}

//释放ETH 底层驱动申请的内存
void ETH_Mem_Free(void)
{ 
//	myfree(SRAMIN,DMARxDscrTab);//释放内存
//	myfree(SRAMIN,DMATxDscrTab);//释放内存
//	myfree(SRAMIN,Rx_Buff);		//释放内存
//	myfree(SRAMIN,Tx_Buff);		//释放内存  
}

u8 RTL8370MB_SWITCH_CheatInit(void)
{
	/*
	//MCO2 init
	RCC_PLLI2SCmd(DISABLE);
	RCC_PLLI2SConfig((uint32_t)200, (uint32_t)4);
	RCC_I2SCLKConfig(RCC_I2S2CLKSource_PLLI2S);
	RCC_PLLI2SCmd(ENABLE);
	
	GPIO_InitTypeDef  GPIO_InitStructure;
  
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);//使能PC

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;//PC9
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能开启

  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//50MHz


  GPIO_Init(GPIOC, &GPIO_InitStructure);


	RCC_MCO2Config(RCC_MCO2Source_PLLI2SCLK,RCC_MCO2Div_1);
	*/
	return 0;
}

