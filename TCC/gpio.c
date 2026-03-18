/**
 * @file    gpio.c
 * @brief   GPIO 初始化实现（传感器、按键、报警）
 */

#include "gpio.h"
#include "delay.h"
#include "system.h"

/**
 * @brief   初始化所有 GPIO
 */
void GPIO_Init_All(void)
{
    // 使能所有 GPIO 时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | 
                           RCC_APB2Periph_GPIOC, ENABLE);
    
    // 使能 AFIO 时钟（用于 PA15 等复用引脚）
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    
    // 禁用 JTAG，释放 PA13-PA15 作为普通 GPIO
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
    
    GPIO_Sensor_Init();
    GPIO_Key_Init();
    GPIO_Alarm_Init();
    GPIO_OLED_Init();
}

/**
 * @brief   初始化传感器 GPIO（输入模式）
 */
void GPIO_Sensor_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // 车位传感器 PB2, PB4, PB6 - 上拉输入
    GPIO_InitStructure.GPIO_Pin = PARK_SENSOR1_PIN | PARK_SENSOR2_PIN | PARK_SENSOR3_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;  // 上拉输入
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(PARK_SENSOR1_PORT, &GPIO_InitStructure);
    
    // 入口传感器 PA0 - 上拉输入
    GPIO_InitStructure.GPIO_Pin = ENTRY_SENSOR_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(ENTRY_SENSOR_PORT, &GPIO_InitStructure);
    
    // 出口传感器 PA1 - 上拉输入
    GPIO_InitStructure.GPIO_Pin = EXIT_SENSOR_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(EXIT_SENSOR_PORT, &GPIO_InitStructure);
}

/**
 * @brief   初始化按键 GPIO（输入模式）
 */
void GPIO_Key_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // SW2 - 系统启停 PC14 - 上拉输入
    GPIO_InitStructure.GPIO_Pin = KEY_START_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(KEY_START_PORT, &GPIO_InitStructure);
    
    // SW3 - 入口道闸控制 PB14 - 上拉输入
    GPIO_InitStructure.GPIO_Pin = KEY_ENTRY_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(KEY_ENTRY_PORT, &GPIO_InitStructure);
    
    // SW4 - 出口道闸控制 PA15 - 上拉输入（需要 JTAG 禁用）
    GPIO_InitStructure.GPIO_Pin = KEY_EXIT_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(KEY_EXIT_PORT, &GPIO_InitStructure);
    
    // SW5 - 计数复位 PB15 - 上拉输入
    GPIO_InitStructure.GPIO_Pin = KEY_COUNT_RST_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(KEY_COUNT_RST_PORT, &GPIO_InitStructure);
    
    // SW6 - 菜单/确认 PB12 - 上拉输入
    GPIO_InitStructure.GPIO_Pin = KEY_MENU_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(KEY_MENU_PORT, &GPIO_InitStructure);
    
    // SW7 - 上翻/增加 PC13 - 上拉输入
    GPIO_InitStructure.GPIO_Pin = KEY_UP_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(KEY_UP_PORT, &GPIO_InitStructure);
    
    // SW8 - 下翻/减少 PB11 - 上拉输入
    GPIO_InitStructure.GPIO_Pin = KEY_DOWN_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(KEY_DOWN_PORT, &GPIO_InitStructure);
}

/**
 * @brief   初始化报警 GPIO（输出模式）
 */
void GPIO_Alarm_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // 蜂鸣器 PA4 - 推挽输出
    GPIO_InitStructure.GPIO_Pin = BUZZER_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(BUZZER_PORT, &GPIO_InitStructure);
    
    // 报警 LED PA5 - 推挽输出
    GPIO_InitStructure.GPIO_Pin = ALARM_LED_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(ALARM_LED_PORT, &GPIO_InitStructure);
    
    // 初始关闭报警
    Buzzer_Off();
    Alarm_LED_Off();
}

/**
 * @brief   初始化 OLED GPIO（I2C 引脚）
 */
void GPIO_OLED_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // OLED1 I2C - PA6(SCL), PA7(SDA) - 开漏输出
    GPIO_InitStructure.GPIO_Pin = OLED1_SCL_PIN | OLED1_SDA_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(OLED1_SCL_PORT, &GPIO_InitStructure);
    
    // OLED2 I2C - PA8(SCL), PA9(SDA) - 开漏输出
    GPIO_InitStructure.GPIO_Pin = OLED2_SCL_PIN | OLED2_SDA_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_Init(OLED2_SCL_PORT, &GPIO_InitStructure);
}

