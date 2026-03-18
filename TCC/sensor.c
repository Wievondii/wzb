/**
 * @file    sensor.c
 * @brief   传感器检测逻辑实现 - 完全重写版本
 * @version 4.0
 * @date    2026-03-18
 * @note    红外避障模块：检测到物体时输出低电平 (0)，无物体时输出高电平 (1)
 *          触发检测：检测下降沿 (从 1 变 0) 表示有车辆进入检测区域
 *          优化了传感器读取逻辑，确保红外对射避障模块正确识别
 */

#include "sensor.h"
#include "gpio.h"
#include "delay.h"
#include "system.h"

/* ==================== 传感器状态变量 ==================== */
// 当前传感器状态
uint8_t g_park_sensor_state[3] = {1, 1, 1};  // 初始为高电平（无车）
uint8_t g_entry_sensor_state = 1;             // 入口传感器状态
uint8_t g_exit_sensor_state = 1;              // 出口传感器状态

// 触发标志
uint8_t g_entry_trigger = 0;                  // 入口触发标志（有空位时）
uint8_t g_exit_trigger = 0;                   // 出口触发标志
uint8_t g_entry_full_trigger = 0;             // 入口触发标志（车位满时）

/* ==================== 私有变量 ==================== */
// 上一次读取的传感器状态
static uint8_t s_last_park_state[3] = {1, 1, 1};  // 初始为高电平（无车）
static uint8_t s_last_entry_state = 1;             // 初始为高电平（无车）
static uint8_t s_last_exit_state = 1;              // 初始为高电平（无车）

// 消抖计数器
static uint8_t s_debounce_park[3] = {0, 0, 0};
static uint8_t s_debounce_entry = 0;
static uint8_t s_debounce_exit = 0;

// 防反复触发计时器 (单位：100ms)
static uint16_t s_entry_cooldown = 0;
static uint16_t s_exit_cooldown = 0;

/* ==================== 常量定义 ==================== */
#define DEBOUNCE_COUNT      5       // 消抖次数 (约 50ms，每10ms检查一次)
#define COOLDOWN_TIME       50      // 防反复触发时间 5 秒 (50 * 100ms)

/**
 * @brief   读取单个传感器状态（带消抖）
 * @param   read_func: 传感器读取函数指针
 * @retval  消抖后的传感器状态
 */
static uint8_t Sensor_Read_With_Debounce(uint8_t (*read_func)(void))
{
    uint8_t i;
    uint8_t state_sum = 0;

    // 连续读取多次，取平均值
    for (i = 0; i < 3; i++) {
        state_sum += read_func();
        if (i < 2) {
            Delay_ms(1);  // 短延时
        }
    }

    // 如果大于等于2次读到高电平，则认为是高电平
    return (state_sum >= 2) ? 1 : 0;
}

/**
 * @brief   扫描所有传感器
 * @note    每 10ms 调用一次
 *          红外传感器：检测到物体=低电平 (0)，无物体=高电平 (1)
 */
