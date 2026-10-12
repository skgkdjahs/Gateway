#include "stm32f10x.h"
#include "delay.h"
#include "led.h"
#include "key.h"
#include "usart.h"
#include "rs485.h"


int main(void)
{
	u8 rxbuf[RS485RX_MAX];
	u16 len;
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
		RS485_ReceiveEcho();
		*/

		len = RS485_Receive(rxbuf,RS485RX_MAX-1);
		
		if(len > 0)
		{
			RS485_ProcessCommand(rxbuf, len);
		}

	}
	
}
