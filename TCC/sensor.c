/**
 * @file    sensor.c
 * @brief   传感器检测逻辑实现
 * @note    红外避障模块：检测到物体时输出低电平 (0)，无物体时输出高电平 (1)
 *          触发检测：检测下降沿 (从 1 变 0) 表示有车辆进入检测区域
 */

#include "sensor.h"
#include "gpio.h"
#include "delay.h"
#include "system.h"

// 传感器状态
uint8_t g_park_sensor_state[3] = {0};     // 车位传感器状态
uint8_t g_entry_sensor_state = 0;          // 入口传感器状态
uint8_t g_exit_sensor_state = 0;           // 出口传感器状态
uint8_t g_entry_trigger = 0;               // 入口触发标志（有空位时）
uint8_t g_exit_trigger = 0;                // 出口触发标志
uint8_t g_entry_full_trigger = 0;          // 入口触发标志（车位满时）

// 上一次读取的状态
static uint8_t g_last_park_state[3] = {0};
static uint8_t g_last_entry_state = 1;     // 初始为高电平（无车）
static uint8_t g_last_exit_state = 1;      // 初始为高电平（无车）

// 消抖计数器
static uint8_t g_debounce_park[3] = {0};
static uint8_t g_debounce_entry = 0;
static uint8_t g_debounce_exit = 0;

// 防反复触发计时器 (单位：100ms)
static uint16_t g_entry_cooldown = 0;
static uint16_t g_exit_cooldown = 0;

#define DEBOUNCE_COUNT      5       // 消抖次数 (约 50ms)
#define COOLDOWN_TIME       50      // 防反复触发时间 5 秒 (50 * 100ms)

/**
 * @brief   扫描所有传感器
 * @note    每 10ms 调用一次
 *          红外传感器：检测到物体=低电平 (0)，无物体=高电平 (1)
 */
void Sensor_Scan(void)
{
    uint8_t i;
    uint8_t current_state;
    
    // ==================== 扫描车位传感器 ====================
    // 车位传感器：检测到车=低电平 (0)，空位=高电平 (1)
    for (i = 0; i < 3; i++) {
        if (i == 0) {
            current_state = Sensor_Read_Park1();
        } else if (i == 1) {
            current_state = Sensor_Read_Park2();
        } else {
            current_state = Sensor_Read_Park3();
        }
        
        if (current_state != g_last_park_state[i]) {
            if (g_debounce_park[i] < DEBOUNCE_COUNT) {
                g_debounce_park[i]++;
            }
            if (g_debounce_park[i] >= DEBOUNCE_COUNT) {
                g_park_sensor_state[i] = current_state;
                g_last_park_state[i] = current_state;
                g_debounce_park[i] = 0;
            }
        } else {
            g_debounce_park[i] = 0;
        }
    }
    
    // ==================== 扫描入口传感器 ====================
    // 入口传感器：检测到车=低电平 (0)，无车=高电平 (1)
    // 检测下降沿触发 (从 1 变 0 表示车辆进入检测区域)
    current_state = Sensor_Read_Entry();
    
    if (current_state == 0 && g_last_entry_state == 1) {
        // 检测到下降沿（车辆进入）
        if (g_debounce_entry < DEBOUNCE_COUNT) {
            g_debounce_entry++;
        }
        if (g_debounce_entry >= DEBOUNCE_COUNT) {
            // 确认触发
            if (g_entry_cooldown == 0) {
                // 根据车位状态设置不同的触发标志
                if (g_emptyCount > 0) {
                    g_entry_trigger = 1;      // 有空位，正常进入
                } else {
                    g_entry_full_trigger = 1; // 车位满，触发报警
                }
                g_entry_cooldown = COOLDOWN_TIME;
            }
            g_debounce_entry = 0;
        }
    } else if (current_state == 1) {
        g_debounce_entry = 0;
    }
    g_last_entry_state = current_state;
    
    // ==================== 扫描出口传感器 ====================
    // 出口传感器：检测到车=低电平 (0)，无车=高电平 (1)
    // 检测下降沿触发 (从 1 变 0 表示车辆进入检测区域)
    current_state = Sensor_Read_Exit();
    
    if (current_state == 0 && g_last_exit_state == 1) {
        // 检测到下降沿（车辆进入）
        if (g_debounce_exit < DEBOUNCE_COUNT) {
            g_debounce_exit++;
        }
        if (g_debounce_exit >= DEBOUNCE_COUNT) {
            // 确认触发
            if (g_exit_cooldown == 0) {
                g_exit_trigger = 1;
                g_exit_cooldown = COOLDOWN_TIME;
            }
            g_debounce_exit = 0;
        }
    } else if (current_state == 1) {
        g_debounce_exit = 0;
    }
    g_last_exit_state = current_state;
    
    // 冷却计时器递减
    if (g_entry_cooldown > 0) {
        g_entry_cooldown--;
    }
    if (g_exit_cooldown > 0) {
        g_exit_cooldown--;
    }
}

/**
 * @brief   更新车位状态到全局变量
 */
void Sensor_Update(void)
{
    uint8_t i;
    
    for (i = 0; i < 3; i++) {
        // 车位传感器：低电平=有车，高电平=空位
        // 转换为：0=空位，1=占用
        if (g_park_sensor_state[i] == 0) {
            g_parkingSpots[i] = SPOT_OCCUPIED;   // 低电平=有车
        } else {
            g_parkingSpots[i] = SPOT_EMPTY;      // 高电平=空位
        }
    }
    
    // 计算空车位数量
    g_emptyCount = 0;
    for (i = 0; i < TOTAL_PARKING_SPOTS; i++) {
        if (g_parkingSpots[i] == SPOT_EMPTY) {
            g_emptyCount++;
        }
    }
}

/**
 * @brief   获取车位状态
 * @param   index: 车位索引 (0-2)
 * @retval  0: 空车位，1: 占用
 */
uint8_t Sensor_Get_Park_Status(uint8_t index)
{
    if (index >= 3) {
        return 0;
    }
    return g_parkingSpots[index];
}

/**
 * @brief   获取入口触发标志（有空位）
 * @retval  触发标志
 */
uint8_t Sensor_Get_Entry_Trigger(void)
{
    return g_entry_trigger;
}

/**
 * @brief   获取出口触发标志
 * @retval  触发标志
 */
uint8_t Sensor_Get_Exit_Trigger(void)
{
    return g_exit_trigger;
}

/**
 * @brief   获取入口触发标志（车位满）
 * @retval  触发标志
 */
uint8_t Sensor_Get_Entry_Full_Trigger(void)
{
    return g_entry_full_trigger;
}

/**
 * @brief   清除触发标志
 */
void Sensor_Clear_Trigger(void)
{
    g_entry_trigger = 0;
    g_exit_trigger = 0;
}

/**
 * @brief   清除车位满触发标志
 */
void Sensor_Clear_Full_Trigger(void)
{
    g_entry_full_trigger = 0;
}
