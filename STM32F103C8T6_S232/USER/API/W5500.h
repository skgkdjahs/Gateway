#ifndef _W5500_H
#define _W5500_H

#include "stm32f10x.h"                  
#include "stdio.h"
#include "string.h"


/*
仅用来调试SPI通信正常
	SPI通信收发完成
	W5500复位正常
	W5500输出版本号
*/
#define TCP_RST   GPIO_Pin_0
#define TCP_INT		GPIO_Pin_1

#define TCP_CS		GPIO_Pin_4
#define TCP_SCLK	GPIO_Pin_5
#define TCP_MISO	GPIO_Pin_6
#define	TCP_MOSI  GPIO_Pin_7


/**
* @brief SPI初始化
*/
void W5500_SPIInit(void);

/**
* @brief 复位W5500
*/
void W5500_RESET(void);

/*
获取当前W5500的版本号
*/
u8 W5500_VersionGet(void);


#endif
