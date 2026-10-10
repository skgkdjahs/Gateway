#ifndef __W5500_H
#define __W5500_H

#include "stm32f10x.h"

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

#endif
