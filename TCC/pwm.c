/**
 * @file    pwm.c
 * @brief   PWM 配置实现（舵机控制）
 * @note    使用 TIM2 产生 PWM 信号控制 SG90 舵机
 *          SG90 舵机参数：周期 20ms(50Hz), 0.5ms-2.5ms 对应 0-180 度
 */

#include "pwm.h"
#include "delay.h"
#include "system.h"

/**
 * @brief   初始化 PWM（TIM2 CH3 和 CH4）
 * @note    PA2 - TIM2_CH3 (入口舵机)
 *          PA3 - TIM2_CH4 (出口舵机)
 *          频率 50Hz (周期 20ms)
 */
void PWM_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;
    
    // 使能时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    
    // 配置 PA2 和 PA3 为复用推挽输出
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // 时基配置
    // 72MHz / 7200 = 10kHz (0.1ms 一个计数)
    // 2000 个计数 = 20ms (50Hz)
    TIM_TimeBaseStructure.TIM_Period = 19999;          // 自动重装载值 (20000-1)
    TIM_TimeBaseStructure.TIM_Prescaler = 71;          // 预分频 (72-1)
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
    
    // PWM 通道 3 配置 (入口舵机 PA2)
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 750;               // 初始占空比 (0 度)
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OC3Init(TIM2, &TIM_OCInitStructure);
    TIM_OC3PreloadConfig(TIM2, TIM_OCPreload_Enable);
    
    // PWM 通道 4 配置 (出口舵机 PA3)
    TIM_OCInitStructure.TIM_Pulse = 750;               // 初始占空比 (0 度)
    TIM_OC4Init(TIM2, &TIM_OCInitStructure);
    TIM_OC4PreloadConfig(TIM2, TIM_OCPreload_Enable);
    
    // 使能 TIM2
    TIM_Cmd(TIM2, ENABLE);
    
    // 等待定时器稳定后初始化舵机到关闭位置
    Delay_ms(100);
    Servo_Close(SERVO_ENTRY);
    Delay_ms(500);  // 等待舵机转动
    Servo_Close(SERVO_EXIT);
    Delay_ms(500);
}

/**
 * @brief   设置舵机角度
 * @param   servo: 舵机编号 (SERVO_ENTRY 或 SERVO_EXIT)
 * @param   angle: 角度 (0-180)
 * @note    SG90 舵机：0.5ms(0 度) ~ 2.5ms(180 度)
 *          对应 CCR 值：500 ~ 2500 (基于 20ms 周期，10kHz 计数)
 */
void Servo_SetAngle(uint8_t servo, uint16_t angle)
{
    uint16_t ccr_value;
    
    // 限制角度范围
    if (angle > 180) {
        angle = 180;
    }
    
    // 计算 CCR 值：500 + (angle * 2000 / 180)
    ccr_value = 500 + (uint16_t)((uint32_t)angle * 2000 / 180);
    
    // 限制 CCR 值范围
    if (ccr_value > 2500) {
        ccr_value = 2500;
    }
    if (ccr_value < 500) {
        ccr_value = 500;
    }
    
    if (servo == SERVO_ENTRY) {
        TIM_SetCompare3(TIM2, ccr_value);
    } else if (servo == SERVO_EXIT) {
        TIM_SetCompare4(TIM2, ccr_value);
    }
}

/**
 * @brief   打开舵机（抬杆 - 90 度）
 * @param   servo: 舵机编号
 */
void Servo_Open(uint8_t servo)
{
    Servo_SetAngle(servo, SERVO_OPEN_ANGLE);
}

/**
 * @brief   关闭舵机（落杆 - 0 度）
 * @param   servo: 舵机编号
 */
void Servo_Close(uint8_t servo)
{
    Servo_SetAngle(servo, SERVO_CLOSE_ANGLE);
}
