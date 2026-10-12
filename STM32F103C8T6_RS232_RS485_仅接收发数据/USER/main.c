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
		用于测试 PC调试助手 接收发数据
		打开串口 发送一串数据 RS485 回显到串口调试助手上
		*/
		RS485_ReceiveEcho();

	}
	
}
