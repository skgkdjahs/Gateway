#ifndef _RS232_H
#define _RS232_H

#include "stm32f10x.h"
#include "delay.h"

//
#define RS232RX_MAX 256

typedef struct{
	u8 RxBuf[RS232RX_MAX];
	u8 Rx_over;
	u32 Rx_count;
}RS232_TypeDef;







#endif
