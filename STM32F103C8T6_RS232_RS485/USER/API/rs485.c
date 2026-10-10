#include "rs485.h"

/* RS232GetData 接收数据结构体 */
volatile RS485_TypeDef RS485GetData = {0};

void RS485_USART3_Init(u32 brr)
{
	/*
	PB10 → USART3_TX 复用推挽输出
	PB11 → USART3_RX 浮空输入

	PB3 → RS_CHANGE → 74HC4053 → RS232/RS485
	*/
	GPIO_InitTypeDef GPIO_InitStructure; 
	USART_InitTypeDef USART_InitStructure;
	
	/*
	1.打开时钟
	*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
	
	/*
	2.GPIO端口配置
	*/
	//PB10 → USART3_TX 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	
	//PB11 → USART3_RX
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	/*
	3.USART3模块配置
	*/
	USART_InitStructure.USART_BaudRate = brr; 
	USART_InitStructure.USART_WordLength = USART_WordLength_8b; 
	USART_InitStructure.USART_StopBits = USART_StopBits_1; 
	USART_InitStructure.USART_Parity = USART_Parity_No; 
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; 
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	USART_Init(USART3, &USART_InitStructure);
	
	/*清空接收状态 */
	RS485GetData.Rx_count = 0;
	RS485GetData.Rx_over = 0;
	/*
	4.开启中断 RXNE IDLE
	*/
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
	USART_ITConfig(USART3, USART_IT_IDLE, ENABLE);
	
	/*
	5.配置中断NVIC
	*/
	NVIC_SetPriority(USART3_IRQn,2);
	NVIC_EnableIRQ(USART3_IRQn);
	
	
	//USART模块使能
	USART_Cmd(USART3,ENABLE);
}



void USART3_IRQHandler(void)
{
	u8 data =0;
	volatile u32 temp;
	
	//RXNE 接收到一个字节
	if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
	{
		 data = USART_ReceiveData(USART3);
		
		//只有上一帧已被主循环处理，才继续存入新数据
		if (RS485GetData.Rx_over == 0)
		{
			if (RS485GetData.Rx_count < RS485RX_MAX)
			{
				RS485GetData.RxBuf[RS485GetData.Rx_count++] = (u8)data;
			}
			else
			{
				/* 缓冲区已满，丢弃后续数据 */
			}
		}
	}
	//IDLE：线路空闲，标记当前帧接收结束
	if (USART_GetITStatus(USART3, USART_IT_IDLE) != RESET)
	{
	/*
		* STM32F1 清除 IDLE 标志的规定操作：
    * 先读 SR，再读 DR。
  */
	temp = USART3->SR;
  temp = USART3->DR;
	(void)temp;
		
	if ((RS485GetData.Rx_count > 0) && (RS485GetData.Rx_over == 0))
   {
      RS485GetData.Rx_over = 1;
   }
	}

}
/*
发送一个字节
*/
void RS485_SendByte(u8 data)
{
	while (USART_GetFlagStatus( USART3, USART_FLAG_TXE) == RESET);
	
	USART_SendData(USART3, data);
}

void RS485_SendString(const char *str)
{
	while (*str != '\0') 
	{ 
		RS485_SendByte((uint8_t)*str); 
		str++; 
	} 
	/* 等待最后一个字节发送完成 */ 
	while (USART_GetFlagStatus( USART3, USART_FLAG_TC) == RESET);
	
}

void RS485_SendBuff(u8 *data,u16 len)
{
	u16 i;
	
	 for (i = 0; i < len; i++)
    {
        RS485_SendByte(data[i]);
    }
    /* 等待最后一个字节发送完成 */
    while (USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET);
	
}

u8 RS485_ReceiveEcho(void)
{
	u16 len;
	
	if (RS485GetData.Rx_over == 1)
  {
        /*
         * 读取本帧长度
         */
        len = (u16)RS485GetData.Rx_count;

        /*
         * 原样回显收到的数据
         */
        RS485_SendBuff((u8 *)RS485GetData.RxBuf, len);

        /*
         * 处理完成，清除接收状态
         */
        RS485GetData.Rx_count = 0;
        RS485GetData.Rx_over = 0;

        return 1;
  }

    return 0;
}


//printf 重定向
int fputc(int c, FILE *stream)
{
	while(USART_GetFlagStatus(USART3,USART_FLAG_TC) == RESET);//等待上一次数据发送完成
	USART_SendData(USART3,c);
	return c;
}
