#ifndef __DELAY_H_
#define __DELAY_H_

#include "stm32f10x.h"
void Delay_nus(uint32_t time);

#define Delay_nms(x)	Delay_nus(x*1000)

#endif


