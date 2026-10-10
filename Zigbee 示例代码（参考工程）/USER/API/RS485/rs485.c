#include "rs485.h"

 
uint8_t recvstr_485[255]={0};
int count_485=0;
/*
函数功能：串口3初始化
作者：LDJ
备注：PB10----TX----复用推挽
			PB11----RX----浮空输入
			PB3-----RS_CHANGE----低电平选择485，高电平选择232接口
*/
void USART3_Config(uint32_t brr)
{
	//打开时钟
	RCC->APB2ENR |= (0X1<<3);//PB
	RCC->APB2ENR |= (0X1<<0);//AFIO
	AFIO->MAPR |= (0X2<<24);//AFIO
	RCC->APB1ENR |= (0X1<<18);//USART3
	
	//GPIO配置
	//PB3
	GPIOB->CRL &= ~(0XF<<12);
	GPIOB->CRL |= (0X1<<12);
	GPIOB->ODR &= ~(0X1<<3);
	
	//PB10
	GPIOB->CRH &= ~(0XF<<8);
	GPIOB->CRH |= (0XB<<8);
	//PB11
	GPIOB->CRH &= ~(0XF<<12);
	GPIOB->CRH |= (0X4<<12);
	
	//USART3配置
	USART3->CR1 &= ~(0X1<<12);//一个起始位， 8个数据位， n个停止位；
	USART3->CR1 &= ~(0X1<<10);//禁止校验控制
	USART3->CR1 |= (0X3<<2);//收发使能
	USART3->CR1 |= (0x3<<4);//使能接收中断/空闲中断
	USART3->CR2 &= ~(0x3<<12);//1个停止位；
	
	//波特率
	float DIV,DIV_Fra;
	int DIV_Man;
	DIV = 36000000.0/(brr*16);
	DIV_Man = (int)DIV;
	DIV_Fra = DIV-DIV_Man;
	USART3->BRR = (DIV_Man<<4) | (int)(DIV_Fra*16);
	//NVIC
	NVIC_SetPriority(USART3_IRQn,0);
	NVIC_EnableIRQ(USART3_IRQn);
	
	USART3->CR1 |= (0X1<<13);//USART3使能


}
/*
函数功能：串口一发送一个字符
作者：LDJ
备注：PA9----TX----复用推挽
			PA10---RX----浮空输入
*/
void USART3_Sendchar(uint8_t c)
{
	while(!(USART3->SR & (0X1<<6)));
	USART3->DR = c;
//	while(USART_GetFlagStatus(USART3,USART_FLAG_TC)==RESET);
//	USART_SendData(USART3,c);
}
	
/*
函数功能：串口一发送一个字符串
作者：LDJ
备注：PA2----TX----复用推挽
			PA3----RX----浮空输入
*/

void USART3_Sendstring(uint8_t* str,int len)
{
	for(int i=0;i<len;i++)
	{
		USART3_Sendchar(str[i]);
	}
}

/*
函数功能：串口一接收一个字符
作者：LDJ
备注：PA2----TX----复用推挽
			PA3----RX----浮空输入
*/
uint8_t Temp3;
uint8_t USART3_Recvchar(void)
{
	while(!(USART3->SR & (0X1<<5)));
	return USART3->DR;	
//	while(USART_GetFlagStatus(USART3,USART_FLAG_RXNE)==RESET);
//	return USART_ReceiveData(USART3);
}

/****************************
函数名称：RS232_Add
函数作用：ZigBee和校验
函数入口：
	data	缓冲区地址
	len	数据长度
函数出口：和校验结果
函数作者：硬件-王玉川
创建时间：2021.08.11 10:44
修改时间：2021.08.11 10:44
****************************/
uint8_t RS485_Add(uint8_t *data,uint16_t len)
{
	uint32_t Add_Temp = 0,i = 0;
	for(i=0;i<len;i++)
		Add_Temp += data[i];
	return (uint8_t)(Add_Temp&0XFF);
}





int fputc(int c, FILE *stream)
{
	USART3_Sendchar(c);
	return c;
}



int TMP3;
void USART3_IRQHandler(void)
{
	if(USART3->SR & (0X1<<5))//接收中断
	{
		USART3->SR &= ~(0x1<<5);//清中断
		recvstr_485[count_485++] = USART3->DR;
	}
	if(USART3->SR & (0X1<<4))//空闲中断
	{
		//清中断
		TMP3 = USART3->SR;
		TMP3 = USART3->DR;
		printf("%s\r\n",recvstr_485);
		ZigBee_Sendbuf(recvstr_485,count_485);
		count_485=0;
		memset(recvstr_485,0,255);
	}	
}



