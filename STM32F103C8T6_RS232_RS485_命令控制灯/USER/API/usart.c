#include "usart.h"

USARTRecvData USART1_RecvData = {0};

/**
  *@brief USART1 初始化，配置为中断接收方式
  *
  *@note  引脚分配（以当前工程实际使用引脚为准）：
  *       PA9 ---- TX ---- 复用推挽输出
  *       PA10 --- RX ---- 浮空输入
  *       数据格式：8N1（8 位数据位、无校验位、1 位停止位）
  *       开启 RXNE（读数据寄存器非空）与 IDLE（空闲线路）中断，
  *       配合 USART1_RecvData 结构体实现不定长数据帧接收
  *
  *@param uint32_t brr 需要的波特率，如 115200、9600
  */
void USART1_Config(uint32_t brr)
{
	float DIV=0,DIV_F=0;
	int DIV_M=0;
	// 时钟使能（APB2ENR：bit2 = GPIOA，bit14 = USART1）
	RCC->APB2ENR |= (0X1<<2);
	RCC->APB2ENR |= (0X1<<14);
	// GPIO 配置（CRH 的 bit4~bit7 对应 PA9，bit8~bit11 对应 PA10）
	// PA9（TX）：MODE=11（输出 50MHz）、CNF=10（复用推挽输出），即 0xB
	// PA10（RX）：MODE=00（输入模式）、CNF=01（浮空输入），即 0x4
	GPIOA->CRH &= ~(0XFF<<4);
	GPIOA->CRH |= (0X4B<<4);
	// USART 配置
	USART1->CR1 &= ~(0X1<<12);// M 位清零，选择 8 位数据字长
	USART1->CR1 &= ~(0X1<<10);// PCE 位清零，禁止奇偶校验
	USART1->CR1 |= (0X3<<4);// 置位 RXNEIE、IDLEIE，打开接收非空与空闲线路中断
	USART1->CR1 |= (0X3<<2);// 置位 TE、RE，使能发送与接收
	USART1->CR2 &= ~(0x3<<12);// STOP[1:0] = 00，选择 1 位停止位
	// NVIC 配置
	NVIC_SetPriority(USART1_IRQn,0);// 指定中断处理源为 USART1，抢占优先级 0、子优先级 0
	NVIC_EnableIRQ(USART1_IRQn);// 使能 USART1 中断通道

	// 波特率计算
	// USARTDIV = fPCLK2 / (波特率 * 16)
	// BRR 寄存器：bit11~bit0 为整数部分 DIV_Mantissa，bit3~bit0 为小数部分 DIV_Fraction
	DIV = 72000000.0/(brr*16);
	DIV_M = (int)DIV;
	DIV_F = DIV-DIV_M;
	USART1->BRR = DIV_M<<4 | (int)(DIV_F*16);

	// 置位 UE 位，使能 USART1（建议放在配置的最后一步）
	USART1->CR1 |= (0X1<<13);
}

/**
  *@brief USART1 发送一个字节
  *
  *@note  发送流程：先等待 TXE = 1（发送数据寄存器为空）再写入 DR，
  *       最后等待 TC = 1（最后一帧数据完全发送完毕），
  *       保证函数返回时数据已经真正发送出去
  *
  *@param uint8_t data 需要发送的字节
  */
void USART1_SendC(uint8_t data)
{
	// 等待 TXE = 1：发送数据寄存器为空，可以写入下一个字节
	while(!(USART1->SR & (0X1<<7)));
	USART1->DR = data;
	// 等待 TC = 1：发送移位寄存器也已完成移位，本字节发送完毕
	while(!(USART1->SR & (0X1<<6)));
}

/**
  *@brief 单字节数据发送函数，与 USART1_SendC 等效
  *		 提供模板风格的函数命名，便于统一调用
  *
  *@param uint8_t Byte 需要发送的字节
  */
void USART1_SendByte(uint8_t Byte)
{
	USART1_SendC(Byte);
}

/**
  *@brief 数组发送函数（定长方式）
  *
  *@param uint8_t *data 需要发送的数组首地址
  *@param uint8_t len   需要发送的数组长度
  */
void USART1_SendStr(uint8_t* data,uint8_t len)
{
	while(len--)
	{
		USART1_SendC(*data);
		data++;
	}

}

/**
  *@brief 字符串发送函数（以 '\0' 作为结束标志）
  *
  *@param char *String 需要发送的字符串首地址
  */
void USART1_SendString(char *String)
{
	while(*String != '\0')
	{
		USART1_SendC((uint8_t)*String);
		String++;
	}
}

/**
  *@brief 缓冲区发送函数（16 位长度，适合发送大于 255 字节的数据）
  *
  *@param u8  *buffer 字节缓冲区首地址
  *@param u16 len     发送的缓冲区有效长度（字节数）
  */
void USART1_SendBuffer(u8 *buffer, u16 len)
{
	while (len--)
	{
		USART1_SendC(*buffer);
		buffer++;
	}
}

