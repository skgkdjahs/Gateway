#include "Key.h"

extern uint8_t EXTI_SuccessFlag ;

void EXTI4_IRQHandler(void)
{
	if(EXTI->PR & (0x1 << 4))
	{
		EXTI->PR = (0x1 << 4);
		EXTI_SuccessFlag = 1;
	}
}

void Key_Init(void)
{
	// 初始化三个按键引脚：PE2\PE3\PE4，模式为上下拉，配置ODR为高位，即默认高电平
	RCC -> APB2ENR |= (0x1 << 6);
	RCC -> APB2ENR |= (0x1 << 0);//AFIO 时钟使能
	
	GPIOE -> CRL &= ~(0xfff << 8);
	GPIOE -> CRL |= (0x444 << 8);
	GPIOE->ODR |= (1<<2) | (1<<3) | (1<<4);
	
	// 对按键 KEY0 对应引脚 PE4 ，开启 EXTI 外部中断，作为按键中断的端口，后续可以按需再增加
	AFIO -> EXTICR[1] &= ~(0xf << 0);
	AFIO -> EXTICR[1] |= (0x4 << 0);
	
	// 配置边沿检测：按键默认高电压，使用上升沿触发保证按键动作完整进行再进入中断
	EXTI -> FTSR |= (0x1 << 4);
	// 中断屏蔽
	EXTI -> IMR |= (0x1 << 4);
	
	NVIC_SetPriority(EXTI4_IRQn,3);
	NVIC_EnableIRQ(EXTI4_IRQn);
}

uint8_t Key2_GetVal(void)
{
	// 对 Key2 按键的数据读取，对应引脚为 PE2 
	if ((GPIOE->IDR & (1 << 2)) == 0)
	{
		Delay_ms(20);
		if ((GPIOE->IDR & (1 << 2)) == 0)
		{
			while((GPIOE->IDR & (1 << 4)) == 0);
			return 1;
		}
	}
	else
	{
		return 0;
	}
	return 0;
}

uint8_t Key1_GetVal(void)
{
	// 对 Key1 按键的数据读取，对应引脚为 PE3 
	if ((GPIOE->IDR & (1 << 3)) == 0)
	{
		Delay_ms(20);
		if ((GPIOE->IDR & (1 << 3)) == 0)
		{
			while((GPIOE->IDR & (1 << 3)) == 0);
			return 1;
		}
	}
	else
	{
		return 0;
	}
	return 0;
}

uint8_t Key0_GetVal(void)
{
	// 对 Key0 按键的数据读取，对应引脚为 PE4 
	if ((GPIOE->IDR & (1 << 4)) == 0)
	{
		Delay_ms(20);
		if ((GPIOE->IDR & (1 << 4)) == 0)
		{
			while((GPIOE->IDR & (1 << 4)) == 0);
			return 1;
		}
	}
	else
	{
		return 0;
	}
	return 0;
}
