//#include "stm32f10x.h"
//#include "delay.h"
//#include "led.h"
//#include "key.h"
//#include "usart.h"

//int main(void)
//{

//	while(1)
//	{

//	}
//	
//}
#include "stm32f10x.h"
#include "w5500.h"
#include "usart.h"

/* 假设你已有 USART1_Init 和 printf 重定向 */

int main(void)                                                                        
{
    u8 version;

    USART1_Config(115200);                                                                                                                                                                                                                                                                 
    W5500_SPIInit();
    W5500_RESET();

    version = W5500_VersionGet();

    printf("W5500 Version = 0x%02X\r\n", version);

    if(version == 0x04)
    {
        printf("W5500 SPI OK!\r\n");
    }
    else
    {
        printf("W5500 SPI ERROR!\r\n");
    }

    while(1)
    {
    }
}
