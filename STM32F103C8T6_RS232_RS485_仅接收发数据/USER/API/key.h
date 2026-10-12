#ifndef KEY_H_
#define KEY_H_

#include "stm32f10x.h"                  // Device header
#include "delay.h"

/*
 * 引脚分配（以当前工程实际使用引脚为准）：
 * PB7 --- KEY	按键输入引脚（配置为浮空输入）
 *				按键电路为低电平有效：按下时引脚被拉为低电平
 */

/**
  *@brief 按键初始化
  */
void KEY_Config(void);

/**
  *@brief 读取按键引脚原始电平（不消抖，适用于轮询判断引脚状态）
  *
  *@return uint8_t 1 = 引脚为高电平；0 = 引脚为低电平
  */
uint8_t KEY_GetState(void);

/**
  *@brief 读取按键值（软件消抖，阻塞等待按键松开后返回）
  *		 检测到有效按键动作返回 1，未按下返回 0
  *
  *@return uint8_t 1 = 检测到一次有效按键；0 = 无按键
  */
uint8_t KEY_GetVal(void);

/*
 * 按键电平读取宏（与 KEY_GetState 等效，保留以兼容原有代码）
 * 用法：if (KEY == 0) 表示按键按下
 */
#define	KEY	(GPIOB->IDR & (0X1<<7))

#endif
