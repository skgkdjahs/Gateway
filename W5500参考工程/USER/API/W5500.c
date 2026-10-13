#include "W5500.h"
#include "delay.h"
#include "usart.h"

/***************************************SPI配置，寄存器读写，串口打印测试***************************************/

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
		
    SPI_NSSInternalSoftwareConfig(SPI1, SPI_NSSInternalSoft_Set);
    SPI_Cmd(SPI1, ENABLE);
}

uint8_t W5500_SPI_ReadWriteByte(uint8_t data)
{
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
    SPI_I2S_SendData(SPI1, data);

    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
    return SPI_I2S_ReceiveData(SPI1);
}

void W5500_WriteReg(uint16_t address, uint8_t data)
{
    W5500_CS_LOW();
    W5500_SPI_ReadWriteByte((address >> 8) & 0xFF); // 地址高字节
    W5500_SPI_ReadWriteByte(address & 0xFF);        // 地址低字节
    W5500_SPI_ReadWriteByte(0x04);                  // Control Phase: Write, Common
    W5500_SPI_ReadWriteByte(data);
    W5500_CS_HIGH();
}

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
//void W5500_Reset(void)
//{
//    W5500_RST_LOW();
//    Delay_nms(10);
//    W5500_RST_HIGH();
//    Delay_nms(100);
//}

void W5500_Reset(void)
{
    // 硬件复位
    W5500_RST_LOW();
    Delay_nms(50);          // 拉低时间加长
    W5500_RST_HIGH();
    Delay_nms(200);         // 复位后等待时间加长

    // 软件复位 Mode Register
    W5500_WriteReg(0x0000, 0x80);   // MR = Soft Reset
    Delay_nms(10);
    W5500_WriteReg(0x0000, 0x00);   // 清除复位位
    Delay_nms(10);
}


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
		
		uint8_t phy = W5500_ReadReg(0x002E);
    USART1_Printf("PHYCFGR = 0x%02X (LNK=%d)\r\n", phy, phy & 0x01);
}


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

/***************************************实现 TCP Server，让电脑能连上板子并收发数据****************************************/

uint8_t W5500_ReadSocketReg(uint16_t address)
{
    uint8_t data;
    W5500_CS_LOW();
    W5500_SPI_ReadWriteByte((uint8_t)(address >> 8));
    W5500_SPI_ReadWriteByte((uint8_t)address);
    W5500_SPI_ReadWriteByte(0x08);          // Socket0 寄存器 + 读
    data = W5500_SPI_ReadWriteByte(0x00);
    W5500_CS_HIGH();
    return data;
}

void W5500_WriteSocketReg(uint16_t address, uint8_t data)
{
    W5500_CS_LOW();
    W5500_SPI_ReadWriteByte((uint8_t)(address >> 8));
    W5500_SPI_ReadWriteByte((uint8_t)address);
    W5500_SPI_ReadWriteByte(0x0C);          // Socket0 寄存器 + 写
    W5500_SPI_ReadWriteByte(data);
    W5500_CS_HIGH();
}


void W5500_WriteSocketBuf(uint16_t address, uint8_t *buf, uint16_t len)
{
    uint16_t i;
    W5500_CS_LOW();
    W5500_SPI_ReadWriteByte((uint8_t)(address >> 8));
    W5500_SPI_ReadWriteByte((uint8_t)address);
    W5500_SPI_ReadWriteByte(0x14);          // Socket0 TX Buffer + 写
    for (i = 0; i < len; i++)
    {
        W5500_SPI_ReadWriteByte(buf[i]);
    }
    W5500_CS_HIGH();
}

void W5500_ReadSocketBuf(uint16_t address, uint8_t *buf, uint16_t len)
{
    uint16_t i;
    W5500_CS_LOW();
    W5500_SPI_ReadWriteByte((uint8_t)(address >> 8));
    W5500_SPI_ReadWriteByte((uint8_t)address);
    W5500_SPI_ReadWriteByte(0x18);          // Socket0 RX Buffer + 读
    for (i = 0; i < len; i++)
    {
        buf[i] = W5500_SPI_ReadWriteByte(0x00);
    }
    W5500_CS_HIGH();
}


void W5500_Socket_Close(void)
{
    W5500_WriteSocketReg(W5500_S0_CR, W5500_CR_CLOSE);
    while (W5500_ReadSocketReg(W5500_S0_CR));   // 等待命令执行完成
}


uint8_t W5500_GetSocketStatus(void)
{
    return W5500_ReadSocketReg(W5500_S0_SR);
}


void W5500_Socket_TCP_Server(uint16_t port)
{
    W5500_Socket_Close();

    // 设置为 TCP 模式
    W5500_WriteSocketReg(W5500_S0_MR, 0x01);

    // 设置端口
    W5500_WriteSocketReg(W5500_S0_PORT0,     (port >> 8) & 0xFF);
    W5500_WriteSocketReg(W5500_S0_PORT0 + 1,  port & 0xFF);

    // 打开 Socket
    W5500_WriteSocketReg(W5500_S0_CR, W5500_CR_OPEN);
    while (W5500_ReadSocketReg(W5500_S0_CR));

    Delay_nms(5);

    if (W5500_GetSocketStatus() != W5500_SOCK_INIT)
    {
        USART1_Printf("Socket Open Failed!\r\n");
        return;
    }

    // 开始监听
    W5500_WriteSocketReg(W5500_S0_CR, W5500_CR_LISTEN);
    while (W5500_ReadSocketReg(W5500_S0_CR));

    USART1_Printf("TCP Server Listen on Port %d\r\n", port);
}

