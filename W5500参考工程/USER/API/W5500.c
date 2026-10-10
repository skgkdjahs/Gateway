#include "W5500.h"
#include "delay.h"


void W5500_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    //使能 GPIOA 时钟 
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    // PA0：W5500 复位引脚，推挽输出
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // PA1：W5500 中断引脚，输入模式
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // PA4：W5500 片选引脚，推挽输出 
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 默认取消片选，复位引脚保持高电平 
    GPIO_SetBits(GPIOA, GPIO_Pin_4);
    GPIO_SetBits(GPIOA, GPIO_Pin_0);
}

void W5500_SPI1_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    SPI_InitTypeDef SPI_InitStructure;

    // 使能 GPIOA、SPI1 时钟 
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_SPI1,ENABLE);

    // PA5：SPI1_SCLK，复用推挽输出 
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // PA6：SPI1_MISO，浮空输入 
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // PA7：SPI1_MOSI，复用推挽输出 
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // SPI 基本配置 
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;

    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;

    // SPI 模式 0：CPOL=0，CPHA=0 
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;

    // 使用软件管理 NSS，PA4 手动控制 CS 
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;

    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_Init(SPI1, &SPI_InitStructure);

    // 使能 SPI1 
    SPI_Cmd(SPI1, ENABLE);
}

void W5500_Reset(void)
{
    // 取消片选
    GPIO_SetBits(GPIOA, GPIO_Pin_4);

    // 复位引脚拉低 
    GPIO_ResetBits(GPIOA, GPIO_Pin_0);
    Delay_nms(1);

    // 释放复位 
    GPIO_SetBits(GPIOA, GPIO_Pin_0);
    Delay_nms(10);
}


void W5500_Init(void)
{
    W5500_GPIO_Init();

    W5500_SPI1_Init();

    W5500_Reset();
}


uint8_t W5500_SPI_ReadWriteByte(uint8_t data)
{
    // 等待发送缓冲区为空 
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET)
    {
    }

    // 发送数据 
    SPI_I2S_SendData(SPI1, data);

    // 等待接收完成 
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET)
    {
    }

    // 读取并返回接收到的数据 
    return (uint8_t)SPI_I2S_ReceiveData(SPI1);
}


uint8_t W5500_ReadReg(uint16_t address)
{

}


void W5500_WriteReg(uint16_t address, uint8_t data)
{

}




