#include "stm32f4x7_rtl8370mb_eth.h"
#include "rtl8367c_asicdrv_phy.h"
#include "rtk_switch.h"
#include "Timer.h"

/**
  * @brief  Initializes the ETHERNET peripheral according to the specified
  *   parameters in the ETH_InitStruct .
  * @param ETH_InitStruct: pointer to a ETH_InitTypeDef structure that contains
  *   the configuration information for the specified ETHERNET peripheral.
  * @param PHYAddress: external PHY address                    
  * @retval ETH_ERROR: Ethernet initialization failed
  *         ETH_SUCCESS: Ethernet successfully initialized                 
  */
uint32_t ETH_RTL8370MB_Init(ETH_InitTypeDef* ETH_InitStruct, uint16_t PHYAddress)
{
  uint32_t  tmpreg = 0;
  __IO uint32_t i = 0;
  RCC_ClocksTypeDef  rcc_clocks;
  uint32_t hclk = 60000000;
  __IO uint32_t timeout = 0;
  /* Check the parameters */
  /* MAC --------------------------*/ 
  assert_param(IS_ETH_AUTONEGOTIATION(ETH_InitStruct->ETH_AutoNegotiation));
  assert_param(IS_ETH_WATCHDOG(ETH_InitStruct->ETH_Watchdog));
  assert_param(IS_ETH_JABBER(ETH_InitStruct->ETH_Jabber));
  assert_param(IS_ETH_INTER_FRAME_GAP(ETH_InitStruct->ETH_InterFrameGap));
  assert_param(IS_ETH_CARRIER_SENSE(ETH_InitStruct->ETH_CarrierSense));
  assert_param(IS_ETH_SPEED(ETH_InitStruct->ETH_Speed));
  assert_param(IS_ETH_RECEIVE_OWN(ETH_InitStruct->ETH_ReceiveOwn));
  assert_param(IS_ETH_LOOPBACK_MODE(ETH_InitStruct->ETH_LoopbackMode));
  assert_param(IS_ETH_DUPLEX_MODE(ETH_InitStruct->ETH_Mode));
  assert_param(IS_ETH_CHECKSUM_OFFLOAD(ETH_InitStruct->ETH_ChecksumOffload));
  assert_param(IS_ETH_RETRY_TRANSMISSION(ETH_InitStruct->ETH_RetryTransmission));
  assert_param(IS_ETH_AUTOMATIC_PADCRC_STRIP(ETH_InitStruct->ETH_AutomaticPadCRCStrip));
  assert_param(IS_ETH_BACKOFF_LIMIT(ETH_InitStruct->ETH_BackOffLimit));
  assert_param(IS_ETH_DEFERRAL_CHECK(ETH_InitStruct->ETH_DeferralCheck));
  assert_param(IS_ETH_RECEIVE_ALL(ETH_InitStruct->ETH_ReceiveAll));
  assert_param(IS_ETH_SOURCE_ADDR_FILTER(ETH_InitStruct->ETH_SourceAddrFilter));
  assert_param(IS_ETH_CONTROL_FRAMES(ETH_InitStruct->ETH_PassControlFrames));
  assert_param(IS_ETH_BROADCAST_FRAMES_RECEPTION(ETH_InitStruct->ETH_BroadcastFramesReception));
  assert_param(IS_ETH_DESTINATION_ADDR_FILTER(ETH_InitStruct->ETH_DestinationAddrFilter));
  assert_param(IS_ETH_PROMISCIOUS_MODE(ETH_InitStruct->ETH_PromiscuousMode));
  assert_param(IS_ETH_MULTICAST_FRAMES_FILTER(ETH_InitStruct->ETH_MulticastFramesFilter));  
  assert_param(IS_ETH_UNICAST_FRAMES_FILTER(ETH_InitStruct->ETH_UnicastFramesFilter));
  assert_param(IS_ETH_PAUSE_TIME(ETH_InitStruct->ETH_PauseTime));
  assert_param(IS_ETH_ZEROQUANTA_PAUSE(ETH_InitStruct->ETH_ZeroQuantaPause));
  assert_param(IS_ETH_PAUSE_LOW_THRESHOLD(ETH_InitStruct->ETH_PauseLowThreshold));
  assert_param(IS_ETH_UNICAST_PAUSE_FRAME_DETECT(ETH_InitStruct->ETH_UnicastPauseFrameDetect));
  assert_param(IS_ETH_RECEIVE_FLOWCONTROL(ETH_InitStruct->ETH_ReceiveFlowControl));
  assert_param(IS_ETH_TRANSMIT_FLOWCONTROL(ETH_InitStruct->ETH_TransmitFlowControl));
  assert_param(IS_ETH_VLAN_TAG_COMPARISON(ETH_InitStruct->ETH_VLANTagComparison));
  assert_param(IS_ETH_VLAN_TAG_IDENTIFIER(ETH_InitStruct->ETH_VLANTagIdentifier));
  /* DMA --------------------------*/
  assert_param(IS_ETH_DROP_TCPIP_CHECKSUM_FRAME(ETH_InitStruct->ETH_DropTCPIPChecksumErrorFrame));
  assert_param(IS_ETH_RECEIVE_STORE_FORWARD(ETH_InitStruct->ETH_ReceiveStoreForward));
  assert_param(IS_ETH_FLUSH_RECEIVE_FRAME(ETH_InitStruct->ETH_FlushReceivedFrame));
  assert_param(IS_ETH_TRANSMIT_STORE_FORWARD(ETH_InitStruct->ETH_TransmitStoreForward));
  assert_param(IS_ETH_TRANSMIT_THRESHOLD_CONTROL(ETH_InitStruct->ETH_TransmitThresholdControl));
  assert_param(IS_ETH_FORWARD_ERROR_FRAMES(ETH_InitStruct->ETH_ForwardErrorFrames));
  assert_param(IS_ETH_FORWARD_UNDERSIZED_GOOD_FRAMES(ETH_InitStruct->ETH_ForwardUndersizedGoodFrames));
  assert_param(IS_ETH_RECEIVE_THRESHOLD_CONTROL(ETH_InitStruct->ETH_ReceiveThresholdControl));
  assert_param(IS_ETH_SECOND_FRAME_OPERATE(ETH_InitStruct->ETH_SecondFrameOperate));
  assert_param(IS_ETH_ADDRESS_ALIGNED_BEATS(ETH_InitStruct->ETH_AddressAlignedBeats));
  assert_param(IS_ETH_FIXED_BURST(ETH_InitStruct->ETH_FixedBurst));
  assert_param(IS_ETH_RXDMA_BURST_LENGTH(ETH_InitStruct->ETH_RxDMABurstLength));
  assert_param(IS_ETH_TXDMA_BURST_LENGTH(ETH_InitStruct->ETH_TxDMABurstLength)); 
  assert_param(IS_ETH_DMA_DESC_SKIP_LENGTH(ETH_InitStruct->ETH_DescriptorSkipLength));  
  assert_param(IS_ETH_DMA_ARBITRATION_ROUNDROBIN_RXTX(ETH_InitStruct->ETH_DMAArbitration));       
  /*-------------------------------- MAC Config ------------------------------*/   
  /*---------------------- ETHERNET MACMIIAR Configuration -------------------*/
  /* Get the ETHERNET MACMIIAR value */
  tmpreg = ETH->MACMIIAR;
  /* Clear CSR Clock Range CR[2:0] bits */
  tmpreg &= MACMIIAR_CR_MASK;
  /* Get hclk frequency value */
  RCC_GetClocksFreq(&rcc_clocks);
  hclk = rcc_clocks.HCLK_Frequency;
  
  /* Set CR bits depending on hclk value */
  if((hclk >= 20000000)&&(hclk < 35000000))
  {
    /* CSR Clock Range between 20-35 MHz */
    tmpreg |= (uint32_t)ETH_MACMIIAR_CR_Div16;
  }
  else if((hclk >= 35000000)&&(hclk < 60000000))
  {
    /* CSR Clock Range between 35-60 MHz */ 
    tmpreg |= (uint32_t)ETH_MACMIIAR_CR_Div26;    
  }  
  else if((hclk >= 60000000)&&(hclk < 100000000))
  {
    /* CSR Clock Range between 60-100 MHz */ 
    tmpreg |= (uint32_t)ETH_MACMIIAR_CR_Div42;    
  }  
  else if((hclk >= 100000000)&&(hclk < 150000000))
  {
    /* CSR Clock Range between 100-150 MHz */ 
    tmpreg |= (uint32_t)ETH_MACMIIAR_CR_Div62;    
  }   
  else /* ((hclk >= 150000000)&&(hclk <= 168000000)) */
  {
    /* CSR Clock Range between 150-168 MHz */ 
    tmpreg |= (uint32_t)ETH_MACMIIAR_CR_Div102;    
  }
  
  /* Write to ETHERNET MAC MIIAR: Configure the ETHERNET CSR Clock Range */
  ETH->MACMIIAR = (uint32_t)tmpreg;  
  /*-------------------- PHY initialization and configuration ----------------*/
  /* Put the PHY in reset mode */
	/*
  if( rtl8367c_setAsicPHYReg(rtk_switch_port_L2P_get(EXT_PORT1), static_cast<rtk_port_phy_reg_t>(PHY_STATUS_REG), PHY_Reset) != RT_ERR_OK)
  {
    // Return ERROR in case of write timeout 
    return ETH_ERROR;
  }
  
  // Delay to assure PHY reset 
  BaseTimer::Instance()->delay_ms(50);
  rtk_uint32 control_reg = 0;
	rtk_uint32 status_reg = 0;
	
  if(ETH_InitStruct->ETH_AutoNegotiation != ETH_AutoNegotiation_Disable)
  {  
    // We wait for linked status... 
    do
    {
      timeout++;
			rtk_api_ret_t ret = rtl8367c_setAsicPHYReg(rtk_switch_port_L2P_get(EXT_PORT1), static_cast<rtk_port_phy_reg_t>(PHY_CONTROL_REG), control_reg);//rtk_port_phyReg_get(EXT_PORT1, static_cast<rtk_port_phy_reg_t>(PHY_CONTROL_REG), &control_reg);
			if(ret != RT_ERR_OK) return ETH_ERROR;
    } while (!(control_reg & PHY_Linked_Status) && (timeout < PHY_READ_TO));

    // Return ERROR in case of timeout 
    if(timeout == PHY_READ_TO)
    {
      return ETH_ERROR;
    }

    // Reset Timeout counter 
    timeout = 0; 
    // Enable Auto-Negotiation 
    if(rtk_port_phyReg_set(EXT_PORT1, static_cast<rtk_port_phy_reg_t>(PHY_CONTROL_REG), PHY_AutoNegotiation) != RT_ERR_OK)
    {
      // Return ERROR in case of write timeout 
      return ETH_ERROR;
    }

    // Wait until the auto-negotiation will be completed 
    do
    {
      timeout++;
			rtk_api_ret_t ret = rtk_port_phyReg_get(EXT_PORT1, static_cast<rtk_port_phy_reg_t>(PHY_STATUS_REG), &status_reg);
			if(ret != RT_ERR_OK) return ETH_ERROR;
    } while (!(status_reg & PHY_AutoNego_Complete) && (timeout < (uint32_t)PHY_READ_TO));  

    // Return ERROR in case of timeout 
    if(timeout == PHY_READ_TO)
    {
      return ETH_ERROR;
    }

    // Reset Timeout counter 
    timeout = 0;
    
		rtk_port_linkStatus_t linkStatus;
		rtk_port_speed_t linkSpeed;
		rtk_port_duplex_t linkDuplex;
    // Read the result of the auto-negotiation 
    //RegValue = ETH_ReadPHYRegister(PHYAddress, PHY_SR);
		if( rtk_port_phyStatus_get(EXT_PORT1,&linkStatus,&linkSpeed,&linkDuplex) != RT_ERR_OK)
		{
			return ETH_ERROR;
		}
  
    // Configure the MAC with the Duplex Mode fixed by the auto-negotiation process 
    if((linkDuplex & PORT_FULL_DUPLEX) != (uint32_t)RESET)
    {
      // Set Ethernet duplex mode to Full-duplex following the auto-negotiation 
      ETH_InitStruct->ETH_Mode = ETH_Mode_FullDuplex;  
    }
    else
    {
      // Set Ethernet duplex mode to Half-duplex following the auto-negotiation 
      ETH_InitStruct->ETH_Mode = ETH_Mode_HalfDuplex;           
    }

    // Configure the MAC with the speed fixed by the auto-negotiation process 
    if(linkSpeed & PORT_SPEED_10M)
    {  
      // Set Ethernet speed to 10M following the auto-negotiation 
      ETH_InitStruct->ETH_Speed = ETH_Speed_10M; 
    }
    else if(linkSpeed & PORT_SPEED_100M)
    {   
      // Set Ethernet speed to 100M following the auto-negotiation 
      ETH_InitStruct->ETH_Speed = ETH_Speed_100M;      
    }
		else
		{
			return ETH_ERROR;
		}
  }
  else
  {
    if(!ETH_WritePHYRegister(PHYAddress, PHY_BCR, ((uint16_t)(ETH_InitStruct->ETH_Mode >> 3) |
                                                   (uint16_t)(ETH_InitStruct->ETH_Speed >> 1))))
    {
      // Return ERROR in case of write timeout 
      return ETH_ERROR;
    }
    // Delay to assure PHY configuration 
    BaseTimer::Instance()->delay_ms(50);
    
  }  
	*/
  
	ETH_InitStruct->ETH_Mode = ETH_Mode_FullDuplex;
	ETH_InitStruct->ETH_Speed = ETH_Speed_100M; 
  /*------------------------ ETHERNET MACCR Configuration --------------------*/
  /* Get the ETHERNET MACCR value */  
  tmpreg = ETH->MACCR;
  /* Clear WD, PCE, PS, TE and RE bits */
  tmpreg &= MACCR_CLEAR_MASK;
  /* Set the WD bit according to ETH_Watchdog value */
  /* Set the JD: bit according to ETH_Jabber value */
  /* Set the IFG bit according to ETH_InterFrameGap value */ 
  /* Set the DCRS bit according to ETH_CarrierSense value */  
  /* Set the FES bit according to ETH_Speed value */ 
  /* Set the DO bit according to ETH_ReceiveOwn value */ 
  /* Set the LM bit according to ETH_LoopbackMode value */ 
  /* Set the DM bit according to ETH_Mode value */ 
  /* Set the IPCO bit according to ETH_ChecksumOffload value */                   
  /* Set the DR bit according to ETH_RetryTransmission value */ 
  /* Set the ACS bit according to ETH_AutomaticPadCRCStrip value */ 
  /* Set the BL bit according to ETH_BackOffLimit value */ 
  /* Set the DC bit according to ETH_DeferralCheck value */                          
  tmpreg |= (uint32_t)(ETH_InitStruct->ETH_Watchdog | 
                  ETH_InitStruct->ETH_Jabber | 
                  ETH_InitStruct->ETH_InterFrameGap |
                  ETH_InitStruct->ETH_CarrierSense |
                  ETH_InitStruct->ETH_Speed | 
                  ETH_InitStruct->ETH_ReceiveOwn |
                  ETH_InitStruct->ETH_LoopbackMode |
                  ETH_InitStruct->ETH_Mode | 
                  ETH_InitStruct->ETH_ChecksumOffload |    
                  ETH_InitStruct->ETH_RetryTransmission | 
                  ETH_InitStruct->ETH_AutomaticPadCRCStrip | 
                  ETH_InitStruct->ETH_BackOffLimit | 
                  ETH_InitStruct->ETH_DeferralCheck);
  /* Write to ETHERNET MACCR */
  ETH->MACCR = (uint32_t)tmpreg;
  
  /*----------------------- ETHERNET MACFFR Configuration --------------------*/ 
  /* Set the RA bit according to ETH_ReceiveAll value */
  /* Set the SAF and SAIF bits according to ETH_SourceAddrFilter value */
  /* Set the PCF bit according to ETH_PassControlFrames value */
  /* Set the DBF bit according to ETH_BroadcastFramesReception value */
  /* Set the DAIF bit according to ETH_DestinationAddrFilter value */
  /* Set the PR bit according to ETH_PromiscuousMode value */
  /* Set the PM, HMC and HPF bits according to ETH_MulticastFramesFilter value */
  /* Set the HUC and HPF bits according to ETH_UnicastFramesFilter value */
  /* Write to ETHERNET MACFFR */  
  ETH->MACFFR = (uint32_t)(ETH_InitStruct->ETH_ReceiveAll | 
                          ETH_InitStruct->ETH_SourceAddrFilter |
                          ETH_InitStruct->ETH_PassControlFrames |
                          ETH_InitStruct->ETH_BroadcastFramesReception | 
                          ETH_InitStruct->ETH_DestinationAddrFilter |
                          ETH_InitStruct->ETH_PromiscuousMode |
                          ETH_InitStruct->ETH_MulticastFramesFilter |
                          ETH_InitStruct->ETH_UnicastFramesFilter); 
  /*--------------- ETHERNET MACHTHR and MACHTLR Configuration ---------------*/
  /* Write to ETHERNET MACHTHR */
  ETH->MACHTHR = (uint32_t)ETH_InitStruct->ETH_HashTableHigh;
  /* Write to ETHERNET MACHTLR */
  ETH->MACHTLR = (uint32_t)ETH_InitStruct->ETH_HashTableLow;
  /*----------------------- ETHERNET MACFCR Configuration --------------------*/
  /* Get the ETHERNET MACFCR value */  
  tmpreg = ETH->MACFCR;
  /* Clear xx bits */
  tmpreg &= MACFCR_CLEAR_MASK;
  
  /* Set the PT bit according to ETH_PauseTime value */
  /* Set the DZPQ bit according to ETH_ZeroQuantaPause value */
  /* Set the PLT bit according to ETH_PauseLowThreshold value */
  /* Set the UP bit according to ETH_UnicastPauseFrameDetect value */
  /* Set the RFE bit according to ETH_ReceiveFlowControl value */
  /* Set the TFE bit according to ETH_TransmitFlowControl value */  
  tmpreg |= (uint32_t)((ETH_InitStruct->ETH_PauseTime << 16) | 
                   ETH_InitStruct->ETH_ZeroQuantaPause |
                   ETH_InitStruct->ETH_PauseLowThreshold |
                   ETH_InitStruct->ETH_UnicastPauseFrameDetect | 
                   ETH_InitStruct->ETH_ReceiveFlowControl |
                   ETH_InitStruct->ETH_TransmitFlowControl); 
  /* Write to ETHERNET MACFCR */
  ETH->MACFCR = (uint32_t)tmpreg;
  /*----------------------- ETHERNET MACVLANTR Configuration -----------------*/
  /* Set the ETV bit according to ETH_VLANTagComparison value */
  /* Set the VL bit according to ETH_VLANTagIdentifier value */  
  ETH->MACVLANTR = (uint32_t)(ETH_InitStruct->ETH_VLANTagComparison | 
                             ETH_InitStruct->ETH_VLANTagIdentifier); 
       
  /*-------------------------------- DMA Config ------------------------------*/
  /*----------------------- ETHERNET DMAOMR Configuration --------------------*/
  /* Get the ETHERNET DMAOMR value */  
  tmpreg = ETH->DMAOMR;
  /* Clear xx bits */
  tmpreg &= DMAOMR_CLEAR_MASK;
  
  /* Set the DT bit according to ETH_DropTCPIPChecksumErrorFrame value */
  /* Set the RSF bit according to ETH_ReceiveStoreForward value */
  /* Set the DFF bit according to ETH_FlushReceivedFrame value */
  /* Set the TSF bit according to ETH_TransmitStoreForward value */
  /* Set the TTC bit according to ETH_TransmitThresholdControl value */
  /* Set the FEF bit according to ETH_ForwardErrorFrames value */
  /* Set the FUF bit according to ETH_ForwardUndersizedGoodFrames value */
  /* Set the RTC bit according to ETH_ReceiveThresholdControl value */
  /* Set the OSF bit according to ETH_SecondFrameOperate value */
  tmpreg |= (uint32_t)(ETH_InitStruct->ETH_DropTCPIPChecksumErrorFrame | 
                  ETH_InitStruct->ETH_ReceiveStoreForward |
                  ETH_InitStruct->ETH_FlushReceivedFrame |
                  ETH_InitStruct->ETH_TransmitStoreForward | 
                  ETH_InitStruct->ETH_TransmitThresholdControl |
                  ETH_InitStruct->ETH_ForwardErrorFrames |
                  ETH_InitStruct->ETH_ForwardUndersizedGoodFrames |
                  ETH_InitStruct->ETH_ReceiveThresholdControl |                                   
                  ETH_InitStruct->ETH_SecondFrameOperate); 
  /* Write to ETHERNET DMAOMR */
  ETH->DMAOMR = (uint32_t)tmpreg;
  
  /*----------------------- ETHERNET DMABMR Configuration --------------------*/ 
  /* Set the AAL bit according to ETH_AddressAlignedBeats value */
  /* Set the FB bit according to ETH_FixedBurst value */
  /* Set the RPBL and 4*PBL bits according to ETH_RxDMABurstLength value */
  /* Set the PBL and 4*PBL bits according to ETH_TxDMABurstLength value */
  /* Set the DSL bit according to ETH_DesciptorSkipLength value */
  /* Set the PR and DA bits according to ETH_DMAArbitration value */         
  ETH->DMABMR = (uint32_t)(ETH_InitStruct->ETH_AddressAlignedBeats | 
                          ETH_InitStruct->ETH_FixedBurst |
                          ETH_InitStruct->ETH_RxDMABurstLength | /* !! if 4xPBL is selected for Tx or Rx it is applied for the other */
                          ETH_InitStruct->ETH_TxDMABurstLength | 
                         (ETH_InitStruct->ETH_DescriptorSkipLength << 2) |
                          ETH_InitStruct->ETH_DMAArbitration |
                          ETH_DMABMR_USP); /* Enable use of separate PBL for Rx and Tx */
                          
  #ifdef USE_ENHANCED_DMA_DESCRIPTORS
    /* Enable the Enhanced DMA descriptors */
    ETH->DMABMR |= ETH_DMABMR_EDE;
  #endif /* USE_ENHANCED_DMA_DESCRIPTORS */
                              
  /* Return Ethernet configuration success */
  return ETH_SUCCESS;
}
