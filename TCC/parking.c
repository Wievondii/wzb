/**
 * @file    parking.c
 * @brief   停车场业务逻辑实现
 */

#include "parking.h"
#include "pwm.h"
#include "sensor.h"
#include "alarm.h"
#include "oled.h"
#include "i2c_soft.h"
#include "delay.h"
#include "system.h"
#include "gpio.h"

// 停车场状态
uint8_t g_parking_total = TOTAL_PARKING_SPOTS;   // 总车位数
uint8_t g_parking_empty = TOTAL_PARKING_SPOTS;   // 空车位数
uint8_t g_entry_gate_state = 0;    // 入口道闸状态 0-关闭 1-开启
uint8_t g_exit_gate_state = 0;     // 出口道闸状态 0-关闭 1-开启
uint32_t g_parking_count = 0;      // 累计停车次数
uint8_t g_parking_full_alarm = 0;  // 车位已满报警标志

// 道闸自动关闭计时器
uint16_t g_entry_gate_timer = 0;
uint16_t g_exit_gate_timer = 0;

// 停车时间记录
uint32_t g_current_parking_duration = 0; // 当前停车时长（秒）
static uint32_t s_entry_timestamp = 0;   // 车辆进入时间戳
static uint8_t s_vehicle_entered = 0;    // 车辆已进入标志

// 入口车辆等待计时器
static uint16_t s_entry_wait_timer = 0;
static uint8_t s_entry_waiting = 0;

// 出口车辆等待计时器
static uint16_t s_exit_wait_timer = 0;
static uint8_t s_exit_waiting = 0;

// OLED 显示闪烁控制
static uint8_t s_display_flash_cnt = 0;
static uint8_t s_display_flash_state = 0;

#define ENTRY_WAIT_TIME   100     // 入口等待时间 10 秒
#define EXIT_WAIT_TIME    100     // 出口等待时间 10 秒
#define FLASH_TIME        50      // 闪烁时间 500ms

/**
 * @brief   初始化停车场系统
 */
void Parking_Init(void)
{
    g_parking_total = TOTAL_PARKING_SPOTS;
    g_parking_empty = TOTAL_PARKING_SPOTS;
    g_entry_gate_state = 0;
    g_exit_gate_state = 0;
    g_parking_count = 0;
    g_parking_full_alarm = 0;
    g_entry_gate_timer = 0;
    g_exit_gate_timer = 0;
    s_entry_timestamp = 0;
    s_vehicle_entered = 0;
    g_current_parking_duration = 0;
    
    // 初始化舵机到关闭位置
    Servo_Close(SERVO_ENTRY);
    Delay_ms(800);
    Servo_Close(SERVO_EXIT);
    Delay_ms(800);
    
    // 初始化显示
    Parking_Update_Display();
}

/**
 * @brief   处理停车场逻辑 (每 100ms 调用)
 */
void Parking_Process(void)
{
    // 处理入口道闸自动关闭
    if (g_entry_gate_state == 1 && g_entry_gate_timer > 0) {
        g_entry_gate_timer--;
        if (g_entry_gate_timer == 0) {
            Servo_Close(SERVO_ENTRY);
            g_entry_gate_state = 0;
        }
    }
    
    // 处理出口道闸自动关闭
    if (g_exit_gate_state == 1 && g_exit_gate_timer > 0) {
        g_exit_gate_timer--;
        if (g_exit_gate_timer == 0) {
            Servo_Close(SERVO_EXIT);
            g_exit_gate_state = 0;
        }
    }
    
    // 处理入口车辆进入等待
    if (s_entry_waiting) {
        s_entry_wait_timer++;
        if (s_entry_wait_timer >= ENTRY_WAIT_TIME) {
            if (g_entry_gate_state == 1) {
                Servo_Close(SERVO_ENTRY);
                g_entry_gate_state = 0;
            }
            s_entry_waiting = 0;
            s_entry_wait_timer = 0;
        }
    }
    
    // 处理出口车辆离开等待
    if (s_exit_waiting) {
        s_exit_wait_timer++;
        if (s_exit_wait_timer >= EXIT_WAIT_TIME) {
            if (g_exit_gate_state == 1) {
                Servo_Close(SERVO_EXIT);
                g_exit_gate_state = 0;
            }
            s_exit_waiting = 0;
            s_exit_wait_timer = 0;
        }
    }
    
    // 更新停车时长
    if (s_vehicle_entered) {
        g_current_parking_duration = g_systemTickCount - s_entry_timestamp;
    }

    // 同步空车位数量
    g_parking_empty = g_emptyCount;

    // 自动模式下的处理
    if (g_systemMode == MODE_AUTO) {
        // 检查入口传感器触发（有空位时）
        if (Sensor_Get_Entry_Trigger()) {
            Sensor_Clear_Trigger();
            Parking_Entry_Vehicle();
        }
        
        // 检查入口传感器触发（车位满时）
        if (Sensor_Get_Entry_Full_Trigger()) {
            Sensor_Clear_Full_Trigger();
            // 车位已满，触发报警，舵机不动作
            g_parking_full_alarm = 1;
            Alarm_Trigger(ALARM_FULL);
        }
        
        // 检查出口传感器触发
        if (Sensor_Get_Exit_Trigger()) {
            Sensor_Clear_Trigger();
            Parking_Exit_Vehicle();
        }
    }
    
    // 闪烁控制
    s_display_flash_cnt++;
    if (s_display_flash_cnt >= FLASH_TIME) {
        s_display_flash_cnt = 0;
        s_display_flash_state = !s_display_flash_state;
    }
}

/**
 * @brief   入口车辆进入处理
 */
