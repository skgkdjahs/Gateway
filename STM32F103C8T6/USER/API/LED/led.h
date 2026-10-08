#ifndef __LED_H_
#define __LED_H_

#include "stm32f10x.h"

void LED_Config(void);
void KEY_Config(void);
#define	LINK(X)	(X)?(GPIOB->ODR |= (0X1<<4)):(GPIOB->ODR &= ~(0X1<<4))
#define	M0(X)		(X)?(GPIOB->ODR |= (0X1<<5)):(GPIOB->ODR &= ~(0X1<<5))
#define	M1(X)		(X)?(GPIOB->ODR |= (0X1<<6)):(GPIOB->ODR &= ~(0X1<<6))

#define	KEY	(GPIOB->IDR & (0X1<<7))


#endif

