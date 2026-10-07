#ifndef KEY_H_
#define KEY_H_

#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "stdio.h"
#include "Serial.h"

/**
  *@biref °´¼ü³õÊ¼»¯
  */
void Key_Init(void);

uint8_t Key2_GetVal(void);

uint8_t Key1_GetVal(void);

uint8_t Key0_GetVal(void);

#endif
