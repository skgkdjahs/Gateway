#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "Led.h"
#include "Beep.h"
#include "Key.h"
#include "Serial.h"
#include "stdio.h"
#include "string.h"
#include "ADC.h"

uint8_t EXTI_SuccessFlag = 0;

int main(void)
{
	// 设置当前程序中的中断优先级分组情况：两位抢占优先级、两位子优先级
	NVIC_SetPriorityGrouping(NVIC_PriorityGroup_2);
	
	Led_Init();
	Key_Init();
	ADC3_Init();
	USART1_Init(115200);
	
	uint16_t ADC_Value = 0;
	printf("串口输出功能正常\r\n");
	
	while(1)
	{
//		printf("主程序正在进行\r\n");
		
		if(EXTI_SuccessFlag)
		{
			EXTI_SuccessFlag = 0;
			printf("按键 EXTI 中断正常进行\r\n");
		}
		
		if(Key2_GetVal())
		{
			ADC_Value = Light_GetData();
			printf("当前采集到的 ADC 数值为：%d\r\n",ADC_Value);
			printf("对应当前亮度为：%d%%\r\n",(ADC_Value*100)/4096 );
			Delay_ms(200);
		}
		

	}
}