void Parking_Entry_Vehicle(void)
{
    // 清除车位已满报警
    g_parking_full_alarm = 0;
    Alarm_Clear();
    
    // 记录进入时间
    s_entry_timestamp = g_systemTickCount;
    s_vehicle_entered = 1;
    
    // 开启入口道闸
    Servo_Open(SERVO_ENTRY);
    g_entry_gate_state = 1;
    g_entry_gate_timer = GATE_AUTO_CLOSE_TIME;
    s_entry_waiting = 1;
    s_entry_wait_timer = 0;
    
    // 增加停车计数
    g_parking_count++;
}

/**
 * @brief   出口车辆离开处理
 */
void Parking_Exit_Vehicle(void)
{
    // 记录离开时间和停车时长
    if (s_vehicle_entered) {
        g_lastParkingTime.exit_time = g_systemTickCount;
        g_lastParkingTime.duration = g_systemTickCount - s_entry_timestamp;
        g_lastParkingTime.valid = 1;
        g_current_parking_duration = g_lastParkingTime.duration;
    }
    
    // 开启出口道闸
    Servo_Open(SERVO_EXIT);
    g_exit_gate_state = 1;
    g_exit_gate_timer = GATE_AUTO_CLOSE_TIME;
    s_exit_waiting = 1;
    s_exit_wait_timer = 0;
}

/**
 * @brief   手动控制入口道闸
 * @param   open: 1-开启，0-关闭
 */
void Parking_Manual_Entry_Gate(uint8_t open)
{
    if (open) {
        Servo_Open(SERVO_ENTRY);
        g_entry_gate_state = 1;
    } else {
        Servo_Close(SERVO_ENTRY);
        g_entry_gate_state = 0;
    }
}

/**
 * @brief   手动控制出口道闸
 * @param   open: 1-开启，0-关闭
 */
void Parking_Manual_Exit_Gate(uint8_t open)
{
    if (open) {
        Servo_Open(SERVO_EXIT);
        g_exit_gate_state = 1;
    } else {
        Servo_Close(SERVO_EXIT);
        g_exit_gate_state = 0;
    }
}

/**
 * @brief   格式化时间显示
 */
void Parking_Format_Time(uint32_t seconds, char* buffer)
{
    uint32_t h, m, s;
    h = seconds / 3600;
    m = (seconds % 3600) / 60;
    s = seconds % 60;
    sprintf(buffer, "%02lu:%02lu:%02lu", h, m, s);
}

/**
 * @brief   更新 OLED 显示
 */
void Parking_Update_Display(void)
{
    char time_str[16];
    char duration_str[16];
    
    // 格式化当前时间
    sprintf(time_str, "%02d:%02d:%02d", g_currentTime.hour, g_currentTime.minute, g_currentTime.second);
    
    // ==================== 入口显示屏 (OLED1) ====================
    OLED_Clear(&I2C1_Pin);
    
    // 第一行：当前时间
    OLED_Show_String(&I2C1_Pin, 0, 0, "Time:");
    OLED_Show_String(&I2C1_Pin, 48, 0, time_str);
    
    // 第二行：车位信息
    OLED_Show_String(&I2C1_Pin, 0, 2, "Total:");
    OLED_Show_Num(&I2C1_Pin, 42, 2, g_parking_total);
    OLED_Show_String(&I2C1_Pin, 60, 2, "Empty:");
    OLED_Show_Num(&I2C1_Pin, 102, 2, g_parking_empty);
    
    // 第三行：车位满溢提示（闪烁）
    if (g_parking_empty == 0) {
        if (s_display_flash_state) {
            // 显示"车位已满"
            OLED_Show_Chinese(&I2C1_Pin, 16, 4, CHI_CHE);   // 车
            OLED_Show_Chinese(&I2C1_Pin, 32, 4, CHI_WEI);   // 位
            OLED_Show_Chinese(&I2C1_Pin, 48, 4, CHI_YI);    // 已
            OLED_Show_Chinese(&I2C1_Pin, 64, 4, CHI_MAN);   // 满
        }
    } else {
        // 显示"欢迎光临"
        OLED_Show_Chinese(&I2C1_Pin, 16, 4, CHI_HUAN);  // 欢
        OLED_Show_Chinese(&I2C1_Pin, 32, 4, CHI_YING);  // 迎
        OLED_Show_Chinese(&I2C1_Pin, 48, 4, CHI_GUANG); // 光
        OLED_Show_Chinese(&I2C1_Pin, 64, 4, CHI_LIN);   // 临
    }
    
    OLED_Refresh_Gram(&I2C1_Pin);
    
    // ==================== 出口显示屏 (OLED2) ====================
    OLED_Clear(&I2C2_Pin);
    
    // 第一行：当前时间
    OLED_Show_String(&I2C2_Pin, 0, 0, "Time:");
    OLED_Show_String(&I2C2_Pin, 48, 0, time_str);
    
    // 第二行：停车时长
    OLED_Show_String(&I2C2_Pin, 0, 2, "Parking:");
    Parking_Format_Time(g_current_parking_duration, duration_str);
    OLED_Show_String(&I2C2_Pin, 64, 2, duration_str);
    
    // 第三行：感谢提示 "一路平安"
    OLED_Show_Chinese(&I2C2_Pin, 16, 4, CHI_YI1);   // 一
    OLED_Show_Chinese(&I2C2_Pin, 32, 4, CHI_LU);    // 路
    OLED_Show_Chinese(&I2C2_Pin, 48, 4, CHI_PING);  // 平
    OLED_Show_Chinese(&I2C2_Pin, 64, 4, CHI_AN);    // 安
    
    OLED_Refresh_Gram(&I2C2_Pin);
}
