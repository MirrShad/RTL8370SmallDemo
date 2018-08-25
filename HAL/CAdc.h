/**
  ******************************************************************************
  * @file    CAdc.h
  * @author  Wuzeling
  * @version V1.2
  * @date    2018-03-01
  * @brief   This file defines the CAdc class.
  ******************************************************************************/
/*******************************************************************************
* release information
* 2016/01/02 chenxx add DmaSendArray();
*******************************************************************************/
#ifndef __CADC_H
#define __CADC_H

#include "stm32f4xx.h"

class CAdc
{
public:
    CAdc(ADC_TypeDef* adc_used,uint8_t channel);
    
    void InitAdcGPIO();
    static void InitAdc1Regular();
		static void InitAdc1Injected();
    void startConv();
		uint16_t revData();
    static void InitDMA();

private:
		static int count_;
		int rank_;
    ADC_TypeDef* adc_;
    uint8_t channel_;
    GPIO_TypeDef* port_;
    uint16_t pin_;
		bool _firstIn;
    static uint16_t ADC_ConvertedValue[3];

};

#endif// __CADC_H
