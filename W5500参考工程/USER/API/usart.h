#ifndef USART_H_
#define USART_H_

#include "stm32f10x.h"                  // Device header
#include "stdio.h"
#include "string.h"
#include "stdarg.h"

// 串口接收缓冲区大小
#define USART1_BUFFER_SIZE (256)

/*
 * 串口中断接收需要使用的结构体
 * 记录接收数据缓冲区、一帧数据接收完成标志位以及已接收数据长度
 * 收发流程：USART1 中断收到数据后写入 buffer，总线空闲（IDLE）时置位 flag，
 *          主循环检测到 flag == 1 后取出数据并处理，处理完成后将结构体整体清零
 */
typedef struct {
	uint8_t buffer[USART1_BUFFER_SIZE];	// 数据接收缓冲区
	uint8_t flag;						// 一帧数据接收完成标志位
	uint16_t count;						// 已接收数据字节数
} USARTRecvData;

extern USARTRecvData USART1_RecvData;

/**
  *@brief USART1 初始化，配置为中断接收方式
  *		 引脚：TX ---> PA9	复用推挽输出，默认输出高电平
  *		       RX ---> PA10	浮空输入，默认高电平
  *		 数据格式：8N1 协议（一位起始位、八位数据位、无校验位、一位停止位）
  *		 中断触发条件：接收数据非空（RXNE）、空闲线路（IDLE）
  *		 波特率：由参数指定
  *
  *@param uint32_t brr 需要的波特率，推荐 115200 或者 9600
  */
void USART1_Config(uint32_t brr);

/**
  *@brief 数据发送函数，一次发送 16 位数据
  *		 有效等同库函数中的 USART_SendData
  *
  *@param uint16_t Data 需要发送的数据，16 位长度字节长度
  */
void USART1_SendC(uint8_t data);

/**
  *@brief 单字节数据发送函数，一次发送一个字节
  *
  *@param uint8_t Byte 需要发送的字节，长度为一个字节长度
  */
void USART1_SendByte(uint8_t Byte);

/**
  *@brief 数组发送函数（定长方式，保留以兼容原有代码）
  *
  *@param uint8_t *data 需要发送的数组首地址
  *@param uint8_t len   需要发送的数组长度
  */
void USART1_SendStr(uint8_t* data, uint8_t len);

/**
  *@brief 字符串发送函数（以 '\0' 作为结束标志）
  *
  *@param char *String 需要发送的字符串首地址
  */
void USART1_SendString(char *String);

/**
  *@brief 缓冲区发送函数（16 位长度，适合发送大于 255 字节的数据）
  *
  *@param u8  *buffer 字节缓冲区首地址
  *@param u16 len     发送的缓冲区有效长度（字节数）
  */
void USART1_SendBuffer(u8 *buffer, u16 len);

/**
  *@brief 幂运算转换函数
  *		 将数字 Y 转换成 10 为底的幂，供数字转字符串发送时使用
  *
  *@param uint32_t X 底数（固定传入 10）
  *@param uint32_t Y 需要转换的幂次
  *
  *@return 转换后的数字，即 X 的 Y 次幂
  */
uint32_t USART1_PowU32(uint32_t X, uint32_t Y);

/**
  *@brief 数字发送函数
  *		 将数字按十进制逐位合成字符发送（如 1234 以 "1234" 形式发送）
  *
  *@param uint32_t Number 需要发送的数字
  *@param uint8_t  Length 需要发送数字的位数
  */
void USART1_SendNumber(uint32_t Number, uint8_t Length);

/**
  *@brief 格式化发送函数
  *		 使用方法与 printf 的格式化字符串一致
  *		 内部使用 vsnprintf，最多格式化 128 字节，超出部分被截断
  *
  *@param char *format 格式化字符串，以及可变参数
  */
void USART1_Printf(char *format, ...);

/**
  *@brief 阻塞式单字节接收函数（查询方式）
  *		 与中断接收方式二选一使用，不要同时使用
  *
  *@return uint8_t 接收到的字节
  */
uint8_t USART1_Recv(void);

#endif
