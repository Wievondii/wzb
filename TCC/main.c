/**
 * @file    main.c
 * @brief   智能停车场系统主程序 - 完全重写版本
 * @author  王子柏（2023132316）张智研（2023132319）
 * @version 4.0
 * @date    2026-03-18
 * @note    完全重写修复了OLED乱码、传感器识别等问题
 */

#include "system.h"
#include "delay.h"
#include "gpio.h"
#include "pwm.h"
#include "i2c_soft.h"
#include "oled.h"
#include "sensor.h"
#include "key.h"
#include "alarm.h"
#include "parking.h"

/* ==================== 全局变量定义 ==================== */
SystemMode g_systemMode = MODE_AUTO;
SystemStatus g_systemStatus = STATUS_IDLE;
uint8_t g_parkingSpots[TOTAL_PARKING_SPOTS] = {0, 0, 0};
uint8_t g_emptyCount = TOTAL_PARKING_SPOTS;
uint8_t g_entryGateOpen = 0;
uint8_t g_exitGateOpen = 0;
uint8_t g_alarmFlag = 0;
RTC_TimeTypeDef g_currentTime = {12, 0, 0};
uint32_t g_systemTickCount = 0;
ParkingTimeTypeDef g_lastParkingTime = {0, 0, 0, 0};

/* ==================== 系统滴答定时器配置 ==================== */
static volatile uint32_t s_ms_Cnt = 0;
static volatile uint8_t s_10ms_Flag = 0;
static volatile uint8_t s_100ms_Flag = 0;
static volatile uint8_t s_1s_Flag = 0;

/**
 * @brief   配置系统滴答定时器 (1ms 中断)
 */
void SysTick_Configuration(void)
{
    // 停止SysTick
    SysTick->CTRL = 0;
    SysTick->VAL = 0;

    // 配置SysTick重装载值：72MHz/8=9MHz, 1ms需要9000个计数
    SysTick->LOAD = 8999;

    // 使能SysTick中断，使用外部时钟源(HCLK/8)
    SysTick->CTRL = SysTick_CTRL_TICKINT_Msk |
                    SysTick_CTRL_ENABLE_Msk;

    // 设置SysTick中断优先级
    NVIC_SetPriority(SysTick_IRQn, 0x0F);
}

/**
 * @brief   系统滴答定时器中断服务函数
 */
void SysTick_Handler(void)
{
    s_ms_Cnt++;

    // 10ms 时间标志
    if (s_ms_Cnt % 10 == 0) {
        s_10ms_Flag = 1;
    }

    // 100ms 时间标志
    if (s_ms_Cnt % 100 == 0) {
        s_100ms_Flag = 1;
    }

    // 1s 时间标志
    if (s_ms_Cnt % 1000 == 0) {
        s_1s_Flag = 1;
        s_ms_Cnt = 0;  // 重置计数器，避免溢出
    }
}

/**
 * @brief   RTC 初始化
 */
void RTC_Init(void)
{
    g_currentTime.hour = 12;
    g_currentTime.minute = 0;
    g_currentTime.second = 0;
    g_systemTickCount = 0;
}

/**
 * @brief   更新时间
 */
void RTC_Time_Update(void)
{
    g_currentTime.second++;

    if (g_currentTime.second >= 60) {
        g_currentTime.second = 0;
        g_currentTime.minute++;

        if (g_currentTime.minute >= 60) {
            g_currentTime.minute = 0;
            g_currentTime.hour++;

            if (g_currentTime.hour >= 24) {
                g_currentTime.hour = 0;
            }
        }
    }

    g_systemTickCount++;
}

/**
 * @brief   系统初始化
 */
