#include "stm32f10x.h"
#include "delay.h"
#include "led.h"

int main(void)
{
	LED_Config();
	while(1)
	{
		if(!KEY)
		{
			Delay_nms(100);
			if(!KEY)
			{
				LINK(1);
				M0(1);
				M1(1);
				Delay_nms(500);
				LINK(0);
				Delay_nms(800);
			}	
		}
	}
	
}
