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


	
LED_Config();
	RS485_USART3_Init(115200);
	

	printf("RS485正常\r\n");
	while(1)
	{
		/*
		测试基础外设
		LED_M0_Toggle();
		Delay_nms(1000);
		LED_LINK_Toggle();
		Delay_nms(1000);
		LED_M1_Toggle();
		Delay_nms(1000);

		*/
   /*
		用于测试 PC调试助手 接收发数据
		RS485_ReceiveEcho();
		*/

		RS485_ProcessCommand();

	}
	
}
