#include "zigbee.h"


_ZigBeeAddSta Zigbee_AddState = ZigBee_OpenNet;//状态机
u8 flag_zigbee0=0;
u8 flag_zigbee1=0;

_ZigBeeData_TypeDef ZigBeeGetData = {"\0"};
_ZigBeeCtrl	Z_DevCtrlInfor;	//控制设备信息
_ZigBeeDevLib Z_DeviceLib;	//ZigBee灯控设备库
_WinCoverDevLib W_DeviceLib;//ZigBee窗帘设备库
_ZigAddDev ZigBeeAddFlag = Z_AddLemp;	//zigbee添加设备标识
uint8_t ZigBeeAddNem = 0;	//ZigBee添加号
uint16_t ZigBeeAddFrom = 0;		//ZigBee添加消息来源

/****************************
函数名称：ZigBee_Config
函数作用：ZigBee模块初始化
函数入口：brr 波特率
函数出口：无
函数作者：LDJ
创建时间：2021.08.05 16:56
修改时间：2021.08.05 16:56
****************************/
void ZigBee_Config(uint32_t brr)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);

	//GPIO端口配置
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	//USART配置
	USART_InitStructure.USART_BaudRate = brr;		//波特率
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//硬件控制流--不使用
	USART_InitStructure.USART_Mode = USART_Mode_Rx|USART_Mode_Tx;//接收器、发送器
	USART_InitStructure.USART_Parity = USART_Parity_No;	//不校验
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//停止位
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//数据位
	USART_Init(USART2,&USART_InitStructure);
	//配置为串口接收中断与空闲中断
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
	USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);
	NVIC_SetPriority(USART2_IRQn,1);
	NVIC_EnableIRQ(USART2_IRQn);
	//USART模块使能
	USART_Cmd(USART2,ENABLE);
	
	ZigBee_Reset(1);
}

/****************************
函数名称：USART2_IRQHandler
函数作用：ZigBee中断服务函数
函数入口：无
函数出口：无
函数作者：LDJ
创建时间：2021.08.05 15:31
修改时间：2021.08.05 15:34
****************************/
void USART2_IRQHandler(void)
{
	uint8_t data = 0;
	data = data;
	if(USART_GetITStatus(USART2, USART_IT_RXNE) == SET)
	{
		USART_ClearITPendingBit(USART2, USART_IT_RXNE);
		if(ZigBeeGetData.Rx_over == 0)
			ZigBeeGetData.RxBuf[ZigBeeGetData.Rx_count++] = USART2->DR;
	}
	if(USART_GetITStatus(USART2, USART_IT_IDLE) == SET)
	{
		data = USART2->SR;
		data = USART2->DR;
//		for(int i=0;i<ZigBeeGetData.Rx_count;i++)
//			printf("%c",ZigBeeGetData.RxBuf[i]);
//		memset(ZigBeeGetData.RxBuf,0,ZigBeeGetData.Rx_count);
//		ZigBeeGetData.Rx_count=0;
		ZigBeeGetData.Rx_over = 1;	//接收完成
	}
}

/****************************
函数名称：ZigBee_Sendbuf
函数作用：ZigBee发送缓冲区
函数入口：
	data	缓冲区地址
	size	发送长度
函数出口：无
函数作者：LDJ
创建时间：2021.08.06 10:13
修改时间：2021.08.06 10:13
****************************/
void ZigBee_Sendbuf(uint8_t* data, uint8_t size)
{
	uint8_t i = 0;
	for(i=0; i<size; i++)
	{
		while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);
		USART2->DR = data[i];
	}
}

/****************************
函数名称：ZigBee_XoR
函数作用：ZigBee异或校验计算
函数入口：
	data	缓冲区地址
	len	数据长度
函数出口：异或结果
函数作者：LDJ
创建时间：2021.08.06 11:44
修改时间：2021.08.06 11:44
****************************/
uint8_t ZigBee_XoR(uint8_t *data, uint8_t len)
{
	uint32_t i=0;
	uint8_t DataXoR = '\0';
	DataXoR = *data;
	data++;
	for(i=0;i<len-1;i++)
	{
		DataXoR ^= *data;
		data++;
	}
	return DataXoR;
}

