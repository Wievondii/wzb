/**
 * @file    delay.c
 * @brief   延时函数实现
 */

#include "delay.h"
#include "system.h"

static uint32_t g_fac_us = 0;   // us 延时倍乘因子
static uint32_t g_fac_ms = 0;   // ms 延时倍乘因子

/**
 * @brief   初始化延时函数
 * @note    使用 SysTick 定时器，时钟源为 HCLK/8
 */
void Delay_Init(void)
{
    // SysTick 时钟源选择 HCLK/8，72MHz/8 = 9MHz
    // 1us 需要 9 个时钟周期
    g_fac_us = SYSCLK_FREQ / 8000000;
    g_fac_ms = (uint32_t)g_fac_us * 1000;
}

/**
 * @brief   微秒级延时
 * @param   us: 延时的微秒数 (最大 16383us)
 */
void Delay_us(uint32_t us)
{
    uint32_t temp;
    
    // 设置重装载值
    SysTick->LOAD = us * g_fac_us;
    
    // 清空当前值
    SysTick->VAL = 0;
    
    // 配置控制寄存器：使能，使用中断，时钟源
    SysTick->CTRL = SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_CLKSOURCE_Msk;
    
    // 等待延时结束
    do {
        temp = SysTick->CTRL;
    } while ((temp & 0x01) && !(temp & (1 << 16)));
    
    // 关闭计数器
    SysTick->CTRL = 0;
}

/**
 * @brief   毫秒级延时
 * @param   ms: 延时的毫秒数
 */
void Delay_ms(uint32_t ms)
{
    uint32_t i, j;
    
    // 对于较长的延时，使用循环方式
    for (i = 0; i < ms; i++) {
        // 1ms 延时，假设 72MHz
        for (j = 0; j < 9000; j++) {
            __NOP();
        }
    }
}

/**
 * @brief   秒级延时
 * @param   s: 延时的秒数
 */
void Delay_s(uint32_t s)
{
    uint32_t i;
    for (i = 0; i < s; i++) {
        Delay_ms(1000);
    }
}
