#include "Beep.h"


void Beep_Init(void)
{
	// 初始化引脚 PB8，模式设置为上下拉模式，修改输出寄存器为默认低电平
	RCC -> APB2ENR |= (0x1 << 3);
	GPIOB -> CRH &= ~(0xf);
	GPIOB -> CRH |= (0x1);
	GPIOB -> ODR &= ~(0x1 << 8);
}

void Beep_On(void)
{
	GPIOB -> ODR |= (0x1 << 8);
}

void Beep_Off(void)
{
	GPIOB -> ODR &= ~(0x1 << 8);
}

