#include "stm32f10x.h"
#include "delay.h"
#include "usart.h"
#include "rs485.h"
#include "zigbee.h"
#include "led.h"


int main(void)
{
	NVIC_SetPriorityGrouping(5);//中断优先级分组，抢占和次级各占2位	
	USART1_Config(9600);
	ZigBee_Config(115200);
	USART3_Config(9600);
	KEY_Config();
	LED_Config();
	printf("初始化完成\r\n");
	ZigBeeAddDevice();//添加设备到zigee网络
	while(1)
	{

		if(!KEY)
		{
			Delay_nms(200);
			if(!KEY)
			{
				ZigBee_LempCtrl(Z_DeviceLib,0x02);//切换开关状态
				printf("mode change\r\n");
			}
		}
	}
	
	
}