void Sensor_Scan(void)
{
    uint8_t i;
    uint8_t current_state;

    /* ==================== 扫描车位传感器 ==================== */
    // 车位传感器：检测到车=低电平 (0)，空位=高电平 (1)
    for (i = 0; i < 3; i++) {
        // 读取传感器状态
        if (i == 0) {
            current_state = Sensor_Read_With_Debounce(Sensor_Read_Park1);
        } else if (i == 1) {
            current_state = Sensor_Read_With_Debounce(Sensor_Read_Park2);
        } else {
            current_state = Sensor_Read_With_Debounce(Sensor_Read_Park3);
        }

        // 状态发生变化，开始消抖
        if (current_state != s_last_park_state[i]) {
            s_debounce_park[i]++;

            // 消抖计数达到阈值，确认状态改变
            if (s_debounce_park[i] >= DEBOUNCE_COUNT) {
                g_park_sensor_state[i] = current_state;
                s_last_park_state[i] = current_state;
                s_debounce_park[i] = 0;
            }
        } else {
            // 状态未变化，重置消抖计数器
            s_debounce_park[i] = 0;
        }
    }

    /* ==================== 扫描入口传感器 ==================== */
    // 入口传感器：检测到车=低电平 (0)，无车=高电平 (1)
    // 检测下降沿触发 (从 1 变 0 表示车辆进入检测区域)
    current_state = Sensor_Read_With_Debounce(Sensor_Read_Entry);

    // 检测到下降沿（车辆进入）
    if (current_state == 0 && s_last_entry_state == 1) {
        s_debounce_entry++;

        // 消抖计数达到阈值，确认触发
        if (s_debounce_entry >= DEBOUNCE_COUNT) {
            // 检查冷却时间
            if (s_entry_cooldown == 0) {
                // 根据车位状态设置不同的触发标志
                if (g_emptyCount > 0) {
                    g_entry_trigger = 1;      // 有空位，正常进入
                } else {
                    g_entry_full_trigger = 1; // 车位满，触发报警
                }

                // 设置冷却时间
                s_entry_cooldown = COOLDOWN_TIME;
            }

            s_debounce_entry = 0;
        }
    } else if (current_state != s_last_entry_state) {
        // 状态变化但不是下降沿，重置消抖计数器
        s_debounce_entry = 0;
    } else if (current_state == 1) {
        // 恢复到高电平，重置消抖计数器
        s_debounce_entry = 0;
    }

    // 更新状态
    g_entry_sensor_state = current_state;
    s_last_entry_state = current_state;

    /* ==================== 扫描出口传感器 ==================== */
    // 出口传感器：检测到车=低电平 (0)，无车=高电平 (1)
    // 检测下降沿触发 (从 1 变 0 表示车辆进入检测区域)
    current_state = Sensor_Read_With_Debounce(Sensor_Read_Exit);

    // 检测到下降沿（车辆进入）
    if (current_state == 0 && s_last_exit_state == 1) {
        s_debounce_exit++;

        // 消抖计数达到阈值，确认触发
        if (s_debounce_exit >= DEBOUNCE_COUNT) {
            // 检查冷却时间
            if (s_exit_cooldown == 0) {
                g_exit_trigger = 1;

                // 设置冷却时间
                s_exit_cooldown = COOLDOWN_TIME;
            }

            s_debounce_exit = 0;
        }
    } else if (current_state != s_last_exit_state) {
        // 状态变化但不是下降沿，重置消抖计数器
        s_debounce_exit = 0;
    } else if (current_state == 1) {
        // 恢复到高电平，重置消抖计数器
        s_debounce_exit = 0;
    }

    // 更新状态
    g_exit_sensor_state = current_state;
    s_last_exit_state = current_state;
}

/**
 * @brief   更新冷却计时器
 * @note    每 100ms 调用一次
 */
void Sensor_Update_Cooldown(void)
{
    // 冷却计时器递减
    if (s_entry_cooldown > 0) {
        s_entry_cooldown--;
    }

    if (s_exit_cooldown > 0) {
        s_exit_cooldown--;
    }
}

/**
 * @brief   更新车位状态到全局变量
 * @note    每 100ms 调用一次
 */
void Sensor_Update(void)
{
    uint8_t i;
    uint8_t empty_count = 0;

    // 更新车位状态数组
    for (i = 0; i < 3; i++) {
        // 车位传感器：低电平=有车 (0)，高电平=空位 (1)
        // 转换为：SPOT_OCCUPIED=有车，SPOT_EMPTY=空位
        if (g_park_sensor_state[i] == 0) {
            g_parkingSpots[i] = SPOT_OCCUPIED;   // 低电平=有车
        } else {
            g_parkingSpots[i] = SPOT_EMPTY;      // 高电平=空位
            empty_count++;
        }
    }

    // 更新空车位数量
    g_emptyCount = empty_count;
}

/**
 * @brief   获取车位状态
 * @param   index: 车位索引 (0-2)
 * @retval  SPOT_EMPTY: 空车位，SPOT_OCCUPIED: 占用
 */
uint8_t Sensor_Get_Park_Status(uint8_t index)
{
    if (index >= 3) {
        return SPOT_EMPTY;
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

/**
 * @brief   获取传感器诊断信息
 * @param   diag: 诊断信息结构体指针
 */
void Sensor_Get_Diagnostics(Sensor_Diagnostics_t* diag)
{
    if (diag == NULL) {
        return;
    }

    // 车位传感器状态
    diag->park1_state = g_park_sensor_state[0];
    diag->park2_state = g_park_sensor_state[1];
    diag->park3_state = g_park_sensor_state[2];

    // 入口/出口传感器状态
    diag->entry_state = g_entry_sensor_state;
    diag->exit_state = g_exit_sensor_state;

    // 冷却时间
    diag->entry_cooldown = s_entry_cooldown;
    diag->exit_cooldown = s_exit_cooldown;

    // 触发标志
    diag->entry_trigger = g_entry_trigger;
    diag->exit_trigger = g_exit_trigger;
    diag->entry_full_trigger = g_entry_full_trigger;
}
