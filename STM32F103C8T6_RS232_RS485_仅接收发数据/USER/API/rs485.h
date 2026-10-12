#ifndef _RS485_H
#define _RS485_H

#include "stm32f10x.h"
#include "delay.h"
#include "string.h"
#include "stdio.h"
#include "led.h"

// 接收缓冲区大小
#define RS485RX_MAX 256

typedef struct{
	u8 RxBuf[RS485RX_MAX];//串口数据接收缓冲区
	u8 Rx_over;						//接收完成标志0 -- 没有接收完成 1 -- 接收完成
	u16 Rx_count;					//当前帧接收长度
	u8 Rx_error;					//1 表示当前帧超长
}RS485_TypeDef;

extern volatile RS485_TypeDef RS485GetData;

void RS485_USART3_Init(u32 brr);

void RS485_SendByte(u8 data);

void RS485_SendString(const char *str);

void RS485_SendBuff(u8 *data,u16 len);

/**
* @brief 接收PC调试助手发送的数据 并回显
*
* @return 返回1 正常 返回0 有误
*/
u8 RS485_ReceiveEcho(void);



#endif
