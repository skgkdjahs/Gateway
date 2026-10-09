#include "led.h"

/**
  *@brief 输出引脚初始化
  *
  *@note  引脚分配（以当前工程实际使用引脚为准）：
  *       PB4 --- LINK	（默认复用为 JTAG 的 NJTRST 引脚）
  *       PB5 --- M0
  *       PB6 --- M1
  *       均配置为通用推挽输出，初始化完成后默认输出低电平
  */
void LED_Config(void)
{
	// 时钟使能（APB2ENR：bit3 = GPIOB，bit0 = AFIO）
	RCC->APB2ENR |= (0X1<<3);//PB
	RCC->APB2ENR |= (0X1<<0);//AFIO
	// SWJ_CFG[2:0] = 010：关闭 JTAG-DP、使能 SW-DP
	// PB4 默认为 JTAG 的 NJTRST 引脚，必须先关闭 JTAG 才能作为普通 IO 使用
	AFIO->MAPR |= (0X2<<24);//关闭JTAG
	// 配置 PB4/5/6 为通用推挽输出
	// CRL：MODE=10（输出 2MHz）、CNF=00（通用推挽输出），每个引脚占 4 位
	// 0X222<<16 即 bit16~bit27，对应 PB4/5/6
	GPIOB->CRL &= ~(0XFFF<<16);
	GPIOB->CRL |= (0X222<<16);//PB4/5/6通用推挽输出
	// 初始化完成后默认输出低电平（ODR 的 bit4~bit6 对应 PB4/5/6）
	GPIOB->ODR &= ~(0X7<<4);

}

/**
  *@brief LINK 引脚（PB4）输出高电平
  */
void LED_LINK_On(void)
{
	GPIOB->ODR |= (0x1 << 4);
}

/**
  *@brief LINK 引脚（PB4）输出低电平
  */
void LED_LINK_Off(void)
{
	GPIOB->ODR &= ~(0x1 << 4);
}

/**
  *@brief LINK 引脚（PB4）电平翻转
  */
void LED_LINK_Toggle(void)
{
	GPIOB->ODR ^= (0x1 << 4);
}

/**
  *@brief M0 引脚（PB5）输出高电平
  */
void LED_M0_On(void)
{
	GPIOB->ODR |= (0x1 << 5);
}

/**
  *@brief M0 引脚（PB5）输出低电平
  */
void LED_M0_Off(void)
{
	GPIOB->ODR &= ~(0x1 << 5);
}

/**
  *@brief M0 引脚（PB5）电平翻转
  */
void LED_M0_Toggle(void)
{
	GPIOB->ODR ^= (0x1 << 5);
}

/**
  *@brief M1 引脚（PB6）输出高电平
  */
void LED_M1_On(void)
{
	GPIOB->ODR |= (0x1 << 6);
}

/**
  *@brief M1 引脚（PB6）输出低电平
  */
void LED_M1_Off(void)
{
	GPIOB->ODR &= ~(0x1 << 6);
}

/**
  *@brief M1 引脚（PB6）电平翻转
  */
void LED_M1_Toggle(void)
{
	GPIOB->ODR ^= (0x1 << 6);
}
