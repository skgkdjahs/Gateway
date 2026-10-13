#ifndef __W5500_H
#define __W5500_H

#include "stm32f10x.h"


/***************************************SPI配置，寄存器读写，串口打印测试***************************************/

// W5500 控制引脚
#define W5500_RST_PORT      GPIOA
#define W5500_RST_PIN       GPIO_Pin_0

#define W5500_INT_PORT      GPIOA
#define W5500_INT_PIN       GPIO_Pin_1

#define W5500_CS_PORT       GPIOA
#define W5500_CS_PIN        GPIO_Pin_4

// 片选控制
#define W5500_CS_LOW()      GPIO_ResetBits(W5500_CS_PORT, W5500_CS_PIN)
#define W5500_CS_HIGH()     GPIO_SetBits(W5500_CS_PORT, W5500_CS_PIN)

// 复位控制
#define W5500_RST_LOW()     GPIO_ResetBits(W5500_RST_PORT, W5500_RST_PIN)
#define W5500_RST_HIGH()    GPIO_SetBits(W5500_RST_PORT, W5500_RST_PIN)

// 硬件初始化
void W5500_GPIO_Init(void);
void W5500_SPI1_Init(void);
void W5500_Reset(void);
void W5500_Init(void);

// SPI 收发一个字节
uint8_t W5500_SPI_ReadWriteByte(uint8_t data);

// W5500 寄存器读写（Common 寄存器）
uint8_t W5500_ReadReg(uint16_t address);
void W5500_WriteReg(uint16_t address, uint8_t data);

/***************************************设置网络参数并读取回来验证****************************************/

// 网络参数（根据你电脑实际网段修改） 
#define W5500_GATEWAY   192,168,5,1
#define W5500_SUBNET    255,255,255,0
#define W5500_MAC       0x00,0x08,0xDC,0x11,0x22,0x33
#define W5500_IP        192,168,5,105

void W5500_WriteBuf(uint16_t address, uint8_t *buf, uint16_t len);
void W5500_ReadBuf(uint16_t address, uint8_t *buf, uint16_t len);
void W5500_SetNetwork(void);
void W5500_PrintNetwork(void);

/***************************************实现 TCP Server，让电脑能连上板子并收发数据****************************************/

// Socket 寄存器偏移（Socket 0）
#define W5500_S0_MR         0x0000
#define W5500_S0_CR         0x0001
#define W5500_S0_IR         0x0002
#define W5500_S0_SR         0x0003
#define W5500_S0_PORT0      0x0004
#define W5500_S0_TX_FSR0    0x0020
#define W5500_S0_TX_RD0     0x0022
#define W5500_S0_TX_WR0     0x0024
#define W5500_S0_RX_RSR0    0x0026
#define W5500_S0_RX_RD0     0x0028

// Socket 命令 
#define W5500_CR_OPEN       0x01
#define W5500_CR_LISTEN     0x02
#define W5500_CR_CLOSE      0x10
#define W5500_CR_SEND       0x20
#define W5500_CR_RECV       0x40

// Socket 状态
#define W5500_SOCK_CLOSED      0x00
#define W5500_SOCK_INIT        0x13
#define W5500_SOCK_LISTEN      0x14
#define W5500_SOCK_ESTABLISHED 0x17
#define W5500_SOCK_CLOSE_WAIT  0x1C

// Socket 相关函数声明
void     W5500_Socket_Close(void);
uint8_t  W5500_GetSocketStatus(void);
void     W5500_Socket_TCP_Server(uint16_t port);
uint16_t W5500_GetTXFreeSize(void);
uint16_t W5500_GetRXReceivedSize(void);
void     W5500_SendData(uint8_t *buf, uint16_t len);
uint16_t W5500_RecvData(uint8_t *buf, uint16_t maxlen);
void     W5500_Process(void);

#endif
