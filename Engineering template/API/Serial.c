#include "Serial.h"

USARTRecvData USART1_RecvData = {0};

void USART1_Init(uint32_t BRR)
{
	// GPIOA 时钟
	RCC->APB2ENR |= (0x1 << 2);
	// USART1 时钟
	RCC->APB2ENR |= (0x1 << 14);
	
	// PA9 配置：复用推挽输出，默认高电平
	GPIOA->ODR |= (0x1 << 9);
	GPIOA->CRH &= ~(0xF << 4);
	GPIOA->CRH |= (0xB << 4);
	
	// PA10 配置：上拉输入，默认高电平
	GPIOA->ODR |= (0x1 << 10);
	GPIOA->CRH &= ~(0xF << 8);
	GPIOA->CRH |= (0x8 << 8);
	
	// 配置字长：一个起始位，八个数据位，n个停止位
	USART1->CR1 &= ~(0x1 << 12);
	// 配置校验位：无校验位
	USART1->CR1 &= ~(0x1 << 10);
	// 发送使能
	USART1->CR1 |= (0x1 << 3);
	// 接收使能
	USART1->CR1 |= (0x1 << 2);
	// 停止位选择
	USART1->CR2 &= ~(0x3 << 12);
	
	// 配置波特率
	USART1->BRR = 72000000/BRR;
	
	
//	// 中断优先级分组，确定优先级寄存器分区：两位抢占优先级、两位子优先级
//	NVIC_SetPriorityGrouping(5);
	// 抢占优先级 3 ，次级优先级 2 --> 1110 --> 14(十进制) --> 0xE
	NVIC_SetPriority(USART1_IRQn,14);
	// NVIC->IP[(uint32_t)(IRQn)] = ((priority << (8 - __NVIC_PRIO_BITS)) & 0xff);
	// 告诉NVIC USART1配置了中断优先级
	NVIC_EnableIRQ(USART1_IRQn);
	// NVIC->ISER[((uint32_t)(IRQn) >> 5)] = (1 << ((uint32_t)(IRQn) & 0x1F));
	
	// 打开接收缓冲区非空中断
	USART1->CR1 |= (0x1 << 5);
	// 打开空闲时间中断
	USART1->CR1 |= (0x1 << 4);
	
	// 使能 UE 激活USART1
	USART1->CR1 |= (0x1 << 13);
}

void USART1_SendData(uint16_t Data)
{
	// 等待 TXE 发送数据寄存器空 标志位
	while(!(USART1->SR & (0x1<<7))); 
	USART1->DR = (Data & (uint16_t)0x01FF);
	// 等待 TC 发送完成 标志位
	while(!(USART1->SR & (0x1<<6))); 
}	

void USART1_SendByte(uint8_t Byte)
{
	USART1_SendData((uint16_t)Byte);
}	

void USART1_SendArray(uint16_t *Array, uint16_t Length)
{
	uint16_t i;
	for (i = 0; i < Length ; i++)
	{
		USART1_SendByte(Array[i]);
	}
}

void USART1_SendString(char *String)
{
	uint8_t i;
	for(i = 0; String[i] != '\0'; i++)
	{
		USART1_SendByte(String[i]);
	}
}

uint32_t USART1_PowU32(uint32_t X, uint32_t Y)
{
	uint32_t result = 1;
	while(Y--)
	{
		result *= X;
	}
	return result ; 
}

void USART1_SendNumber(uint32_t Number, uint8_t Length)
{
	uint8_t i;
	uint8_t temp = 0;
	for(i = 0; i < Length; i++)
	{
		temp = Number / USART1_PowU32(10, Length - i -1 ) % 10 + '0' ;
		USART1_SendByte(temp);
	}
}	

// 输出重定向，把 printf 函数的输出方向切换为串口输出
int fputc(int ch, FILE *f)
{
	USART1_SendByte(ch);
	return ch;
}

void USART1_Printf(char *format, ...)
{
	char String[128];
	va_list arg;
	va_start(arg, format);
//	vsprintf(String, format, arg);
	vsnprintf(String, sizeof(String), format, arg);
	va_end(arg);
	USART1_SendString(String);
}

void USART1_SendBuffer(u8 *buffer, u16 len) 
{
	while (len--)
	{
		USART1_SendByte(*buffer);
		buffer++;
	}
}

void USART1_IRQHandler(void)
{
	
	if(USART1_RecvData.flag)
	{
		memset(&USART1_RecvData, 0, sizeof(USARTRecvData));
	}
	
	if(USART1 -> SR & (0x01 << 4))
	{
		(void)USART1->SR;
		(void)USART1->DR;
		
		USART1_RecvData.flag = 1;
		USART1_Printf("串口接收功能正常，接收的数据为：\r\n");
		USART1_SendBuffer(USART1_RecvData.buffer, USART1_RecvData.count);
		USART1_SendString("\r\n");
	}
	
	if(USART1->SR & (0x01 << 5))
	{
		USART1_RecvData.buffer[USART1_RecvData.count] = USART1->DR;
		USART1_RecvData.count ++;
		
		if(USART1_BUFFER_SIAE == USART1_RecvData.count)
		{
			USART1_RecvData.flag = 1;
			// 【临时处理】
			USART1_SendString("USART1 Recv Data : ");
			USART1_SendBuffer(USART1_RecvData.buffer, USART1_RecvData.count);
			USART1_SendString("\r\n");
		}
	}
}


/*
	一个状态机写法的串口中断接收函数
	实现逻辑为：
		对接收的数据进行格式上的锁死：必须以 '@' 字符为开始，'\r\n' 为结尾
		基于上面严格的格式，对数据进行读取（不过可以使用串口调试助手在发送数据时直接自动加上上述内容，其实不麻烦）
*/
//void USART1_IRQHandler(void)
//{
//	static uint8_t RxState = 0;
//	static uint8_t pRxPacket = 0;
//	if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET)
//	{
//		uint8_t RxData = USART_ReceiveData(USART1);
//		
//		if (RxState == 0)
//		{
//			if (RxData == '@' && USART1_RxFlag == 0)
//			{
//				RxState = 1;
//				pRxPacket = 0;
//			}
//		}
//		else if (RxState == 1)
//		{
//			if (RxData == '\r')
//			{
//				RxState = 2;
//			}
//			else
//			{
//				USART1_RxPacket[pRxPacket] = RxData;
//				pRxPacket ++;
//			}
//		}
//		else if (RxState == 2)
//		{
//			if (RxData == '\n')
//			{
//				RxState = 0;
//				USART1_RxPacket[pRxPacket] = '\0';
//				USART1_RxFlag = 1;
//			}
//		}
//		
//		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
//	}
//}
