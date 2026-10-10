#ifndef _ZIGBEE_H_
#define _ZIGBEE_H_

#include "stm32f10x.h"
#include "stdio.h"
#include "string.h"
#include "rs485.h"
#include "usart.h"
#include "delay.h"
#include "led.h"

#define ZigBee_Reset(x) (GPIO_WriteBit(GPIOB,GPIO_Pin_13,(BitAction)x))

#define ZigBeeRX_MAX 128
//Lora接收数据结构体
typedef struct{
	uint8_t  RxBuf[ZigBeeRX_MAX];	//串口数据接收缓冲区
	uint8_t  Rx_over;    				//接收完成标志0 -- 没有接收完成 1 -- 接收完成
	uint32_t Rx_count;          //接收数据的个数
}_ZigBeeData_TypeDef;

//添加新ZigBee节点时使用到的枚举
typedef enum{
	ZigBee_OpenNet=0,			//打开协调器网络
	ZigBee_GetDev,				//获取新节点接入网络
	ZigBee_GetDevInfor,		//获取新节点信息
//	ZigBee_AddLib,				//将新节点保存库
	ZigBee_AddFail				//新节点添加失败
}_ZigBeeAddSta;

//Zigbee节点库（会保存在Flash中）
typedef struct{
	uint16_t  NetAddr;		//短地址
	uint8_t  PointNum;		//端点数量
	uint8_t  MAC[8];			//MAC地址
	uint32_t DevType;			//设备类型
	uint8_t  PointList[8];//端点号数组
}_ZigBeeNewDevice;

typedef struct{
	_ZigBeeNewDevice Dev[10];		//每个设备信息
	uint8_t DevCount;						//节点设备数量
}_ZigBeeDevLib;
extern _ZigBeeDevLib Z_DeviceLib;

typedef struct{
	_ZigBeeNewDevice Dev[10];		//每个设备信息
	uint8_t DevCount;						//节点设备数量
}_WinCoverDevLib;
extern _WinCoverDevLib W_DeviceLib;
 
//控制ZigBee设备信息
typedef struct{
	uint8_t DevNumber;		//设备号
	uint8_t DevPort;			//设备端点
	uint8_t PortState;		//端点状态
}_ZigBeeCtrl;
extern _ZigBeeCtrl	Z_DevCtrlInfor;

typedef enum{
	Z_AddNo=0,
	Z_AddLemp,
	Z_AddWindows,
}_ZigAddDev;

extern _ZigAddDev ZigBeeAddFlag;
extern uint8_t ZigBeeAddNem;
extern uint16_t ZigBeeAddFrom;
extern u8 flag_zigbee0;
extern u8 flag_zigbee1;
extern _ZigBeeData_TypeDef ZigBeeGetData;

void ZigBee_Config(uint32_t brr);
void ZigBee_Sendbuf(uint8_t* data, uint8_t size);
void ZigBee_DataAnalysis(void);
uint8_t ZigBee_XoR(uint8_t *data, uint8_t len);
void ZigBee_SoftReset(void);
void ZigBee_PANID(void);
void ZigBee_LempCtrl(_ZigBeeDevLib LempInfo,uint8_t state);
//void ZigBee_LempCtrl(_ZigBeeCtrl LempInfo);
void ZigBee_LempFind(_ZigBeeCtrl LempInfo);
void ZigBee_OpenCloseCoor(uint8_t timers);
void ZigBee_GetPointList(uint16_t addr);

void ZigBee_WindowsSetState(uint16_t addr,uint8_t state);
void ZigBee_WindowsSetNum(uint16_t addr,uint8_t num);
void ZigBee_WindowsGetMode(uint16_t addr);
void ZigBee_GetDevType(uint16_t addr);
u8 ZigBeeAddDevice(void);
#endif
