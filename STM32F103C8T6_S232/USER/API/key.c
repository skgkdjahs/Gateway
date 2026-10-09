#include "key.h"

/**
  *@brief 按键初始化
  *
  *@note  按键引脚为 PB7，配置为浮空输入
  *       CRL：MODE=00（输入模式）、CNF=01（浮空输入）
  *       若按键电路为"按键接地且无外部上拉电阻"，
  *       建议将 CNF 由 0x4 改为 0x8（上拉输入），硬件无需改动
  */
void KEY_Config(void)
{
	// 时钟使能（APB2ENR：bit3 = GPIOB）
	RCC->APB2ENR |= (0X1<<3);//PB
	// 配置 PB7 为浮空输入（CRL 的 bit28~bit31 对应 PB7）
	GPIOB->CRL &= ~(0XF<<28);
	GPIOB->CRL |= (0X4<<28);//PB7浮空输入

}

/**
  *@brief 读取按键引脚原始电平（不消抖）
  *
  *@return uint8_t 1 = 引脚为高电平；0 = 引脚为低电平
  */
uint8_t KEY_GetState(void)
{
	return ((GPIOB->IDR & (0X1<<7)) != 0);
}

/**
  *@brief 读取按键值（软件消抖，阻塞式）
  *
  *@note  消抖流程：检测到引脚为低电平（按键按下）后
  *       延时 20ms 再次确认，滤除按下瞬间的机械抖动；
  *       确认有效后阻塞等待按键松开，避免一次按压被重复识别
  *
  *@return uint8_t 1 = 检测到一次有效按键；0 = 无按键
  */
uint8_t KEY_GetVal(void)
{
	// 按键电路低电平有效：按下时引脚被拉为低电平
	if ((GPIOB->IDR & (0X1<<7)) == 0)
	{
		Delay_nms(20);// 延时 20ms 消抖
		if ((GPIOB->IDR & (0X1<<7)) == 0)
		{
			// 等待按键松开
			while((GPIOB->IDR & (0X1<<7)) == 0);
			return 1;
		}
	}
	return 0;
}
