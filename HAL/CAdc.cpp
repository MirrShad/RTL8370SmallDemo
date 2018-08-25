#include "CAdc.h"

//#ifdef RUN_FOR_SRC_2000
    #define ADC1_CHL8_PORT GPIOB
    #define ADC1_CHL8_PIN GPIO_Pin_0
    #define ADC1_CHL6_PORT GPIOA
    #define ADC1_CHL6_PIN GPIO_Pin_6
    #define ADC1_CHL5_PORT GPIOA
    #define ADC1_CHL5_PIN GPIO_Pin_5

	#define ADC_DMA_STREAM DMA2_Stream4
	#define ADC_DMA_CHANNEL DMA_Channel_0
//#endif

int CAdc::count_ = 0;
uint16_t CAdc::ADC_ConvertedValue[3];
CAdc::CAdc(ADC_TypeDef* adc_used,uint8_t channel)
	:_firstIn(true)
{
    adc_ = adc_used;
    channel_ = channel;
    if(ADC1 == adc_used)
    {
        switch(channel)
        {
        case ADC_Channel_5:
            port_ = ADC1_CHL5_PORT;
            pin_ = ADC1_CHL5_PIN;
            break;
        case ADC_Channel_6:
            port_ = ADC1_CHL6_PORT;
            pin_ = ADC1_CHL6_PIN;
            break;
        case ADC_Channel_8:
            port_ = ADC1_CHL8_PORT;
            pin_ = ADC1_CHL8_PIN;
            break;
        default:
            break;
        }
    }
}

void CAdc::InitAdcGPIO()
{
  uint32_t RCC_AHBPeriphx;
	GPIO_InitTypeDef GPIO_InitStructure;
  RCC_AHBPeriphx	= RCC_AHB1Periph_GPIOA<< (((uint32_t)port_-(uint32_t)GPIOA)/0x400);
	RCC_AHB1PeriphClockCmd(RCC_AHBPeriphx, ENABLE);
	RCC_AHBPeriphx	= RCC_APB2Periph_ADC1<< (((uint32_t)adc_-(uint32_t)ADC1)/0x100);
	RCC_APB2PeriphClockCmd(RCC_AHBPeriphx, ENABLE);

	GPIO_InitStructure.GPIO_Pin = pin_;
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(port_, &GPIO_InitStructure);
	
	count_++;	
	rank_ = count_;
	
    ADC_RegularChannelConfig(adc_, channel_, rank_, ADC_SampleTime_480Cycles );
}

void CAdc::InitAdc1Regular()
{
	ADC_CommonInitTypeDef ADC_CommonInitStructure;
	ADC_InitTypeDef       ADC_InitStructure;
	
	ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
 	ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
 	ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_1;
 	ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div4;
 	ADC_CommonInit(&ADC_CommonInitStructure);

  ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfConversion = count_;
	ADC_Init(ADC1, &ADC_InitStructure);
    ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);
	ADC_Cmd(ADC1, ENABLE);
	ADC_DMACmd(ADC1,ENABLE); 
}

void CAdc::InitAdc1Injected()
{
	ADC_CommonInitTypeDef ADC_CommonInitStructure;
	ADC_InitTypeDef       ADC_InitStructure;
	
	ADC_Cmd(ADC1, DISABLE);
	ADC_InjectedDiscModeCmd(ADC1,DISABLE);
	
	ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
 	ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
 	ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_1;
 	ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div4;
 	ADC_CommonInit(&ADC_CommonInitStructure);

  ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfConversion = 0;
	ADC_Init(ADC1, &ADC_InitStructure);
	ADC_ExternalTrigInjectedConvEdgeConfig(ADC1,ADC_ExternalTrigInjecConvEdge_None);
	ADC_InjectedSequencerLengthConfig(ADC1,count_);
  //ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);
	ADC_Cmd(ADC1, ENABLE);
	//ADC_DMACmd(ADC1,ENABLE); 
}

void CAdc::InitDMA()
{
	DMA_InitTypeDef  DMA_InitStructure;
	
	if((u32)ADC_DMA_STREAM>(u32)DMA2){RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2,ENABLE);}
	else {RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1,ENABLE);}

  DMA_DeInit(ADC_DMA_STREAM);

	DMA_InitStructure.DMA_Channel = ADC_DMA_CHANNEL;  
  DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&ADC1->DR;//bug::if it is ADC2
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)ADC_ConvertedValue;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
  DMA_InitStructure.DMA_BufferSize = 1;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_Init(ADC_DMA_STREAM, &DMA_InitStructure);

  DMA_SetCurrDataCounter(ADC_DMA_STREAM,count_);
    DMA_Cmd (ADC_DMA_STREAM,ENABLE);  

}

void CAdc::startConv()
{
  //ADC_SoftwareStartConv(adc_);//
	if(_firstIn)
	{
		ADC_InjectedChannelConfig(adc_, channel_, rank_, ADC_SampleTime_480Cycles );
		_firstIn = false;
	}
	ADC_SoftwareStartInjectedConv(adc_);
}

uint16_t CAdc::revData()
{
	if(DMA_GetFlagStatus(ADC_DMA_STREAM,DMA_FLAG_TCIF4)==RESET)
	{
		return (uint16_t)-1;
	}
	return ADC_ConvertedValue[rank_-1];
}


