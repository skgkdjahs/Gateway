#include "stm32f10x.h"
#include "delay.h"
#include "usart.h"
#include "W5500.h"

/*SPI配置，寄存器读写，串口打印测试
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
*/

/*设置网络参数并读取回来验证
int main(void)
{
    USART1_Config(115200);

    USART1_Printf("\r\n===== W5500 Network Test =====\r\n");

    W5500_Init();

    while (1)
    {
    }
}
*/


int main(void)
{
    USART1_Config(115200);

    USART1_Printf("\r\n===== W5500 TCP Echo Test =====\r\n");

    W5500_Init();                   // 初始化 + 设置网络参数
    W5500_Socket_TCP_Server(5000);  // 打开 TCP Server，监听 5000 端口

    while (1)
    {
        W5500_Process();            // 状态机 + 回显处理
    }
}