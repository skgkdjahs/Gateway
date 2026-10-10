#include "led.h"

/*
函数功能：LED配置
函数作者：LDJ
备注：
PB4---LINK
PB5---M0
PB6---M1

*/
void LED_Config(void)
{
	//时钟配置
	RCC->APB2ENR |= (0X1<<3);//PB
	RCC->APB2ENR |= (0X1<<0);//AFIO
	AFIO->MAPR |= (0X2<<24);//关闭JTAG
	//GPIO配置
	GPIOB->CRL &= ~(0XFFF<<16);
	GPIOB->CRL |= (0X222<<16);//PB4/5/6通用推挽输出
	GPIOB->ODR |= (0X7<<4);

}

void KEY_Config(void)
{
	RCC->APB2ENR |= (0X1<<3);//PB
	GPIOB->CRL &= ~(0XFu<<28);
	GPIOB->CRL |= (0X4u<<28);//PB7通用浮空输入
	
}
