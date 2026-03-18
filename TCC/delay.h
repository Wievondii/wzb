/**
 * @file    delay.h
 * @brief   延时函数头文件
 */

#ifndef __DELAY_H__
#define __DELAY_H__

#include "stm32f10x.h"

void Delay_Init(void);
void Delay_us(uint32_t us);
void Delay_ms(uint32_t ms);
void Delay_s(uint32_t s);

#endif /* __DELAY_H__ */
