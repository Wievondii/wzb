/**
 * @file    parking.h
 * @brief   停车场业务逻辑头文件
 */

#ifndef __PARKING_H__
#define __PARKING_H__

#include "stm32f10x.h"

void Parking_Init(void);
void Parking_Process(void);
void Parking_Update_Display(void);
void Parking_Entry_Vehicle(void);
void Parking_Exit_Vehicle(void);
void Parking_Manual_Entry_Gate(uint8_t open);
void Parking_Manual_Exit_Gate(uint8_t open);
void Parking_Record_Entry_Time(void);
void Parking_Record_Exit_Time(void);
void Parking_Format_Time(uint32_t seconds, char* buffer);

// 停车场状态
extern uint8_t g_parking_total;       // 总车位数
extern uint8_t g_parking_empty;       // 空车位数
extern uint8_t g_entry_gate_state;    // 入口道闸状态 0-关闭 1-开启
extern uint8_t g_exit_gate_state;     // 出口道闸状态 0-关闭 1-开启
extern uint32_t g_parking_count;      // 累计停车次数
extern uint8_t g_parking_full_alarm;  // 车位已满报警标志

// 道闸自动关闭计时器
extern uint16_t g_entry_gate_timer;
extern uint16_t g_exit_gate_timer;

// 停车时间记录
extern uint32_t g_current_parking_duration; // 当前停车时长（秒）

#define GATE_AUTO_CLOSE_TIME  50      // 道闸自动关闭时间 5 秒 (50 * 100ms)

#endif /* __PARKING_H__ */