/****************************
函数名称：ZigBee_SoftReset
函数作用：ZigBee重启模块
函数入口：无
函数出口：无
函数作者：LDJ
创建时间：2021.08.06 11:44
修改时间：2021.08.06 11:54
****************************/
void ZigBee_SoftReset(void)
{
	uint8_t SendBuf[16] = "\0";
	uint8_t i=0;
	
	SendBuf[i++] = 0XFE;
	SendBuf[i++] = 0X0A;
	SendBuf[i++] = 0X01;
	SendBuf[i++] = 0X02;
	SendBuf[i++] = 0X02;
	SendBuf[i++] = 0X00;
	SendBuf[i++] = 0X00;
	SendBuf[i++] = 0X00;
	SendBuf[i++] = 0X02;
	SendBuf[i] = ZigBee_XoR(SendBuf,i);

	ZigBee_Sendbuf(SendBuf,i+1);
}

/****************************
函数名称：ZigBee_OpenCloseCoor
函数作用：ZigBee开关网络
函数入口：无
函数出口：无
函数作者：LDJ
创建时间：2021.08.09 13:35
修改时间：2021.08.09 13:35
****************************/
void ZigBee_OpenCloseCoor(uint8_t timers)
{
	uint16_t i = 0;
	uint8_t SendBuf[32] = {0};
	SendBuf[i++] = 0xFE;                                                //帧头
	SendBuf[i++] = 0x0B;                                                //帧长
	SendBuf[i++] = 0x02;                                                //层命令
	SendBuf[i++] = 0x01;                                                //子命令
	SendBuf[i++] = 0x02;                                                //地址模式
	SendBuf[i++] = 0X00;                                             //地址低位
	SendBuf[i++] = 0X00;                                             //地址高位
	SendBuf[i++] = 0X00;                                             //端点
	SendBuf[i++] = 0X04;                                             //帧序号    
	SendBuf[i++] = timers;	
	SendBuf[i] = ZigBee_XoR(SendBuf,i);                                //计算效验值
//	USART3_Sendstring(SendBuf,i+1);
	ZigBee_Sendbuf(SendBuf,i+1);
}

/****************************
函数名称：ZigBee_GetPointList
函数作用：获取ZigBee端点数据
函数入口：addr	端点短地址
函数出口：无
函数作者：LDJ
创建时间：2021.08.09 17:39
修改时间：2021.08.09 17:39
****************************/
void ZigBee_GetPointList(uint16_t addr)
{
	uint16_t i = 0;
	uint8_t SendBuf[32] = "\0";
	SendBuf[i++] = 0xFE;                                                //帧头
	SendBuf[i++] = 0x0A;                                                //帧长
	SendBuf[i++] = 0x02;                                                //层命令
	SendBuf[i++] = 0x03;                                                //子命令
	SendBuf[i++] = 0x02;                                                //地址模式
	SendBuf[i++] = addr&0XFF;                                           //地址低位
	SendBuf[i++] = addr>>8;                                             //地址高位
	SendBuf[i++] = 0X00;                                             		//端点
	SendBuf[i++] = 0X04;                                             		//帧序号    	
	SendBuf[i] = ZigBee_XoR(SendBuf,i);                                	//计算效验值
//	USART3_Sendstring(SendBuf,i+1);
	ZigBee_Sendbuf(SendBuf,i+1);
	
}

/****************************
函数名称：ZigBee_GetPointList
函数作用：获取ZigBee端点数据
函数入口：addr	端点短地址
函数出口：无
函数作者：LDJ
创建时间：2021.08.09 17:39
修改时间：2021.08.09 17:39
****************************/
void ZigBee_GetDevType(uint16_t addr)
{
	uint16_t i = 0;
	uint8_t SendBuf[32] = "\0";
	SendBuf[i++] = 0xFE;                                                //帧头
	SendBuf[i++] = 0x0A;                                                //帧长
	SendBuf[i++] = 0x02;                                                //层命令
	SendBuf[i++] = 0x04;                                                //子命令
	SendBuf[i++] = 0x02;                                                //地址模式
	SendBuf[i++] = addr&0XFF;                                           //地址低位
	SendBuf[i++] = addr>>8;                                             //地址高位
	SendBuf[i++] = 0X01;                                             		//端点
	SendBuf[i++] = 0X05;                                             		//帧序号    	
	SendBuf[i] = ZigBee_XoR(SendBuf,i);                                	//计算效验值
	
	ZigBee_Sendbuf(SendBuf,i+1);
}

