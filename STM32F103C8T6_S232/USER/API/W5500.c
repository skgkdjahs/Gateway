#include "W5500.h"


/* 简单延时：用于复位时序 */
static void W5500_Delay(volatile u32 count)
{
    while(count--)
    {
    __nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();
		__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();
		__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();
		__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();
		__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();
		__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();
		__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();
		__nop();__nop();
    }
}

/**
 * @brief SPI初始化
 * @note  使用 SPI1，软件控制 CS，SPI Mode 0
 */
void W5500_SPIInit(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    SPI_InitTypeDef SPI_InitStructure;

    /* 1. 开启 GPIOA 和 SPI1 时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |
                           RCC_APB2Periph_SPI1, ENABLE);

    /* 2. PA0、PA4 配置为推挽输出 */
    GPIO_InitStructure.GPIO_Pin = TCP_RST | TCP_CS;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* 默认不复位，取消片选 */
    GPIO_SetBits(GPIOA, TCP_RST | TCP_CS);

    /* 3. PA1 配置为浮空输入：W5500 中断引脚 */
    GPIO_InitStructure.GPIO_Pin = TCP_INT;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* 4. PA5、PA7 配置为复用推挽输出 */
    GPIO_InitStructure.GPIO_Pin = TCP_SCLK | TCP_MOSI;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* 5. PA6 配置为浮空输入 */
    GPIO_InitStructure.GPIO_Pin = TCP_MISO;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* 6. 配置 SPI1 */
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;

    /* SPI Mode 0：空闲时钟低电平，第一个时钟边沿采样 */
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;

    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;

    /* 高位先发 */
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;

    /* SPI1 时钟 = 72MHz / 16 = 4.5MHz（假设 APB2 为 72MHz） */
    SPI_InitStructure.SPI_BaudRatePrescaler =
        SPI_BaudRatePrescaler_16;

    SPI_InitStructure.SPI_CRCPolynomial = 7;

    SPI_Init(SPI1, &SPI_InitStructure);

    /* 7. 使能 SPI1 */
    SPI_Cmd(SPI1, ENABLE);
}

/**
 * @brief 复位 W5500
 * @note  W5500 的 RSTn 为低电平有效
 */
void W5500_RESET(void)
{
    /* 保证 CS 未选中 */
    GPIO_SetBits(GPIOA, TCP_CS);

    /* 拉低复位引脚 */
    GPIO_ResetBits(GPIOA, TCP_RST);
    W5500_Delay(100000);

    /* 释放复位 */
    GPIO_SetBits(GPIOA, TCP_RST);
    W5500_Delay(1000000);
}

/**
 * @brief SPI发送并接收一个字节
 * @param data 要发送的数据
 * @return 接收到的数据；超时返回 0xFF
 */
static u8 W5500_SPI_ReadWriteByte(u8 data)
{
    u32 timeout = 0x10000;

    /* 等待发送缓冲区空 */
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET)
    {
        if(--timeout == 0)
            return 0xFF;
    }

    /* 发送一个字节 */
    SPI_I2S_SendData(SPI1, data);

    timeout = 0x10000;

    /* 等待接收数据 */
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET)
    {
        if(--timeout == 0)
            return 0xFF;
    }

    return (u8)SPI_I2S_ReceiveData(SPI1);
}

/**
 * @brief 获取 W5500 版本号
 * @return 正常应返回 0x04
 */
u8 W5500_VersionGet(void)
{
    u16 version=0;

    /* 选中 W5500 */
    GPIO_ResetBits(GPIOA, TCP_CS);

    /* 地址高字节：0x00 */
    W5500_SPI_ReadWriteByte(0x00);

    /* 地址低字节：0x39 */
    W5500_SPI_ReadWriteByte(0x39);

    /*
     * 控制字节：
     * BSB = 00000：Common Register
     * RWB = 1：读操作
     * OM  = 00：变长数据模式
     * 因此控制字节 = 0x04
     */
    W5500_SPI_ReadWriteByte(0x04);

    /* 读取版本寄存器 */
    version = W5500_SPI_ReadWriteByte(0xFF);

    /* 取消片选 */
    GPIO_SetBits(GPIOA, TCP_CS);

    return version;
}

