#ifndef ADC_H_
#define ADC_H_

#include "stm32f10x.h"                  // Device header
#include "Delay.h"
/*
 *@brief 初始化 ADC3 
 *			PE8 引脚，为 ADC3_IN6 通道
 */
void ADC3_Init(void);

/*
 *@brief 得到当前光敏电阻的阻值
 *
 *@return 返回当前的光敏电阻阻值
 */
uint16_t Light_GetData(void);

#endif