/*************************************控制函数*****************************************/
/****************************
函数名称：ZigBee_LempCtrl
函数作用：ZigBee灯泡控制（开关）
函数入口：
	LempInfo	控制灯设备结构体
函数出口：无
函数作者：LDJ
创建时间：2021.08.09 11:40
修改时间：2021.08.10 14:06
****************************/
void ZigBee_LempCtrl(_ZigBeeDevLib LempInfo,uint8_t state)
{
	uint16_t i = 0;
	uint8_t SendBuf[32] = "\0";
	SendBuf[i++] = 0xfe;                                               //帧头
	SendBuf[i++] = 0x0b;                                               //帧长
	SendBuf[i++] = 0x05;                                               //层命令
	SendBuf[i++] = 0x01;                                               //子命令
	SendBuf[i++] = 0x02;                                               //地址模式
	SendBuf[i++] = LempInfo.Dev[0].NetAddr & 0xff; //地址低位
	SendBuf[i++] = LempInfo.Dev[0].NetAddr >>8;   //地址高位
	SendBuf[i++] = Z_DeviceLib.Dev[ZigBeeAddNem].PointList[0];                                   //端点
	SendBuf[i++] = 0X0A;                                               //帧序号
	SendBuf[i++] = state;                                 //操作命令 
	SendBuf[i] = ZigBee_XoR(SendBuf,i);                                //计算效验值
//	USART3_Sendstring(SendBuf,i+1);
	ZigBee_Sendbuf(SendBuf,i+1);
}
/****************************
函数名称：ZigBee_LempFind
函数作用：ZigBee灯查询状态
函数入口：
	LempInfo	控制灯设备结构体
函数出口：无
函数作者：LDJ
创建时间：2021.08.11 11:40
修改时间：2021.08.11 11:40
****************************/
void ZigBee_LempFind(_ZigBeeCtrl LempInfo)
{
	uint16_t i = 0;
	uint8_t SendBuf[32] = "\0";
	SendBuf[i++] = 0xfe;                                               //帧头
	SendBuf[i++] = 0x0c;                                               //帧长
	SendBuf[i++] = 0x05;                                               //层命令
	SendBuf[i++] = 0x05;                                               //子命令
	SendBuf[i++] = 0x02;                                               //地址模式
	SendBuf[i++] = (Z_DeviceLib.Dev[LempInfo.DevNumber-1].NetAddr)&0XFF; //地址低位
	SendBuf[i++] = (Z_DeviceLib.Dev[LempInfo.DevNumber-1].NetAddr)>>8;   //地址高位
	SendBuf[i++] = LempInfo.DevPort;                                   //端点
	SendBuf[i++] = 0X0B;                                               //帧序号
	SendBuf[i++] = 0X00;    
	SendBuf[i++] = 0X00; 	
	SendBuf[i] = ZigBee_XoR(SendBuf,i);                                //计算效验值
	
	ZigBee_Sendbuf(SendBuf,i+1);
}

/***********************************解析数据函数***************************************/
/****************************
函数名称：LempGetState
函数作用：ZigBee灯泡状态获取
函数入口：无
函数出口：无
函数作者：LDJ
创建时间：2021.08.10 14:08
修改时间：2021.08.10 14:08
****************************/
static void LempGetState(void)
{
	uint8_t i = 0;
	uint8_t Local_ACK[16] = {0XAA,0X43,0X00,0X05,0X01,0XFF,0XFF,0XFF,0XFF,0XFF};
	
	//最多10个设备，遍历查找设备
	for(i=0;i<10;i++)
	{
		if(Z_DeviceLib.Dev[i].NetAddr == (ZigBeeGetData.RxBuf[5]|(ZigBeeGetData.RxBuf[6]<<8)))
			break;
	}
	//发送响应
	Local_ACK[5] = i+1;//设备编码
	Local_ACK[6] = ZigBeeGetData.RxBuf[7];//设备节点
	Local_ACK[7] = ZigBeeGetData.RxBuf[3]&0x0F;//CMD
	Local_ACK[8] = 0X00;//指令响应成功
	Local_ACK[9] = RS485_Add(Local_ACK,9);
	USART3_Sendstring(Local_ACK,10);
}

