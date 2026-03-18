/**
 * @file    alarm.c
 * @brief   声光报警控制实现
 */

#include "alarm.h"
#include "gpio.h"
#include "delay.h"
#include "system.h"

// 报警状态
uint8_t g_alarm_type = ALARM_NONE;      // 当前报警类型
uint8_t g_alarm_active = 0;             // 报警激活标志

// 报警闪烁计时器
static uint16_t s_alarm_timer = 0;
static uint8_t s_alarm_flash_state = 0;

#define ALARM_FLASH_TIME    50      // 闪烁周期 500ms (50 * 10ms)

/**
 * @brief   初始化报警模块
 */
void Alarm_Init(void)
{
    g_alarm_type = ALARM_NONE;
    g_alarm_active = 0;
    Buzzer_Off();
    Alarm_LED_Off();
}

/**
 * @brief   更新报警状态 (每 10ms 调用)
 */
void Alarm_Update(void)
{
    if (g_alarm_type != ALARM_NONE) {
        g_alarm_active = 1;
        
        // 闪烁报警 LED 和蜂鸣器
        s_alarm_timer++;
        if (s_alarm_timer >= ALARM_FLASH_TIME) {
            s_alarm_timer = 0;
            s_alarm_flash_state = !s_alarm_flash_state;
            
            if (s_alarm_flash_state) {
                Alarm_LED_On();
                Buzzer_On();
            } else {
                Alarm_LED_Off();
                Buzzer_Off();
            }
        }
    } else {
        g_alarm_active = 0;
        Buzzer_Off();
        Alarm_LED_Off();
    }
}

/**
 * @brief   触发报警
 * @param   alarm_type: 报警类型
 */
void Alarm_Trigger(uint8_t alarm_type)
{
    g_alarm_type |= alarm_type;
}

/**
 * @brief   清除报警
 */
void Alarm_Clear(void)
{
    g_alarm_type = ALARM_NONE;
    Buzzer_Off();
    Alarm_LED_Off();
}

/**
 * @brief   蜂鸣器鸣叫指定次数
 * @param   times: 鸣叫次数
 */
void Alarm_Beep(uint8_t times)
{
    uint8_t i;
    
    for (i = 0; i < times; i++) {
        Buzzer_On();
        Delay_ms(100);
        Buzzer_Off();
        if (i < times - 1) {
            Delay_ms(100);
        }
    }
}
