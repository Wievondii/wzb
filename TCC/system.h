/**
 * @file    system.h
 * @brief   智能停车场系统 - 系统头文件
 * @author  王子柏（2023132316）张智研（2023132319）
 * @version 2.0
 * @date    2026-03-18
 */

#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#include "stm32f10x.h"
#include <stdio.h>
#include <string.h>

/* ==================== 系统时钟配置 ==================== */
#define SYSCLK_FREQ     72000000    // 系统时钟 72MHz

/* ==================== GPIO 引脚定义 ==================== */
/* 车位检测传感器 */
#define PARK_SENSOR1_PIN    GPIO_Pin_2
#define PARK_SENSOR1_PORT   GPIOB

#define PARK_SENSOR2_PIN    GPIO_Pin_4
#define PARK_SENSOR2_PORT   GPIOB

#define PARK_SENSOR3_PIN    GPIO_Pin_6
#define PARK_SENSOR3_PORT   GPIOB

/* 车辆进出检测传感器 */
#define ENTRY_SENSOR_PIN    GPIO_Pin_0
#define ENTRY_SENSOR_PORT   GPIOA

#define EXIT_SENSOR_PIN     GPIO_Pin_1
#define EXIT_SENSOR_PORT    GPIOA

/* 舵机控制引脚 */
#define ENTRY_SERVO_PIN     GPIO_Pin_2
#define ENTRY_SERVO_PORT    GPIOA
#define ENTRY_SERVO_CH      TIM_Channel_3

#define EXIT_SERVO_PIN      GPIO_Pin_3
#define EXIT_SERVO_PORT     GPIOA
#define EXIT_SERVO_CH       TIM_Channel_4

/* 声光报警 */
#define BUZZER_PIN          GPIO_Pin_4
#define BUZZER_PORT         GPIOA

#define ALARM_LED_PIN       GPIO_Pin_5
#define ALARM_LED_PORT      GPIOA

/* OLED1 (入口显示屏) - 软件 I2C */
#define OLED1_SCL_PIN       GPIO_Pin_6
#define OLED1_SCL_PORT      GPIOA

#define OLED1_SDA_PIN       GPIO_Pin_7
#define OLED1_SDA_PORT      GPIOA

/* OLED2 (出口显示屏) - 软件 I2C */
#define OLED2_SCL_PIN       GPIO_Pin_8
#define OLED2_SCL_PORT      GPIOA

#define OLED2_SDA_PIN       GPIO_Pin_9
#define OLED2_SDA_PORT      GPIOA

/* 按键定义 */
#define KEY_START_PIN       GPIO_Pin_14   // SW2 - 系统启停
#define KEY_START_PORT      GPIOC

#define KEY_ENTRY_PIN       GPIO_Pin_14   // SW3 - 入口道闸控制
#define KEY_ENTRY_PORT      GPIOB

#define KEY_EXIT_PIN        GPIO_Pin_15   // SW4 - 出口道闸控制
#define KEY_EXIT_PORT       GPIOA

#define KEY_COUNT_RST_PIN   GPIO_Pin_15   // SW5 - 计数复位
#define KEY_COUNT_RST_PORT  GPIOB

#define KEY_MENU_PIN        GPIO_Pin_12   // SW6 - 菜单/确认
#define KEY_MENU_PORT       GPIOB

#define KEY_UP_PIN          GPIO_Pin_13   // SW7 - 上翻/增加
#define KEY_UP_PORT         GPIOC

#define KEY_DOWN_PIN        GPIO_Pin_11   // SW8 - 下翻/减少
#define KEY_DOWN_PORT       GPIOB

/* ==================== 车位数量定义 ==================== */
#define TOTAL_PARKING_SPOTS     3       // 总车位数

/* ==================== 舵机角度定义 ==================== */
#define SERVO_CLOSE_ANGLE   0         // 道闸关闭角度
#define SERVO_OPEN_ANGLE    90        // 道闸开启角度

/* ==================== 系统状态定义 ==================== */
typedef enum {
    MODE_AUTO = 0,      // 自动模式
    MODE_MANUAL = 1     // 手动模式
} SystemMode;

typedef enum {
    STATUS_IDLE = 0,    // 空闲
    STATUS_RUNNING = 1  // 运行中
} SystemStatus;

/* ==================== 车位状态定义 ==================== */
typedef enum {
    SPOT_EMPTY = 0,     // 空车位
    SPOT_OCCUPIED = 1   // 车位占用
} SpotStatus;

/* ==================== RTC 时间结构体 ==================== */
typedef struct {
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
} RTC_TimeTypeDef;

/* ==================== 停车时间结构体 ==================== */
typedef struct {
    uint32_t entry_time;    // 进入时间（秒）
    uint32_t exit_time;     // 离开时间（秒）
    uint32_t duration;      // 停车时长（秒）
    uint8_t valid;          // 是否有效
} ParkingTimeTypeDef;

/* ==================== 全局变量声明 ==================== */
extern SystemMode g_systemMode;         // 系统模式
extern SystemStatus g_systemStatus;     // 系统状态
extern uint8_t g_parkingSpots[TOTAL_PARKING_SPOTS];  // 车位状态数组
extern uint8_t g_emptyCount;            // 空车位数量
extern uint8_t g_entryGateOpen;         // 入口道闸开启标志
extern uint8_t g_exitGateOpen;          // 出口道闸开启标志
extern uint8_t g_alarmFlag;             // 报警标志
extern RTC_TimeTypeDef g_currentTime;   // 当前时间
extern uint32_t g_systemTickCount;      // 系统计时（秒）
extern ParkingTimeTypeDef g_lastParkingTime; // 上次停车时间

/* ==================== 函数声明 ==================== */
void System_Init(void);
void RTC_Init(void);
void RTC_Time_Update(void);
void System_Tick_Handler(void);

#endif /* __SYSTEM_H__ */