/****************************
函数名称：LempFindState
函数作用：ZigBee灯状态查询结果
函数入口：无
函数出口：无
函数作者：LDJ
创建时间：2021.08.10 14:31
修改时间：2021.08.10 14:31
****************************/
static void LempFindState(void)
{
	uint8_t i = 0;
	uint8_t Local_ACK[16] = {0XAA,0X43,0X00,0X05,0X01,0XFF,0XFF,0XFF,0XFF,0XFF};
	for(i=0;i<10;i++)	//遍历找设备
	{
		if(Z_DeviceLib.Dev[i].NetAddr == (ZigBeeGetData.RxBuf[5]|(ZigBeeGetData.RxBuf[6]<<8)))
			break;
	}
	
	//发送响应
	Local_ACK[5] = i+1;//设备编码
	Local_ACK[6] = ZigBeeGetData.RxBuf[7];//设备节点
	Local_ACK[7] = 0X02;//CMD
	Local_ACK[8] = ZigBeeGetData.RxBuf[10];
	Local_ACK[9] = RS485_Add(Local_ACK,9);
	USART3_Sendstring(Local_ACK,10);
}

/****************************
函数名称：KeyGetState
函数作用：ZigBee按键状态获取
函数入口：无
函数出口：无
函数作者：LDJ
创建时间：2021.08.10 14:31
修改时间：2021.08.10 14:31
****************************/
static void KeyGetState(void)
{
	uint8_t i = 0;
	uint8_t Local_ACK[16] = {0XAA,0X43,0X01,0X04,0X01,0XFF,0XFF,0XFF,0XFF,0XFF};
	
	for(i=0;i<10;i++)
	{
		if(Z_DeviceLib.Dev[i].NetAddr == (ZigBeeGetData.RxBuf[5]|(ZigBeeGetData.RxBuf[6]<<8)))
			break;
	}
	
	//发送响应
	Local_ACK[5] = i+1;//设备编码
	Local_ACK[6] = ZigBeeGetData.RxBuf[7];//设备节点
	Local_ACK[7] = ZigBeeGetData.RxBuf[9];
	Local_ACK[8] = RS485_Add(Local_ACK,8);
	USART3_Sendstring(Local_ACK,9);

}

/************************************************************窗帘***********************************************************************/
/****************************
函数名称：ZigBee_WindowsSetState
函数作用：设置窗帘状态
函数入口：
	addr	端点短地址
	state	0x00-关 0x01 开 0x02暂停
函数出口：无
函数作者：LDJ
创建时间：2021.08.26 13:58
修改时间：2021.08.26 13:58
****************************/
void ZigBee_WindowsSetState(uint16_t addr,uint8_t state)
{
	uint16_t i = 0;
	uint8_t SendBuf[32] = "\0";
	SendBuf[i++] = 0xfe;                    //帧头
	SendBuf[i++] = 0x0b;                    //帧长
	SendBuf[i++] = 0x08;                    //层命令
	SendBuf[i++] = 0x01;                    //子命令
	SendBuf[i++] = 0x02;                    //地址模式
	SendBuf[i++] = addr&0XFF; 							//地址低位
	SendBuf[i++] = addr>>8;   							//地址高位
	SendBuf[i++] = 0X01;                    //端点
	SendBuf[i++] = 0X88;                    //帧序号
	SendBuf[i++] = state;                   //操作命令 
	SendBuf[i] = ZigBee_XoR(SendBuf,i);     //计算效验值
	
	ZigBee_Sendbuf(SendBuf,i+1);
}

