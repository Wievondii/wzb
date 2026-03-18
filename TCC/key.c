/**
 * @file    key.c
 * @brief   按键扫描实现
 */

#include "key.h"
#include "gpio.h"
#include "delay.h"
#include "system.h"

// 按键事件
uint8_t g_key_value = KEY_NONE;       // 当前按键值
uint8_t g_key_long_press = KEY_NONE;  // 长按按键值

// 按键状态
static uint8_t s_key_state[7] = {0};      // 按键当前状态
static uint8_t s_key_last[7] = {0};       // 按键上次状态
static uint16_t s_key_press_time[7] = {0}; // 按键按下时间
static uint8_t s_key_processed[7] = {0};   // 按键已处理标志
static uint8_t s_key_released[7] = {0};   // 按键已释放标志

#define KEY_DEBOUNCE_TIME   2       // 消抖时间 (2 * 10ms = 20ms)
#define KEY_LONG_PRESS_TIME   30    // 长按时间 3 秒 (30 * 100ms)
#define KEY_REPEAT_TIME     15      // 连发时间 1.5 秒

/**
 * @brief   扫描所有按键
 * @note    每 10ms 调用一次，按键按下为低电平
 */
void Key_Scan(void)
{
    uint8_t i;
    uint8_t current;
    
    // 读取所有按键状态 (低电平表示按下)
    s_key_state[0] = (Key_Read_Start() == 0) ? 1 : 0;     // KEY_START
    s_key_state[1] = (Key_Read_Entry() == 0) ? 1 : 0;     // KEY_ENTRY
    s_key_state[2] = (Key_Read_Exit() == 0) ? 1 : 0;      // KEY_EXIT
    s_key_state[3] = (Key_Read_CountRst() == 0) ? 1 : 0;  // KEY_COUNT_RST
    s_key_state[4] = (Key_Read_Menu() == 0) ? 1 : 0;      // KEY_MENU
    s_key_state[5] = (Key_Read_Up() == 0) ? 1 : 0;        // KEY_UP
    s_key_state[6] = (Key_Read_Down() == 0) ? 1 : 0;      // KEY_DOWN
    
    for (i = 0; i < 7; i++) {
        if (s_key_state[i] != s_key_last[i]) {
            Delay_ms(10);  // 消抖延时
            if (s_key_state[i] != s_key_last[i]) {
                s_key_last[i] = s_key_state[i];
                
                if (s_key_state[i] == 1) {
                    // 按键按下
                    s_key_press_time[i] = 0;
                    s_key_processed[i] = 0;
                    s_key_released[i] = 0;
                }
            }
        }
        
        // 按键按下计时
        if (s_key_state[i] == 1) {
            s_key_press_time[i]++;
            
            // 检测长按 (3 秒)
            if (s_key_press_time[i] >= KEY_LONG_PRESS_TIME && !s_key_processed[i]) {
                s_key_processed[i] = 1;
                g_key_long_press = i + 1;  // 1-7 对应 7 个按键
            }
            
            // 检测短按 (按下后释放，且未达到长按时间)
            if (s_key_press_time[i] >= KEY_DEBOUNCE_TIME && 
                s_key_press_time[i] < KEY_LONG_PRESS_TIME && 
                !s_key_processed[i] && 
                !s_key_released[i]) {
                // 等待释放时触发
            }
        }
        
        // 按键释放时触发短按事件
        if (s_key_state[i] == 0 && s_key_last[i] == 1) {
            if (s_key_press_time[i] >= KEY_DEBOUNCE_TIME && 
                s_key_press_time[i] < KEY_LONG_PRESS_TIME && 
                !s_key_released[i]) {
                g_key_value = i + 1;  // 1-7 对应 7 个按键
                s_key_released[i] = 1;
            }
            s_key_processed[i] = 0;
            s_key_press_time[i] = 0;
        }
    }
}

/**
 * @brief   获取按键值
 * @retval  按键编号 (KEY_NONE 表示无按键)
 */
uint8_t Key_GetValue(void)
{
    uint8_t temp = g_key_value;
    g_key_value = KEY_NONE;
    return temp;
}

/**
 * @brief   获取长按按键值
 * @retval  长按按键编号
 */
uint8_t Key_GetLongPress(void)
{
    uint8_t temp = g_key_long_press;
    g_key_long_press = KEY_NONE;
    return temp;
}

/**
 * @brief   清除按键状态
 */
void Key_Clear(void)
{
    g_key_value = KEY_NONE;
    g_key_long_press = KEY_NONE;
}
