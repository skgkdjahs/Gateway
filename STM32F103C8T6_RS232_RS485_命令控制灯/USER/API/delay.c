#include "delay.h"

/**
  *@brief 微秒级阻塞延时函数
  *
  *@note  实现原理：CPU 空跑 nop 指令进行软件延时
  *       __nop() 的执行时间为一个指令周期，与系统主频相关
  *       系统主频为 72MHz 时，一个 __nop() 耗时 1/72us
  *       故循环体内放置 72 个 __nop()，一次循环约耗时 1us
  *       （未计入循环变量自减与跳转的开销，实际略大于 1us）
  *
  *@param uint32_t time 需要延时的微秒数
  */
void Delay_nus(uint32_t time)
{
	while(time--)
	{
		__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();
		__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();
		__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();
		__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();
		__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();
		__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();
		__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();
		__nop();__nop();
	}
}
