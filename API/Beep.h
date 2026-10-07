#ifndef BEEP_H_
#define BEEP_H_

/*
	蜂鸣器对应 PB8 引脚，同时也是 TIM_CH3
	高电平驱动
	后续可以使用定时器输出特定频率的音频放音乐
*/

#include "stm32f10x.h"                  // Device header

/**
  *@biref 蜂鸣器引脚初始化
  */
void Beep_Init(void);

void Beep_On(void);

void Beep_Off(void);



#endif
