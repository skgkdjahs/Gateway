#include "stm32f10x.h"
#include "delay.h"
#include "led.h"
#include "key.h"
#include "usart.h"
#include "rs485.h"


int main(void)
{
	NVIC_SetPriorityGrouping(NVIC_PriorityGroup_2);
//	LED_Config();
//	KEY_Config();


	

	RS485_USART3_Init(115200);
	

	printf("11111\r\n");
	while(1)
	{
		/*
		≤‚ ‘ª˘¥°Õ‚…Ë
		LED_M0_Toggle();
		Delay_nms(1000);
		LED_LINK_Toggle();
		Delay_nms(1000);
		LED_M1_Toggle();
		Delay_nms(1000);

		*/
   RS485_ReceiveEcho();

	}
	
}
