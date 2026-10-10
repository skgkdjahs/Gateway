#ifndef __USART_H_
#define __USART_H_

 
#include "stm32f10x.h"
#include "stdio.h"
#include "string.h"


void USART1_Config(uint32_t brr);
void USART1_SendC(uint8_t data);
void USART1_SendStr(uint8_t* data,uint8_t len);
void USART1_SendStr1(uint8_t* data);
uint8_t USART1_Recv(void);
#endif



