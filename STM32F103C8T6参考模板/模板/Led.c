#include "Led.h"


void Led_Init(void)
{
	// 初始化完成后引脚默认写高，LED 不亮
	
	// 初始化引脚 PB5 
	RCC -> APB2ENR |= (0x1 << 3);
	GPIOB -> CRL &= ~(0xf << 20);
	GPIOB -> CRL |= (0x3 << 20);
	GPIOB -> ODR |= (0x1 << 5);
	
	// 初始化引脚 PE5 
	RCC -> APB2ENR |= (0x1 << 6);
	GPIOE -> CRL &= ~(0xf << 20);
	GPIOE -> CRL |= (0x3 << 20);
	GPIOE -> ODR |= (0x1 << 5);
}

void Led_DS0_On(void)
{
	GPIOB -> ODR &= ~(0x1 << 5);
}

void Led_DS0_Off(void)
{
	GPIOB -> ODR |= (0x1 << 5);
}

void Led_DS1_On(void)
{
	GPIOE -> ODR &= ~(0x1 << 5);
}

void Led_DS1_Off(void)
{
	GPIOE -> ODR |= (0x1 << 5);
}

