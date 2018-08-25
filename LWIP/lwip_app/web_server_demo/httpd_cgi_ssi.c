#include "lwip/debug.h"
#include "httpd.h"
#include "lwip/tcp.h"
#include "fs.h"
#include "lwip_comm.h"
#include "led.h"
#include "beep.h"
#include "adc.h"
#include "rtc.h"
#include "lcd.h"

#include <string.h>
#include <stdlib.h>


#define NUM_CONFIG_CGI_URIS	(sizeof(ppcURLs) / sizeof(tCGI))
#define NUM_CONFIG_SSI_TAGS	(sizeof(ppcTAGs) / sizeof(char *))
	
extern short Get_Temprate(void);  //声明Get_Temperate()函数
 
//控制LED的CGI handler
const char* LEDS_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
const char* BEEP_CGI_Handler(int iIndex,int iNumParams,char *pcParam[],char *pcValue[]);

static const char *ppcTAGs[]=  //SSI的Tag
{
	"t", //ADC值
	"w", //温度值
	"h", //时间
	"y"  //日期
};


static const tCGI ppcURLs[]= //cgi程序
{
	{"/leds.cgi",LEDS_CGI_Handler},
	{"/beep.cgi",BEEP_CGI_Handler},
};


//当web客户端请求浏览器的时候,使用此函数被CGI handler调用
static int FindCGIParameter(const char *pcToFind,char *pcParam[],int iNumParams)
{
	int iLoop;
	for(iLoop = 0;iLoop < iNumParams;iLoop ++ )
	{
		if(strcmp(pcToFind,pcParam[iLoop]) == 0)
		{
			return (iLoop); //返回iLOOP
		}
	}
	return (-1);
}


//SSIHandler中需要用到的处理ADC的函数
void ADC_Handler(char *pcInsert)
{ 
    char Digit1=0, Digit2=0, Digit3=0, Digit4=0; 
    uint32_t ADCVal = 0;        

     //获取ADC的值
     ADCVal = Get_Adc_Average(5,10); //获取ADC1_CH5的电压值
		
     
     //转换为电压 ADCVval * 0.8mv
     ADCVal = (uint32_t)(ADCVal * 0.8);  
     
     Digit1= ADCVal/1000;
     Digit2= (ADCVal-(Digit1*1000))/100 ;
     Digit3= (ADCVal-((Digit1*1000)+(Digit2*100)))/10;
     Digit4= ADCVal -((Digit1*1000)+(Digit2*100)+ (Digit3*10));
        
     //准备添加到html中的数据
     *pcInsert       = (char)(Digit1+0x30);
     *(pcInsert + 1) = (char)(Digit2+0x30);
     *(pcInsert + 2) = (char)(Digit3+0x30);
     *(pcInsert + 3) = (char)(Digit4+0x30);
}

//SSIHandler中需要用到的处理内部温度传感器的函数
void Temperate_Handler(char *pcInsert)
{
		char Digit1=0, Digit2=0, Digit3=0, Digit4=0,Digit5=0; 
		short Temperate = 0;
		
		//获取内部温度值
		Temperate = Get_Temprate(); //获取温度值 此处扩大了100倍
	  Digit1 = Temperate / 10000;
		Digit2 = (Temperate % 10000)/1000;
    Digit3 = (Temperate % 1000)/100 ;
    Digit4 = (Temperate % 100)/10;
    Digit5 = Temperate % 10;
		//添加到html中的数据
		*pcInsert 		= (char)(Digit1+0x30);
		*(pcInsert+1) = (char)(Digit2+0x30);
		*(pcInsert+2)	=	(char)(Digit3+0x30);
		*(pcInsert+3) = '.';
		*(pcInsert+4) = (char)(Digit4+0x30);
		*(pcInsert+5) = (char)(Digit5+0x30);
}

//SSIHandler中需要用到的处理RTC时间的函数
void RTCTime_Handler(char *pcInsert)
{
	u8 hour,min,sec;
	RTC_TimeTypeDef RTC_TimeStruct;
	RTC_GetTime(RTC_Format_BIN,&RTC_TimeStruct);
  hour=RTC_TimeStruct.RTC_Hours;
	min=RTC_TimeStruct.RTC_Minutes;
	sec=RTC_TimeStruct.RTC_Seconds;

  	
	*pcInsert = 		(char)((hour/10) + 0x30);
	*(pcInsert+1) = (char)((hour%10) + 0x30);
	*(pcInsert+2) = ':';
	*(pcInsert+3) = (char)((min/10) + 0x30);
	*(pcInsert+4) = (char)((min%10) + 0x30);
	*(pcInsert+5) = ':';
	*(pcInsert+6) = (char)((sec/10) + 0x30);
	*(pcInsert+7) = (char)((sec%10) + 0x30);
}

