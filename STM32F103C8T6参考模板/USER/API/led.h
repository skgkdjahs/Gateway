#ifndef LED_H_
#define LED_H_

#include "stm32f10x.h"                  // Device header

/*
 * 引脚分配（以当前工程实际使用引脚为准）：
 * PB4 --- LINK		输出引脚（默认复用为 JTAG 的 NJTRST，初始化时已关闭 JTAG 释放）
 * PB5 --- M0		输出引脚
 * PB6 --- M1		输出引脚
 */

/**
  *@brief 输出引脚初始化
  *		 初始化完成后 LINK/M0/M1 默认输出低电平
  */
void LED_Config(void);

/**
  *@brief LINK（PB4）输出控制
  */
void LED_LINK_On(void);
void LED_LINK_Off(void);
void LED_LINK_Toggle(void);

/**
  *@brief M0（PB5）输出控制
  */
void LED_M0_On(void);
void LED_M0_Off(void);
void LED_M0_Toggle(void);

/**
  *@brief M1（PB6）输出控制
  */
void LED_M1_On(void);
void LED_M1_Off(void);
void LED_M1_Toggle(void);

/*
 * 电平控制宏（与上面的函数等效，保留以兼容原有代码）
 * 用法：LINK(1) 输出高电平；LINK(0) 输出低电平
 */
#define	LINK(X)	((X)?(GPIOB->ODR |= (0X1<<4)):(GPIOB->ODR &= ~(0X1<<4)))
#define	M0(X)	((X)?(GPIOB->ODR |= (0X1<<5)):(GPIOB->ODR &= ~(0X1<<5)))
#define	M1(X)	((X)?(GPIOB->ODR |= (0X1<<6)):(GPIOB->ODR &= ~(0X1<<6)))

#endif
