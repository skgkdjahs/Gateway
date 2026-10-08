#ifndef SERIAL_H_
#define SERIAL_H_

#include "stm32f10x.h"                  // Device header
#include "stdio.h"
#include "string.h"
#include "stdarg.h"

#define USART1_BUFFER_SIAE (256)

// 接收数据需要使用的结构体，包含临时缓冲区、接收标志位、接收数据长度
typedef struct {
	uint8_t buffer[USART1_BUFFER_SIAE] ;
	uint8_t flag ;
	uint16_t count ;
} USARTRecvData;

//extern USARTRecvData USART1_RecvData = {0};

//// 状态机写法的 中断接收所使用的临时缓冲区
//char USART1_RxPacket[USART1_BUFFER_SIAE];	
//// 状态机写法的 接收标志位
//uint8_t USART1_RxFlag;

/*
 *@brief USART1 初始化，接收为中断接收
 *		 引脚：TX--->PA9  使用复用推挽输出，默认输出高电平  
 *			   RX--->PA10 使用上拉输入，默认输出高电平
 *		 数据格式：8N1协议
 *  			  一个起始位、八个数据位、无校验位、一位停止位
 *		 波特率：可调
 *		 中断触发标志位：数据接收区非空、空闲时间
 *				数据接收区非空触发中断，进入数据接收状态
 *				空闲时间触发中断，进入接收完成等待状态
 *		 中断等级：两位抢占优先级，两位子优先级
 *
 *@param uint32_t BRR 需要的波特率，推荐 115200 或者 9600
 */
void USART1_Init(uint32_t BRR);

/*
 *@brief 数据发送函数，一次性发送两个字节
 *		 等效于库函数中的 USART_SendData 函数
 *
 *@param uint16_t Data 被发送的数据,16 位，两字节长度
 */
void USART1_SendData(uint16_t Data);

/*
 *@brief 单字节数据发送函数，一次性发送一个字节
 *
 *@param uint8_t Byte 被发送的字节，八位，一字节长度
 */
void USART1_SendByte(uint8_t Byte);

/*
 *@brief 数组发送函数
 *
 *@param uint8_t *Array 被传输的数组地址
 *@param uint16_t Length 被传输的数组长度
 */
void USART1_SendArray(uint16_t *Array, uint16_t Length);

/*
 *@brief 字符串发送函数
 *
 *@param char *String 被发送的字符串首地址
 */
void USART1_SendString(char *String);

/*
 *@brief 进制转换函数
 *		 将数字 Y 转换成进制为 X 进制的数字进行输出
 *		 辅助函数，配合数字发送函数使用
 *
 *@param uint32_t X 结果的数字进制
 *@param uint32_t Y 需要转换的数字
 *
 *@return 转换后的数字，按照指定进制进行数字排列
 */
uint32_t USART1_PowU32(uint32_t X, uint32_t Y);

/*
 *@brief 数字发送函数
 *		 将任意进制、符合长度要求的数字，以十进制形式输出
 *
 *@param uint32_t Number 被发送的数字
 *@param uint8_t Length 被发送数字的长度
 */
void USART1_SendNumber(uint32_t Number, uint8_t Length);

/*
 *@brief 串口输出函数
 *		 类似于 printf 的格式化串口输出函数
 *
 *@param char *format 格式化字符串，输出的实际内容，可能存在较多的占位符，实际内容是后续的参数
 */
void USART1_Printf(char *format, ...);

/**
 * @brief MCU 通过 USART1 模块发送字节缓冲区内容
 * 
 * @param buffer 字节缓冲区首地址
 * @param len    发送的缓冲区有效数据字节数
 */
void USART1_SendBuffer(u8 *buffer, u16 len);

#endif

