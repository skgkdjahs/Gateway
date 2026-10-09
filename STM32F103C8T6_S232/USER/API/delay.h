#ifndef DELAY_H_
#define DELAY_H_

#include "stm32f10x.h"                  // Device header

/**
  *@brief 微秒级阻塞延时函数（nop 空跑实现）
  *       系统主频为 72MHz 时，一个 __nop() 执行时间为 1/72us，
  *       循环体内放置 72 个 __nop()，一次循环约耗时 1us
  *
  *@param uint32_t time 需要延时的微秒数
  */
void Delay_nus(uint32_t time);

/**
  *@brief 毫秒级阻塞延时（宏实现，内部调用 Delay_nus）
  *
  *@note  参数 x 过大时注意 x*1000 溢出问题（uint32_t 最大约 4294967ms）
  */
#define Delay_nms(x)	Delay_nus(x*1000)

#endif
