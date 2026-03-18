/**
 * @file    alarm.h
 * @brief   声光报警控制头文件
 */

#ifndef __ALARM_H__
#define __ALARM_H__

#include "stm32f10x.h"

void Alarm_Init(void);
void Alarm_Update(void);
void Alarm_Trigger(uint8_t alarm_type);
void Alarm_Clear(void);
void Alarm_Beep(uint8_t times);

// 报警类型定义
#define ALARM_NONE          0x00    // 无报警
#define ALARM_FULL          0x01    // 车位已满
#define ALARM_ENTRY         0x02    // 入口车辆
#define ALARM_EXIT          0x04    // 出口车辆
#define ALARM_ERROR         0x08    // 异常报警

// 报警状态
extern uint8_t g_alarm_type;        // 当前报警类型
extern uint8_t g_alarm_active;      // 报警激活标志

#endif /* __ALARM_H__ */
