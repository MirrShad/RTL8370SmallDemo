/**
  ******************************************************************************
  * @file    EEPROM_Emulation/src/stm32f4xx_it.c 
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    10-October-2011
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */ 


#ifdef __cplusplus
#include "Timer.h"
#include "Console.h"
#include "MessageTask.h"

#ifdef RUN_FOR_SRC_2000
#include "PowerLogicBoard.h"
#endif

extern "C" {
#endif 
/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_it.h"

/** @addtogroup EEPROM_Emulation
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M4 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief   This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
	
//	Console::Instance()->postErr("Fall into HardFault!");
	Message::Instance()->postMsg("Error: %s(%d)-%s(): %s", __FILE__, __LINE__, __FUNCTION__, "Fall into HardFault!\r\n");

  while(1)
	{
		Console::Instance()->runTransmitter();
			
	}
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**********************************************************************************************************
*��������SysTick_Handler()
*������void
*����ֵ��void
*���ܣ�ϵͳ���Ķ�ʱ���жϺ���
**********************************************************************************************************/

void SysTick_Handler(void)
{
	CPUTIMER0_ISR();
}
/******************************************************************************/
/*                 STM32F4xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f4xx.s).                                               */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

//
//void DMA2_Stream7_IRQHandler(void)  
//
void DMA2_Stream7_IRQHandler(void)  
{
	DMA_ClearITPendingBit(DMA2_Stream7,DMA_IT_TCIF7);
}


#ifdef RUN_FOR_SRC_2000
/**
  * @}
  */ 
void SPI2_IRQHandler(void)
{
 if (SPI_I2S_GetITStatus(SPI2, SPI_I2S_IT_RXNE) == SET)
 {  
		SPI_I2S_ClearITPendingBit( SPI2 , SPI_I2S_IT_RXNE );
		PowerLogicBoard::Instance()->receiveData(SPI2->DR);		
 }
}
#endif

#ifdef __cplusplus
}
#endif 
/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
