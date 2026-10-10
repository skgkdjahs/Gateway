#ifndef __RS485_H_
#define __RS485_H_
#include "stm32f10x.h"
#include "stdio.h"
#include "string.h"
#include "zigbee.h"

void USART3_Config(uint32_t brr);
void USART3_Sendchar(uint8_t c);
void USART3_Sendstring(uint8_t* str,int len);
uint8_t USART3_Recvchar(void);
uint8_t RS485_Add(uint8_t *data,uint16_t len);


#endif