/****************************
函数名称：ZigBee_WindowsSetNum
函数作用：设置窗帘到百分比
函数入口：
	addr	端点短地址
	state	0x00-关 0x01 开 0x02暂停
函数出口：无
函数作者：LDJ
创建时间：2021.08.26 13:58
修改时间：2021.08.26 13:58
****************************/
void ZigBee_WindowsSetNum(uint16_t addr,uint8_t num)
{
	uint16_t i = 0;
	uint8_t SendBuf[32] = "\0";
	SendBuf[i++] = 0xfe;                    //帧头
	SendBuf[i++] = 0x0C;                    //帧长
	SendBuf[i++] = 0x08;                    //层命令
	SendBuf[i++] = 0x07;                    //子命令
	SendBuf[i++] = 0x02;                    //地址模式
	SendBuf[i++] = addr&0XFF; 							//地址低位
	SendBuf[i++] = addr>>8;   							//地址高位
	SendBuf[i++] = 0X01;                    //端点
	SendBuf[i++] = 0X17;                    //帧序号
	SendBuf[i++] = 0X01;                    //类型
	SendBuf[i++] = num;                   //操作命令 
	SendBuf[i] = ZigBee_XoR(SendBuf,i);     //计算效验值
	
	ZigBee_Sendbuf(SendBuf,i+1);
}

/****************************
函数名称：ZigBee_WindowsGetMode
函数作用：设置窗帘工作模式
函数入口：
	addr	端点短地址
函数出口：无
函数作者：LDJ
创建时间：2021.08.26 13:58
修改时间：2021.08.26 13:58
****************************/
void ZigBee_WindowsGetMode(uint16_t addr)
{
	uint16_t i = 0;
	uint8_t SendBuf[32] = "\0";
	SendBuf[i++] = 0xfe;                    //帧头
	SendBuf[i++] = 0x0C;                    //帧长
	SendBuf[i++] = 0x08;                    //层命令
	SendBuf[i++] = 0x04;                    //子命令
	SendBuf[i++] = 0x02;                    //地址模式
	SendBuf[i++] = addr&0XFF; 							//地址低位
	SendBuf[i++] = addr>>8;   							//地址高位
	SendBuf[i++] = 0X01;                    //端点
	SendBuf[i++] = 0X11;                    //帧序号
	SendBuf[i++] = 0X17;                   //操作命令 
	SendBuf[i++] = 0X00;                   //操作命令 
	SendBuf[i] = ZigBee_XoR(SendBuf,i);     //计算效验值
	
	ZigBee_Sendbuf(SendBuf,i+1);
}
/************************************************************数据解析***********************************************************************/