/**
  *@brief 幂运算转换函数，将数字 Y 转换成 X 为底的幂
  *
  *@param uint32_t X 底数
  *@param uint32_t Y 幂次
  *
  *@return 转换后的数字，即 X 的 Y 次幂
  */
uint32_t USART1_PowU32(uint32_t X, uint32_t Y)
{
	uint32_t result = 1;
	while(Y--)
	{
		result *= X;
	}
	return result ;
}

/**
  *@brief 数字发送函数，将数字按十进制逐位转换成字符发送
  *		 如 USART1_SendNumber(1234, 4) 将发送字符串 "1234"
  *
  *@param uint32_t Number 需要发送的数字
  *@param uint8_t  Length 需要发送数字的位数
  */
void USART1_SendNumber(uint32_t Number, uint8_t Length)
{
	uint8_t i;
	uint8_t temp = 0;
	for(i = 0; i < Length; i++)
	{
		// 从高位到低位逐位取出数字并转换成 ASCII 字符发送
		temp = Number / USART1_PowU32(10, Length - i -1 ) % 10 + '0' ;
		USART1_SendC(temp);
	}
}

/**
  *@brief 格式化发送函数，使用方法与 printf 一致
  *
  *@note  内部使用 vsnprintf 格式化到局部缓冲区（最多 128 字节），
  *       相比直接重定向 printf 更安全可控
  *
  *@param char *format 格式化字符串，后面跟可变参数
  */
void USART1_Printf(char *format, ...)
{
	char String[128];
	va_list arg;
	va_start(arg, format);
	vsnprintf(String, sizeof(String), format, arg);
	va_end(arg);
	USART1_SendString(String);
}

/**
  *@brief 阻塞式单字节接收函数（查询方式）
  *
  *@note  与中断接收方式二选一使用，不要同时使用，
  *       否则接收的数据可能被查询函数取走导致中断缓冲区丢字节
  *
  *@return uint8_t 接收到的字节
  */
uint8_t USART1_Recv(void)
{
	// 阻塞等待 RXNE = 1：接收数据寄存器非空
	while(!(USART1->SR & (0X1<<5)));//等待接收数据
	return USART1->DR;// 读 DR 的同时会自动清除 RXNE 标志

}

/**
  *@brief printf 字符重定向，将 printf 输出定向到 USART1
  *
  *@note  使用 printf 前需要先调用 USART1_Config 完成初始化；
  *       MDK 工程需勾选 MicroLIB 或包含 stdio 支持库
  */
//int fputc(int a,FILE* file)
//{
//	USART1_SendC(a);
//	return a;// 按照标准规定返回写入的字符

//}

/**
  *@brief USART1 中断服务函数（不定长数据帧接收）
  *
  *@note  接收逻辑：
  *       1. RXNE = 1：收到一个字节，写入接收缓冲区并计数
  *       2. IDLE = 1 ：总线空闲一个字节时间，判定一帧数据接收完成，
  *          添加字符串结束符并置位 flag，等待主循环处理
  *       主循环中的推荐用法：
  *           if (USART1_RecvData.flag)
  *           {
  *               // 处理 USART1_RecvData.buffer，长度 USART1_RecvData.count
  *               memset(&USART1_RecvData, 0, sizeof(USARTRecvData));// 处理完成后清空，继续接收下一帧
  *           }
  *       确认一帧接收完成后，在中断内直接调用发送函数将收到的内容原样回显；
  *       回显为阻塞式发送，如不需要可删除对应三行即可
  */
void USART1_IRQHandler(void)
{
	//memset(USART1_RecvData.buffer,0,USART1_BUFFER_SIZE);
	// RXNE = 1：收到一个字节，中断是否由接收数据寄存器非空触发
	if(USART1->SR & (0X1<<5))
	{
		USART1->SR &= ~(0X1<<5);// 清除接收数据寄存器非空中断标志
		// 若不清除该标志，中断会一直不停地触发，导致主程序无法执行

		// 预留最后一个字节存放字符串结束符 '\0'，防止缓冲区越界
		if(USART1_RecvData.count < USART1_BUFFER_SIZE - 1)
		{
			USART1_RecvData.buffer[USART1_RecvData.count++]=USART1->DR;
		}
		else
		{
			(void)USART1->DR;// 缓冲区已满，读 DR 丢弃该字节并清除标志
		}
	}
	// IDLE = 1：总线空闲一个字节时间，判定为一帧数据接收完成
	if(USART1->SR & (0X1<<4))
	{

		// 清除空闲中断的标志：需要先读 SR 再读 DR 才能清除
		// 若不清除该标志，中断会一直不停地触发，导致主程序无法执行
		(void)USART1->SR;
		(void)USART1->DR;

		USART1_RecvData.buffer[USART1_RecvData.count] = '\0';// 添加字符串结束符，方便主循环直接 printf("%s")
		USART1_RecvData.flag = 1;// 置位接收完成标志，等待主循环取走数据并处理

		// 接收完成回显：直接使用发送函数把收到的内容原样发回
		USART1_SendString("USART1 Recv Echo : ");
		USART1_SendBuffer(USART1_RecvData.buffer, USART1_RecvData.count);
		USART1_SendString("\r\n");

	}


}
