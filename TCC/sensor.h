/**
 * @file    sensor.h
 * @brief   传感器检测逻辑头文件
 */

#ifndef __SENSOR_H__
#define __SENSOR_H__

#include "stm32f10x.h"

void Sensor_Scan(void);
void Sensor_Update(void);
uint8_t Sensor_Get_Park_Status(uint8_t index);
uint8_t Sensor_Get_Entry_Trigger(void);
uint8_t Sensor_Get_Exit_Trigger(void);
void Sensor_Clear_Trigger(void);
void Sensor_Clear_Full_Trigger(void);

// 传感器状态
extern uint8_t g_park_sensor_state[3];      // 车位传感器状态
extern uint8_t g_entry_sensor_state;        // 入口传感器状态
extern uint8_t g_exit_sensor_state;         // 出口传感器状态
extern uint8_t g_entry_trigger;             // 入口触发标志（有空位时）
extern uint8_t g_exit_trigger;              // 出口触发标志
extern uint8_t g_entry_full_trigger;        // 入口触发标志（车位满时）

#endif /* __SENSOR_H__ */
