#include "usart.h"


 
/*
函数功能：USART1配置
函数作者：LDJ
备注：
PA9----TX----复用推挽输出----0xA
PA10---RX----浮空输入----0X4
*/
void USART1_Config(uint32_t brr)
{
	float DIV=0,DIV_F=0;
	int DIV_M=0;
	//时钟使能
	RCC->APB2ENR |= (0X1<<2);
	RCC->APB2ENR |= (0X1<<14);
	//GPIO配置
	GPIOA->CRH &= ~(0XFF<<4);
	GPIOA->CRH |= (0X4B<<4);
	//USART1配置
	USART1->CR1 &= ~(0X1<<12);//选择8位字长
	USART1->CR1 &= ~(0X1<<10);//禁止奇偶校验
	USART1->CR1 |= (0X3<<4);//允许产生接收和空闲中断
	USART1->CR1 |= (0X3<<2);//收发使能
	USART1->CR2 &= ~(0x3<<12);//1个停止位
	//NVIC配置
	NVIC_SetPriority(USART1_IRQn,0);//指定中断触发源为USART1，同时指定优先级：抢占0，次级0
	NVIC_EnableIRQ(USART1_IRQn);//使能USART1的中断
	
	//波特率设置
	DIV = 72000000.0/(brr*16);
	DIV_M = (int)DIV;
	DIV_F = DIV-DIV_M;
	USART1->BRR = DIV_M<<4 | (int)(DIV_F*16);
//	USART1->BRR = 72000000.0/brr;
	
	USART1->CR1 |= (0X1<<13);//USART1使能
}
/*
函数功能：USART1发送一个字节
函数作者：LDJ
备注：
*/
void USART1_SendC(uint8_t data)
{
	while(!(USART1->SR & (0X1<<6)));//等待上次发送完成
	USART1->DR = data;
}
/*
函数功能：USART1发送字符串
函数作者：LDJ
备注：
*/
void USART1_SendStr(uint8_t* data,uint8_t len)
{
	while(len--)
	{
		USART1_SendC(*data);
		data++;
	}

}


/*
函数功能：USART1接收一个字节
函数作者：LDJ
备注：
*/
uint8_t USART1_Recv(void)
{
	while(!(USART1->SR & (0X1<<5)));//等待接收完成
	return USART1->DR;

}



uint8_t USART1_DATA[255]={0};
uint8_t usart1_cnt=0,temp=0;
void USART1_IRQHandler(void)
{
	if(USART1->SR & (0X1<<5))//判断是否发生接收数据寄存器非空中断
	{
		USART1->SR &= ~(0X1<<5);//清除接收数据寄存器非空中断标志
		//若不清除标志，则中断会一直不停的触发，导致主进程无法执行
		USART1_DATA[usart1_cnt++]=USART1->DR;
	}
	if(USART1->SR & (0X1<<4))//判断是否发生空闲中断
	{
		
		//清除空闲中断的标志
		temp=USART1->SR;
		temp=USART1->DR;
		//若不清除标志，则中断会一直不停的触发，导致主进程无法执行
//		USART1_SendStr(USART1_DATA,usart1_cnt);//查看我收到的全部数据
		printf("%s\r\n",USART1_DATA);
		memset(USART1_DATA,0,usart1_cnt);
		usart1_cnt=0;
	}

}

//int fputc(int a,FILE* file)
//{
//	USART1_SendC(a);
//	return 0;
//	
//}