void System_Initialize(void)
{
    // 初始化延时功能
    Delay_Init();

    // 初始化所有 GPIO
    GPIO_Init_All();

    // 初始化 PWM (舵机)
    PWM_Init();

    // 初始化软件 I2C
    I2C_Soft_Init();

    // 初始化 OLED 显示屏
    OLED_Init(&I2C1_Pin);
    OLED_Init(&I2C2_Pin);

    // 初始化报警系统
    Alarm_Init();

    // 初始化 RTC 时钟
    RTC_Init();

    // 初始化停车场系统
    Parking_Init();

    // 配置系统滴答定时器
    SysTick_Configuration();

    // 显示欢迎信息
    OLED_Clear(&I2C1_Pin);
    OLED_Show_String(&I2C1_Pin, 16, 2, "Smart Park");
    OLED_Show_String(&I2C1_Pin, 32, 4, "System");
    OLED_Refresh_Gram(&I2C1_Pin);

    OLED_Clear(&I2C2_Pin);
    OLED_Show_String(&I2C2_Pin, 32, 2, "Welcome!");
    OLED_Show_String(&I2C2_Pin, 16, 4, "Version 4.0");
    OLED_Refresh_Gram(&I2C2_Pin);

    Delay_ms(1500);

    // 更新显示
    Parking_Update_Display();
}

/**
 * @brief   按键处理函数
 */
void Key_Process(void)
{
    uint8_t key_value;
    uint8_t key_long;

    key_value = Key_GetValue();
    key_long = Key_GetLongPress();

    switch (key_value) {
        case KEY_START:  // SW2 - 模式切换
            if (g_systemMode == MODE_AUTO) {
                g_systemMode = MODE_MANUAL;
            } else {
                g_systemMode = MODE_AUTO;
            }
            Alarm_Beep(1);
            break;

        case KEY_ENTRY:  // SW3 - 入口道闸（手动）
            if (g_systemMode == MODE_MANUAL) {
                if (g_entry_gate_state == 0) {
                    Parking_Manual_Entry_Gate(1);
                } else {
                    Parking_Manual_Entry_Gate(0);
                }
                Alarm_Beep(1);
            }
            break;

        case KEY_EXIT:  // SW4 - 出口道闸（手动）
            if (g_systemMode == MODE_MANUAL) {
                if (g_exit_gate_state == 0) {
                    Parking_Manual_Exit_Gate(1);
                } else {
                    Parking_Manual_Exit_Gate(0);
                }
                Alarm_Beep(1);
            }
            break;

        case KEY_COUNT_RST:  // SW5 - 计数复位
            g_parking_count = 0;
            g_lastParkingTime.valid = 0;
            g_lastParkingTime.duration = 0;
            g_current_parking_duration = 0;
            Alarm_Beep(2);
            break;

        case KEY_UP:  // SW7 - 时间增加
            g_currentTime.hour++;
            if (g_currentTime.hour >= 24) {
                g_currentTime.hour = 0;
            }
            break;

        case KEY_DOWN:  // SW8 - 时间减少
            if (g_currentTime.hour == 0) {
                g_currentTime.hour = 23;
            } else {
                g_currentTime.hour--;
            }
            break;

        default:
            break;
    }

    // 长按处理
    switch (key_long) {
        case KEY_START:  // 长按自检
            Alarm_Beep(3);
            Servo_Open(SERVO_ENTRY);
            Delay_ms(1000);
            Servo_Close(SERVO_ENTRY);
            Delay_ms(500);
            Servo_Open(SERVO_EXIT);
            Delay_ms(1000);
            Servo_Close(SERVO_EXIT);
            break;

        case KEY_COUNT_RST:  // 长按清除报警
            Alarm_Clear();
            g_parking_full_alarm = 0;
            break;

        default:
            break;
    }
}

/**
 * @brief   主函数
 */
int main(void)
{
    // 系统初始化
    System_Initialize();

    // 主循环
    while (1) {
        /* ==================== 10ms 任务 ==================== */
        if (s_10ms_Flag) {
            s_10ms_Flag = 0;

            // 扫描传感器
            Sensor_Scan();

            // 扫描按键
            Key_Scan();
        }

        /* ==================== 100ms 任务 ==================== */
        if (s_100ms_Flag) {
            s_100ms_Flag = 0;

            // 更新传感器冷却计时器
            Sensor_Update_Cooldown();

            // 更新传感器状态
            Sensor_Update();

            // 处理停车场逻辑
            Parking_Process();

            // 更新报警状态
            Alarm_Update();
        }

        /* ==================== 1s 任务 ==================== */
        if (s_1s_Flag) {
            s_1s_Flag = 0;

            // 更新时间
            RTC_Time_Update();

            // 处理按键
            Key_Process();

            // 更新显示
            Parking_Update_Display();
        }
    }
}
