#include "stm32f10x.h"
#include "delay.h"
#include "led.h"
#include "key.h"
#include "usart.h"
#include "rs485.h"


int main(void)
{
	NVIC_SetPriorityGrouping(NVIC_PriorityGroup_2);



	
	LED_Config();
	RS485_USART3_Init(115200);
	

	printf("RS485正常\r\n");
	while(1)
	{

  /*
		用于测试是否能解析灯的命令并执行相关操作
		*/
		
		RS485_ProcessCommand();


	}
	
}
