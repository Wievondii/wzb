/**
 * @file    main.c
 * @brief   智能停车场系统主程序
 * @author  王子柏（2023132316）张智研（2023132319）
 * @version 3.0
 * @date    2026-03-18
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
uint8_t g_parkingSpots[TOTAL_PARKING_SPOTS] = {0};
uint8_t g_emptyCount = TOTAL_PARKING_SPOTS;
uint8_t g_entryGateOpen = 0;
uint8_t g_exitGateOpen = 0;
uint8_t g_alarmFlag = 0;
RTC_TimeTypeDef g_currentTime = {12, 0, 0};
uint32_t g_systemTickCount = 0;
ParkingTimeTypeDef g_lastParkingTime = {0};

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
    SysTick->CTRL = 0;
    SysTick->VAL = 0;
    SysTick->LOAD = 8999;  // 72MHz/8=9MHz, 1ms=9000 计数
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |
                    SysTick_CTRL_TICKINT_Msk |
                    SysTick_CTRL_ENABLE_Msk;
    NVIC_SetPriority(SysTick_IRQn, 0x0F);
}

/**
 * @brief   系统滴答定时器中断服务函数
 */
void SysTick_Handler(void)
{
    s_ms_Cnt++;
    
    if (s_ms_Cnt % 10 == 0) {
        s_10ms_Flag = 1;
    }
    
    if (s_ms_Cnt % 100 == 0) {
        s_100ms_Flag = 1;
    }
    
    if (s_ms_Cnt % 1000 == 0) {
        s_1s_Flag = 1;
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
    Delay_Init();
    GPIO_Init_All();
    PWM_Init();
    I2C_Soft_Init();
    
    OLED_Init(&I2C1_Pin);
    OLED_Init(&I2C2_Pin);
    
    Alarm_Init();
    RTC_Init();
    Parking_Init();
    
    SysTick_Configuration();
    
    // 显示欢迎信息
    OLED_Clear(&I2C1_Pin);
    OLED_Show_String(&I2C1_Pin, 16, 2, "Smart Park");
    OLED_Show_String(&I2C1_Pin, 32, 4, "System");
    OLED_Refresh_Gram(&I2C1_Pin);
    
    OLED_Clear(&I2C2_Pin);
    OLED_Show_String(&I2C2_Pin, 32, 2, "Welcome!");
    OLED_Refresh_Gram(&I2C2_Pin);
    
    Delay_ms(1500);
    
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
            s_vehicle_entered = 0;
            s_entry_timestamp = 0;
            Alarm_Beep(2);
            break;
            
        case KEY_UP:  // SW7 - 时间增加
            g_currentTime.hour++;
            if (g_currentTime.hour >= 24) g_currentTime.hour = 0;
            break;
            
        case KEY_DOWN:  // SW8 - 时间减少
            if (g_currentTime.hour == 0) g_currentTime.hour = 23;
            else g_currentTime.hour--;
            break;
            
        default:
            break;
    }
    
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
    System_Initialize();
    
    while (1) {
        // 10ms 任务
        if (s_10ms_Flag) {
            s_10ms_Flag = 0;
            Sensor_Scan();
            Key_Scan();
        }
        
        // 100ms 任务
        if (s_100ms_Flag) {
            s_100ms_Flag = 0;
            Sensor_Update();
            Parking_Process();
            Alarm_Update();
        }
        
        // 1s 任务
        if (s_1s_Flag) {
            s_1s_Flag = 0;
            RTC_Time_Update();
            Key_Process();
            Parking_Update_Display();
        }
    }
}
