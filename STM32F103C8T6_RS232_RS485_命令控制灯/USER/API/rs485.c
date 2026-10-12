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
				/* 缓冲区溢出，标记错误 */

			}
		}
		/* 如果已有完整帧等待处理，则读取但丢弃新字节 */
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
		RS485_SendByte((u8)*str); 
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


void RS485_ProcessCommand(void)
{
    u32 len;

    if (RS485GetData.Rx_over != 1)
    {
        return;
    }

    len = RS485GetData.Rx_count;

    /* 确保缓冲区长度合法 */
    if (len >= RS485RX_MAX)
    {
        RS485_SendString("CMD TOO LONG\r\n");

        RS485GetData.Rx_count = 0;
        RS485GetData.Rx_over = 0;
        return;
    }

    /* 添加字符串结束符 */
    RS485GetData.RxBuf[len] = '\0';

    /* 删除末尾的回车、换行 */
    while (len > 0)
    {
        if (RS485GetData.RxBuf[len - 1] == '\r' ||
            RS485GetData.RxBuf[len - 1] == '\n')
        {
            RS485GetData.RxBuf[len - 1] = '\0';
            len--;
        }
        else
        {
            break;
        }
    }
		
		RS485_SendString("RX: ");
		RS485_SendBuff((u8 *)RS485GetData.RxBuf, (u16)len);
		RS485_SendString("\r\n");
		
     /* LINK 灯 */
    if (strcmp((char *)RS485GetData.RxBuf,
               "LED_LINK_ON") == 0)
    {
        LED_LINK_On();
        RS485_SendString("LED_LINK ON OK\r\n");
    }
    else if (strcmp((char *)RS485GetData.RxBuf,
                    "LED_LINK_OFF") == 0)
    {
        LED_LINK_Off();
        RS485_SendString("LED_LINK OFF OK\r\n");
    }
    else if (strcmp((char *)RS485GetData.RxBuf,
                    "LED_LINK_TOGGLE") == 0)
    {
        LED_LINK_Toggle();
        RS485_SendString("LED_LINK TOGGLE OK\r\n");
    }

    /* M0 灯 */
    else if (strcmp((char *)RS485GetData.RxBuf,
                    "LED_M0_ON") == 0)
    {
        LED_M0_On();
        RS485_SendString("LED_M0 ON OK\r\n");
    }
    else if (strcmp((char *)RS485GetData.RxBuf,
                    "LED_M0_OFF") == 0)
    {
        LED_M0_Off();
        RS485_SendString("LED_M0 OFF OK\r\n");
    }
    else if (strcmp((char *)RS485GetData.RxBuf,
                    "LED_M0_TOGGLE") == 0)
    {
        LED_M0_Toggle();
        RS485_SendString("LED_M0 TOGGLE OK\r\n");
    }

    /* M1 灯 */
    else if (strcmp((char *)RS485GetData.RxBuf,
                    "LED_M1_ON") == 0)
    {
        LED_M1_On();
        RS485_SendString("LED_M1 ON OK\r\n");
    }
    else if (strcmp((char *)RS485GetData.RxBuf,
                    "LED_M1_OFF") == 0)
    {
        LED_M1_Off();
        RS485_SendString("LED_M1 OFF OK\r\n");
    }
    else if (strcmp((char *)RS485GetData.RxBuf,
                    "LED_M1_TOGGLE") == 0)
    {
        LED_M1_Toggle();
        RS485_SendString("LED_M1 TOGGLE OK\r\n");
    }
    else
    {
        RS485_SendString("UNKNOWN CMD\r\n");
    }

    /* 处理完毕，允许接收下一帧 */
    RS485GetData.Rx_count = 0;
    RS485GetData.Rx_over = 0;
}


//printf 重定向
int fputc(int c, FILE *stream)
{
	while(USART_GetFlagStatus(USART3,USART_FLAG_TC) == RESET);//等待上一次数据发送完成
	USART_SendData(USART3,c);
	return c;
}
