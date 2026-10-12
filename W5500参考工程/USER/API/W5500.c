#include "W5500.h"
#include "delay.h"
#include "usart.h"

/***************************************SPI配置，寄存器读写，串口打印测试***************************************/

/**
  * @brief  W5500 相关 GPIO 初始化
  *         PA0: RST  推挽输出
  *         PA1: INT  浮空输入
  *         PA4: CS   推挽输出
  */
void W5500_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    // PA0 (RST)、PA4 (CS) 推挽输出
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // PA1 (INT) 浮空输入
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    W5500_CS_HIGH();
    W5500_RST_HIGH();
}

/**
  * @brief  SPI1 初始化
  *         PA5: SCK
  *         PA6: MISO
  *         PA7: MOSI
  */
void W5500_SPI1_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    SPI_InitTypeDef  SPI_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_SPI1, ENABLE);

    // PA5/PA7 复用推挽，PA6 浮空输入
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8; // 9MHz
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_Init(SPI1, &SPI_InitStructure);

    SPI_Cmd(SPI1, ENABLE);
}

/**
  * @brief  SPI 收发一个字节
  */
uint8_t W5500_SPI_ReadWriteByte(uint8_t data)
{
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
    SPI_I2S_SendData(SPI1, data);

    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
    return SPI_I2S_ReceiveData(SPI1);
}

/**
  * @brief  写 Common 寄存器
  */
void W5500_WriteReg(uint16_t address, uint8_t data)
{
    W5500_CS_LOW();
    W5500_SPI_ReadWriteByte((address >> 8) & 0xFF); // 地址高字节
    W5500_SPI_ReadWriteByte(address & 0xFF);        // 地址低字节
    W5500_SPI_ReadWriteByte(0x04);                  // Control Phase: Write, Common
    W5500_SPI_ReadWriteByte(data);
    W5500_CS_HIGH();
}

/**
  * @brief  读 Common 寄存器
  */
uint8_t W5500_ReadReg(uint16_t address)
{
    uint8_t data;
    W5500_CS_LOW();
    W5500_SPI_ReadWriteByte((address >> 8) & 0xFF);
    W5500_SPI_ReadWriteByte(address & 0xFF);
    W5500_SPI_ReadWriteByte(0x00);                  // Control Phase: Read, Common
    data = W5500_SPI_ReadWriteByte(0x00);
    W5500_CS_HIGH();
    return data;
}

/**
  * @brief  硬件复位 W5500
  */
void W5500_Reset(void)
{
    W5500_RST_LOW();
    Delay_nms(10);
    W5500_RST_HIGH();
    Delay_nms(100);
}

/**
  * @brief  W5500 初始化（目前只做通信测试）
  */
//void W5500_Init(void)
//{
//    uint8_t version;

//    W5500_GPIO_Init();
//    W5500_SPI1_Init();
//    W5500_Reset();

//    version = W5500_ReadReg(0x0039);  // VERSIONR 寄存器

//    USART1_Printf("W5500 Version = 0x%02X\r\n", version);

//    if (version == 0x04)
//    {
//        USART1_Printf("W5500 SPI Communication OK!\r\n");
//    }
//    else
//    {
//        USART1_Printf("W5500 SPI Communication Failed!\r\n");
//    }
//}

/***************************************设置网络参数并读取回来验证****************************************/


void W5500_Init(void)
{
    uint8_t version;

    W5500_GPIO_Init();
    W5500_SPI1_Init();
    W5500_Reset();

    version = W5500_ReadReg(0x0039);
    USART1_Printf("W5500 Version = 0x%02X\r\n", version);

    if (version != 0x04)
    {
        USART1_Printf("W5500 SPI Communication Failed!\r\n");
        return;
    }

    USART1_Printf("W5500 SPI Communication OK!\r\n");

    // 设置网络参数
    W5500_SetNetwork();
    USART1_Printf("Network parameters set.\r\n");

    // 读回验证
    W5500_PrintNetwork();
}


/**
  * @brief  写多个字节到 Common 寄存器
  */
void W5500_WriteBuf(uint16_t address, uint8_t *buf, uint16_t len)
{
    uint16_t i;

    W5500_CS_LOW();
    W5500_SPI_ReadWriteByte((uint8_t)(address >> 8));
    W5500_SPI_ReadWriteByte((uint8_t)address);
    W5500_SPI_ReadWriteByte(0x04);          // 写操作

    for (i = 0; i < len; i++)
    {
        W5500_SPI_ReadWriteByte(buf[i]);
    }
    W5500_CS_HIGH();
}

/**
  * @brief  从 Common 寄存器读多个字节
  */
void W5500_ReadBuf(uint16_t address, uint8_t *buf, uint16_t len)
{
    uint16_t i;

    W5500_CS_LOW();
    W5500_SPI_ReadWriteByte((uint8_t)(address >> 8));
    W5500_SPI_ReadWriteByte((uint8_t)address);
    W5500_SPI_ReadWriteByte(0x00);          // 读操作

    for (i = 0; i < len; i++)
    {
        buf[i] = W5500_SPI_ReadWriteByte(0x00);
    }
    W5500_CS_HIGH();
}

/**
  * @brief  设置网络参数
  */
void W5500_SetNetwork(void)
{
    uint8_t gateway[4] = {W5500_GATEWAY};
    uint8_t subnet[4]  = {W5500_SUBNET};
    uint8_t mac[6]     = {W5500_MAC};
    uint8_t ip[4]      = {W5500_IP};

    W5500_WriteBuf(0x0001, gateway, 4);   // GAR  网关
    W5500_WriteBuf(0x0005, subnet,  4);   // SUBR 子网掩码
    W5500_WriteBuf(0x0009, mac,     6);   // SHAR MAC
    W5500_WriteBuf(0x000F, ip,      4);   // SIPR IP
}

/**
  * @brief  读取并打印网络参数
  */
void W5500_PrintNetwork(void)
{
    uint8_t buf[6];

    W5500_ReadBuf(0x000F, buf, 4);
    USART1_Printf("IP  : %d.%d.%d.%d\r\n", buf[0], buf[1], buf[2], buf[3]);

    W5500_ReadBuf(0x0001, buf, 4);
    USART1_Printf("GW  : %d.%d.%d.%d\r\n", buf[0], buf[1], buf[2], buf[3]);

    W5500_ReadBuf(0x0005, buf, 4);
    USART1_Printf("MASK: %d.%d.%d.%d\r\n", buf[0], buf[1], buf[2], buf[3]);

    W5500_ReadBuf(0x0009, buf, 6);
    USART1_Printf("MAC : %02X:%02X:%02X:%02X:%02X:%02X\r\n",
                  buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
}

