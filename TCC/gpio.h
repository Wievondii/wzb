/**
 * @file    gpio.h
 * @brief   GPIO 初始化头文件（传感器、按键、报警）
 */

#ifndef __GPIO_H__
#define __GPIO_H__

#include "stm32f10x.h"

void GPIO_Init_All(void);
void GPIO_Sensor_Init(void);
void GPIO_Key_Init(void);
void GPIO_Alarm_Init(void);
void GPIO_OLED_Init(void);

// 传感器读取函数
uint8_t Sensor_Read_Park1(void);
uint8_t Sensor_Read_Park2(void);
uint8_t Sensor_Read_Park3(void);
uint8_t Sensor_Read_Entry(void);
uint8_t Sensor_Read_Exit(void);

// 按键读取函数
uint8_t Key_Read_Start(void);
uint8_t Key_Read_Entry(void);
uint8_t Key_Read_Exit(void);
uint8_t Key_Read_CountRst(void);
uint8_t Key_Read_Menu(void);
uint8_t Key_Read_Up(void);
uint8_t Key_Read_Down(void);

// 报警控制函数
void Buzzer_On(void);
void Buzzer_Off(void);
void Alarm_LED_On(void);
void Alarm_LED_Off(void);

#endif /* __GPIO_H__ */