//SSIHandler中需要用到的处理RTC日期的函数
void RTCdate_Handler(char *pcInsert)
{
	u8 year,month,date,week;
	
	RTC_DateTypeDef RTC_DateStruct;
	RTC_GetDate(RTC_Format_BIN, &RTC_DateStruct);
	
	year=RTC_DateStruct.RTC_Year;
	month=RTC_DateStruct.RTC_Month;
	date=RTC_DateStruct.RTC_Date;
	week=RTC_DateStruct.RTC_WeekDay;
	
	*pcInsert = '2';
	*(pcInsert+1) = '0';
	*(pcInsert+2) = (char)((year/10) + 0x30);
	*(pcInsert+3) = (char)((year%10) + 0x30);
	*(pcInsert+4) = '-';
	*(pcInsert+5) = (char)((month/10) + 0x30);
	*(pcInsert+6) = (char)((month%10) + 0x30);
	*(pcInsert+7) = '-';
	*(pcInsert+8) = (char)((date/10) + 0x30);
	*(pcInsert+9) = (char)((date%10) + 0x30);
	*(pcInsert+10) = ' ';
	*(pcInsert+11) = 'w';
	*(pcInsert+12) = 'e';
	*(pcInsert+13) = 'e';
	*(pcInsert+14) = 'k';
	*(pcInsert+15) = ':';
	*(pcInsert+16) = (char)(week + 0x30);
	
}
//SSI的Handler句柄
static u16_t SSIHandler(int iIndex,char *pcInsert,int iInsertLen)
{
	switch(iIndex)
	{
		case 0: 
				ADC_Handler(pcInsert);
				break;
		case 1:
				Temperate_Handler(pcInsert);
				break;
		case 2:
				RTCTime_Handler(pcInsert);
				break;
		case 3:
				RTCdate_Handler(pcInsert);
				break;
	}
	return strlen(pcInsert);
}

//CGI LED控制句柄
const char* LEDS_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
  u8 i=0;  //注意根据自己的GET的参数的多少来选择i值范围
  
	iIndex = FindCGIParameter("LED1",pcParam,iNumParams);  //找到led的索引号
	//只有一个CGI句柄 iIndex=0
	if (iIndex != -1)
	{
			LED1=1;  //关闭所有的LED1灯
		for (i=0; i<iNumParams; i++) //检查CGI参数: example GET /leds.cgi?led=2&led=4 */
		{
		  if (strcmp(pcParam[i] , "LED1")==0)  //检查参数"led"
		  {
			if(strcmp(pcValue[i], "LED1ON") ==0)  //改变LED1状态
			  LED1=0; //打开LED1
					else if(strcmp(pcValue[i],"LED1OFF") == 0)
						LED1=1; //关闭LED1
		  }
		}
	 }
	if(LED1 == 0 && BEEP == 0) 		return "/STM32F407LED_ON_BEEP_OFF.shtml";  	//LED1开,BEEP关
	else if(LED1 == 0 && BEEP == 1) return "/STM32F407LED_ON_BEEP_ON.shtml";	//LED1开,BEEP开
	else if(LED1 == 1 && BEEP == 1) return "/STM32F407LED_OFF_BEEP_ON.shtml";	//LED1关,BEEP开
	else return "/STM32F407LED_OFF_BEEP_OFF.shtml";   							//LED1关,BEEP关					
}

//BEEP的CGI控制句柄
const char *BEEP_CGI_Handler(int iIndex,int iNumParams,char *pcParam[],char *pcValue[])
{
	u8 i=0;
	iIndex = FindCGIParameter("BEEP",pcParam,iNumParams);  //找到BEEP的索引号
	if(iIndex != -1) //找到BEEP索引号
	{
		BEEP=0;  //关闭
		for(i = 0;i < iNumParams;i++)
		{
			if(strcmp(pcParam[i],"BEEP") == 0)  //查找CGI参数
			{
				if(strcmp(pcValue[i],"BEEPON") == 0) //打开BEEP
					BEEP = 1;
				else if(strcmp(pcValue[i],"BEEPOFF") == 0) //关闭BEEP
					BEEP = 0;
			}
		}
	}
	if(LED1 == 0 && BEEP == 0) 		return "/STM32F407LED_ON_BEEP_OFF.shtml";  	//LED1开,BEEP关
	else if(LED1 == 0 && BEEP == 1) return "/STM32F407LED_ON_BEEP_ON.shtml";	//LED1开,BEEP开
	else if(LED1 == 1 && BEEP == 1) return "/STM32F407LED_OFF_BEEP_ON.shtml";	//LED1关,BEEP开
	else return "/STM32F407LED_OFF_BEEP_OFF.shtml";   //LED1关,BEEP关
	
}

//SSI句柄初始化
void httpd_ssi_init(void)
{  
	//配置内部温度传感器的SSI句柄
	http_set_ssi_handler(SSIHandler,ppcTAGs,NUM_CONFIG_SSI_TAGS);
}

//CGI句柄初始化
void httpd_cgi_init(void)
{ 
  //配置CGI句柄LEDs control CGI) */
  http_set_cgi_handlers(ppcURLs, NUM_CONFIG_CGI_URIS);
}


