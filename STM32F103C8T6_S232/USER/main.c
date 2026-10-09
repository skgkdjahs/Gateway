#include "stm32f10x.h"
#include "delay.h"
#include "led.h"
#include "key.h"
#include "usart.h"

int main(void)
{
	NVIC_SetPriorityGrouping(NVIC_PriorityGroup_2);
	LED_Config();
	KEY_Config();
	USART1_Config(115200);
	while(1)
	{
		LED_M0_Toggle();
		Delay_nms(1000);
		LED_LINK_Toggle();
		Delay_nms(1000);
		LED_M1_Toggle();
		Delay_nms(1000);
		printf("11111\r\n");
	}
	
}
