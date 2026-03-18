/**
 * @file    pwm.h
 * @brief   PWM 配置头文件（舵机控制）
 */

#ifndef __PWM_H__
#define __PWM_H__

#include "stm32f10x.h"

void PWM_Init(void);
void Servo_SetAngle(uint8_t servo, uint16_t angle);
void Servo_Open(uint8_t servo);
void Servo_Close(uint8_t servo);

// 舵机编号定义
#define SERVO_ENTRY     0   // 入口舵机
#define SERVO_EXIT      1   // 出口舵机

#endif /* __PWM_H__ */
