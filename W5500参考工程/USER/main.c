#include "stm32f10x.h"
#include "delay.h"
#include "usart.h"
#include "W5500.h"

int main(void)
{
    USART1_Config(115200);

    USART1_Printf("\r\n===== W5500 SPI Test =====\r\n");

    W5500_Init();

    while (1)
    {
        // 空循环，只看串口打印结果
    }
}