/**
 * @file    key.h
 * @brief   按键扫描头文件
 */

#ifndef __KEY_H__
#define __KEY_H__

#include "stm32f10x.h"

void Key_Scan(void);
uint8_t Key_GetValue(void);
uint8_t Key_GetLongPress(void);
void Key_Clear(void);

// 按键编号定义
typedef enum {
    KEY_NONE = 0,       // 无按键
    KEY_START,          // SW2 - 系统启停
    KEY_ENTRY,          // SW3 - 入口道闸控制
    KEY_EXIT,           // SW4 - 出口道闸控制
    KEY_COUNT_RST,      // SW5 - 计数复位
    KEY_MENU,           // SW6 - 菜单/确认
    KEY_UP,             // SW7 - 上翻/增加
    KEY_DOWN            // SW8 - 下翻/减少
} KeyCode;

// 按键事件
extern uint8_t g_key_value;         // 当前按键值
extern uint8_t g_key_long_press;    // 长按标志

#define KEY_LONG_PRESS_TIME   30    // 长按时间 3 秒 (30 * 100ms)

#endif /* __KEY_H__ */
