#include "stm32f10x.h"
#include "delay.h"
#include "usart.h"
#include "W5500.h"
#include <stdio.h>

int main(void)
{
    uint8_t version;
    char message[64] = {0};

    /* 初始化 USART1，波特率 9600 */
    USART1_Config(9600);

    Delay_nms(100);

    USART1_SendString("\r\nW5500 SPI Test Start\r\n");

    /* 初始化 W5500 */
    W5500_Init();

    Delay_nms(10);

    /* VERSIONR 公共寄存器地址为 0x0039 */
    version = W5500_ReadReg(0x0039);

    sprintf(message,
            "W5500 VERSIONR = 0x%02X\r\n",
            (unsigned int)version);

    USART1_SendString(message);


    if ((uint8_t)version == 04)
    {
        USART1_SendString("W5500 SPI OK!\r\n");
    }
    else
    {
        USART1_SendString("W5500 SPI CHECK FAILED!\r\n");
    }

    while (1)
    {
    }
}