/**
 * @brief   读取车位传感器 1 状态
 * @retval  0: 空车位，1: 占用
 */
uint8_t Sensor_Read_Park1(void)
{
    return GPIO_ReadInputDataBit(PARK_SENSOR1_PORT, PARK_SENSOR1_PIN);
}

/**
 * @brief   读取车位传感器 2 状态
 * @retval  0: 空车位，1: 占用
 */
uint8_t Sensor_Read_Park2(void)
{
    return GPIO_ReadInputDataBit(PARK_SENSOR2_PORT, PARK_SENSOR2_PIN);
}

/**
 * @brief   读取车位传感器 3 状态
 * @retval  0: 空车位，1: 占用
 */
uint8_t Sensor_Read_Park3(void)
{
    return GPIO_ReadInputDataBit(PARK_SENSOR3_PORT, PARK_SENSOR3_PIN);
}

/**
 * @brief   读取入口传感器状态
 * @retval  0: 有车（触发），1: 无车
 */
uint8_t Sensor_Read_Entry(void)
{
    return GPIO_ReadInputDataBit(ENTRY_SENSOR_PORT, ENTRY_SENSOR_PIN);
}

/**
 * @brief   读取出口传感器状态
 * @retval  0: 有车（触发），1: 无车
 */
uint8_t Sensor_Read_Exit(void)
{
    return GPIO_ReadInputDataBit(EXIT_SENSOR_PORT, EXIT_SENSOR_PIN);
}

/**
 * @brief   读取系统启停按键
 * @retval  0: 按下，1: 未按下
 */
uint8_t Key_Read_Start(void)
{
    return GPIO_ReadInputDataBit(KEY_START_PORT, KEY_START_PIN);
}

/**
 * @brief   读取入口道闸按键
 * @retval  0: 按下，1: 未按下
 */
uint8_t Key_Read_Entry(void)
{
    return GPIO_ReadInputDataBit(KEY_ENTRY_PORT, KEY_ENTRY_PIN);
}

/**
 * @brief   读取出口道闸按键
 * @retval  0: 按下，1: 未按下
 */
uint8_t Key_Read_Exit(void)
{
    return GPIO_ReadInputDataBit(KEY_EXIT_PORT, KEY_EXIT_PIN);
}

/**
 * @brief   读取计数复位按键
 * @retval  0: 按下，1: 未按下
 */
uint8_t Key_Read_CountRst(void)
{
    return GPIO_ReadInputDataBit(KEY_COUNT_RST_PORT, KEY_COUNT_RST_PIN);
}

/**
 * @brief   读取菜单按键
 * @retval  0: 按下，1: 未按下
 */
uint8_t Key_Read_Menu(void)
{
    return GPIO_ReadInputDataBit(KEY_MENU_PORT, KEY_MENU_PIN);
}

/**
 * @brief   读取上翻按键
 * @retval  0: 按下，1: 未按下
 */
uint8_t Key_Read_Up(void)
{
    return GPIO_ReadInputDataBit(KEY_UP_PORT, KEY_UP_PIN);
}

/**
 * @brief   读取下翻按键
 * @retval  0: 按下，1: 未按下
 */
uint8_t Key_Read_Down(void)
{
    return GPIO_ReadInputDataBit(KEY_DOWN_PORT, KEY_DOWN_PIN);
}

/**
 * @brief   打开蜂鸣器
 */
void Buzzer_On(void)
{
    GPIO_SetBits(BUZZER_PORT, BUZZER_PIN);
}

/**
 * @brief   关闭蜂鸣器
 */
void Buzzer_Off(void)
{
    GPIO_ResetBits(BUZZER_PORT, BUZZER_PIN);
}

/**
 * @brief   打开报警 LED
 */
void Alarm_LED_On(void)
{
    GPIO_SetBits(ALARM_LED_PORT, ALARM_LED_PIN);
}

/**
 * @brief   关闭报警 LED
 */
void Alarm_LED_Off(void)
{
    GPIO_ResetBits(ALARM_LED_PORT, ALARM_LED_PIN);
}