/****************************
函数名称：ZigBee_DataAnalysis
函数作用：ZigBee数据解析
函数入口：无
函数出口：无
函数作者：LDJ
创建时间：2021.08.05 15:31
修改时间：2021.08.05 15:34
****************************/
void ZigBee_DataAnalysis(void)
{
	uint8_t i = 0;
	if(ZigBeeGetData.Rx_over == 1)
	{
		if(ZigBeeGetData.Rx_count < 3)
			goto GetError;
		if((ZigBeeGetData.RxBuf[0] != 0XFE)||(ZigBeeGetData.RxBuf[ZigBeeGetData.Rx_count-1] != ZigBee_XoR(ZigBeeGetData.RxBuf,ZigBeeGetData.Rx_count-1)))
			goto GetError;
		
		//判断是否为打开协调器网络
		if((ZigBeeGetData.RxBuf[2]==0X02)&&(ZigBeeGetData.RxBuf[3]==0X41))
		{
			Zigbee_AddState = ZigBee_AddFail;
			printf("网络已打开\r\n");
		}
		
		
		
		
		//判断是否为新添加设备
		if((ZigBeeGetData.RxBuf[2]==0X02)&&(ZigBeeGetData.RxBuf[3]==0X82))
		{
			if(ZigBeeAddFlag == Z_AddLemp)//如果在添加灯
			{
				Z_DeviceLib.Dev[ZigBeeAddNem].NetAddr = ZigBeeGetData.RxBuf[5]|(ZigBeeGetData.RxBuf[6]<<8);
				Z_DeviceLib.Dev[ZigBeeAddNem].MAC[0] = ZigBeeGetData.RxBuf[10];
				Z_DeviceLib.Dev[ZigBeeAddNem].MAC[1] = ZigBeeGetData.RxBuf[11];
				Z_DeviceLib.Dev[ZigBeeAddNem].MAC[2] = ZigBeeGetData.RxBuf[12];
				Z_DeviceLib.Dev[ZigBeeAddNem].MAC[3] = ZigBeeGetData.RxBuf[13];
				Z_DeviceLib.Dev[ZigBeeAddNem].MAC[4] = ZigBeeGetData.RxBuf[14];
				Z_DeviceLib.Dev[ZigBeeAddNem].MAC[5] = ZigBeeGetData.RxBuf[15];
				Z_DeviceLib.Dev[ZigBeeAddNem].MAC[6] = ZigBeeGetData.RxBuf[16];
				Z_DeviceLib.Dev[ZigBeeAddNem].MAC[7] = ZigBeeGetData.RxBuf[17];
				u8 flag_zigbee1=0;
				Zigbee_AddState = ZigBee_GetDev;
				
				printf("设备已加入网络!\r\n");
				memset(ZigBeeGetData.RxBuf,0,ZigBeeGetData.Rx_count);
				ZigBeeGetData.Rx_count = 0;
				ZigBeeGetData.Rx_over = 0;
			}

		}
		 
		if((ZigBeeGetData.RxBuf[2]==0X02) && (ZigBeeGetData.RxBuf[3]==0X43))//获取设备端点的响应
		{
			memset(ZigBeeGetData.RxBuf,0,ZigBeeGetData.Rx_count);
			ZigBeeGetData.Rx_count = 0;
			ZigBeeGetData.Rx_over = 0;
			while(ZigBeeGetData.Rx_over==0);
			if(ZigBeeGetData.Rx_over == 1)
			{
				if((ZigBeeGetData.RxBuf[2]==0X02) && (ZigBeeGetData.RxBuf[3]==0X83))//获取设备端点的响应
				{
//					for(int i=0;i<ZigBeeGetData.Rx_count;i++)
//						printf("%x ",ZigBeeGetData.RxBuf[i]);
//					printf("接收到查询端点的第2条响应\r\n");
					if((ZigBeeAddFlag == Z_AddLemp)&&(ZigBeeGetData.RxBuf[2]==0X02))//如果在添加灯
					{
						Z_DeviceLib.Dev[ZigBeeAddNem].PointNum = ZigBeeGetData.RxBuf[10];
						for(i=0;i<ZigBeeGetData.RxBuf[10];i++)
							Z_DeviceLib.Dev[ZigBeeAddNem].PointList[i] = ZigBeeGetData.RxBuf[11+i];
						flag_zigbee0=1;
						Zigbee_AddState = ZigBee_GetDevInfor;
						LINK(0);
						printf("point ok!\r\n");
					}
				}
				
			}
			
		}			
GetError:		
		memset(ZigBeeGetData.RxBuf,0,ZigBeeGetData.Rx_count);
		ZigBeeGetData.Rx_count = 0;
		ZigBeeGetData.Rx_over = 0;
	}
}




/****************************
函数名称：Task_ZigBeeAddDevice
函数作用：ZigBee添加新设备
函数入口：无
函数出口：无
函数作者：LDJ
创建时间：2021.08.09 17:04 
修改时间：2021.08.09 17:04
备注
	vTaskResume(ZigBee_AddDeviceHandle);
****************************/
u8 ZigBeeAddDevice(void)
{	
	while(1)
	{
		switch(Zigbee_AddState)
		{
			case ZigBee_OpenNet:
				ZigBee_OpenCloseCoor(0x3c);//ZigBee开关网络
				while(ZigBeeGetData.Rx_over == 0);
				ZigBee_DataAnalysis();		
				
				break;
			case ZigBee_GetDev://请求获取设备端点
				if(ZigBeeAddFlag == Z_AddLemp)
				{
					if(Z_DeviceLib.Dev[ZigBeeAddNem].NetAddr != 0X0000)
					{
						ZigBee_GetPointList(Z_DeviceLib.Dev[ZigBeeAddNem].NetAddr);
					}
				}
				
				while(ZigBeeGetData.Rx_over == 0);
				ZigBee_DataAnalysis();		
				break;
			case ZigBee_AddFail://等待设备加入网络
				ZigBee_DataAnalysis();		
				break;
			default:
				return 0;

		}
	}

}