uint16_t W5500_GetTXFreeSize(void)
{
    uint16_t val = 0, val1 = 0;

    do
    {
        val1  = W5500_ReadSocketReg(W5500_S0_TX_FSR0);
        val1  = (val1 << 8) + W5500_ReadSocketReg(W5500_S0_TX_FSR0 + 1);

        if (val1 != 0)
        {
            val  = W5500_ReadSocketReg(W5500_S0_TX_FSR0);
            val  = (val << 8) + W5500_ReadSocketReg(W5500_S0_TX_FSR0 + 1);
        }
    } while (val != val1);

    return val;
}

uint16_t W5500_GetRXReceivedSize(void)
{
    uint16_t val = 0, val1 = 0;

    do
    {
        val1  = W5500_ReadSocketReg(W5500_S0_RX_RSR0);
        val1  = (val1 << 8) + W5500_ReadSocketReg(W5500_S0_RX_RSR0 + 1);

        if (val1 != 0)
        {
            val  = W5500_ReadSocketReg(W5500_S0_RX_RSR0);
            val  = (val << 8) + W5500_ReadSocketReg(W5500_S0_RX_RSR0 + 1);
        }
    } while (val != val1);

    return val;
}

void W5500_SendData(uint8_t *buf, uint16_t len)
{
    uint16_t offset;
    uint16_t free_size;

    // 等待发送缓冲区有足够空间
    do
    {
        free_size = W5500_GetTXFreeSize();
    } while (free_size < len);

    // 获取当前写指针
    offset  = W5500_ReadSocketReg(W5500_S0_TX_WR0);
    offset  = (offset << 8) + W5500_ReadSocketReg(W5500_S0_TX_WR0 + 1);

    // 写入数据到 TX Buffer
    W5500_WriteSocketBuf(offset, buf, len);

    // 更新写指针
    offset += len;
    W5500_WriteSocketReg(W5500_S0_TX_WR0,     (offset >> 8) & 0xFF);
    W5500_WriteSocketReg(W5500_S0_TX_WR0 + 1,  offset & 0xFF);

    // 发送命令
    W5500_WriteSocketReg(W5500_S0_CR, W5500_CR_SEND);
    while (W5500_ReadSocketReg(W5500_S0_CR));
}

uint16_t W5500_RecvData(uint8_t *buf, uint16_t maxlen)
{
    uint16_t rx_size;
    uint16_t offset;

    rx_size = W5500_GetRXReceivedSize();
    if (rx_size == 0) return 0;
    if (rx_size > maxlen) rx_size = maxlen;

    // 获取当前读指针
    offset  = W5500_ReadSocketReg(W5500_S0_RX_RD0);
    offset  = (offset << 8) + W5500_ReadSocketReg(W5500_S0_RX_RD0 + 1);

    // 从 RX Buffer 读取数据
    W5500_ReadSocketBuf(offset, buf, rx_size);

    // 更新读指针
    offset += rx_size;
    W5500_WriteSocketReg(W5500_S0_RX_RD0,     (offset >> 8) & 0xFF);
    W5500_WriteSocketReg(W5500_S0_RX_RD0 + 1,  offset & 0xFF);

    // 接收完成命令
    W5500_WriteSocketReg(W5500_S0_CR, W5500_CR_RECV);
    while (W5500_ReadSocketReg(W5500_S0_CR));

    return rx_size;
}

void W5500_Process(void)
{
    static uint8_t last_status = 0xFF;
    uint8_t status;
    uint8_t buf[256];
    uint16_t len;

    status = W5500_GetSocketStatus();

    // 状态发生变化时打印提示
    if (status != last_status)
    {
        last_status = status;

        switch (status)
        {
            case W5500_SOCK_CLOSED:
                USART1_Printf("[Socket] CLOSED → 重新打开\r\n");
                W5500_Socket_TCP_Server(5000);
                break;

            case W5500_SOCK_INIT:
                USART1_Printf("[Socket] INIT\r\n");
                break;

            case W5500_SOCK_LISTEN:
                USART1_Printf("[Socket] LISTEN (等待电脑连接...)\r\n");
                break;

            case W5500_SOCK_ESTABLISHED:
                USART1_Printf("[Socket] ESTABLISHED (电脑已连接)\r\n");
                break;

            case W5500_SOCK_CLOSE_WAIT:
                USART1_Printf("[Socket] CLOSE_WAIT → 关闭\r\n");
                W5500_Socket_Close();
                break;

            default:
                USART1_Printf("[Socket] Status = 0x%02X\r\n", status);
                break;
        }
    }

    // 已连接时处理数据回显
    if (status == W5500_SOCK_ESTABLISHED)
    {
        len = W5500_GetRXReceivedSize();
        if (len > 0)
        {
            len = W5500_RecvData(buf, sizeof(buf) - 1);
            buf[len] = '\0';

            // 串口打印收到的内容
            USART1_Printf("Recv(%d): %s\r\n", len, buf);

            // 回显给电脑
            W5500_SendData(buf, len);
            USART1_Printf("Echo OK\r\n");
        }
    }
}