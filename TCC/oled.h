/**
 * @file    oled.h
 * @brief   OLED 显示驱动头文件
 */

#ifndef __OLED_H__
#define __OLED_H__

#include "stm32f10x.h"
#include "i2c_soft.h"

// OLED 尺寸定义
#define OLED_WIDTH      128
#define OLED_HEIGHT     64
#define OLED_PAGE       8

// 中文字符索引定义
#define CHI_HUAN    0   // 欢
#define CHI_YING    1   // 迎
#define CHI_GUANG   2   // 光
#define CHI_LIN     3   // 临
#define CHI_CHE     4   // 车
#define CHI_WEI     5   // 位
#define CHI_YI      6   // 已
#define CHI_MAN     7   // 满
#define CHI_YI1     8   // 一
#define CHI_LU      9   // 路
#define CHI_PING    10  // 平
#define CHI_AN      11  // 安

// 函数声明
void OLED_Init(const I2C_PinDef* i2c_pin);
void OLED_Clear(const I2C_PinDef* i2c_pin);
void OLED_Display_On(const I2C_PinDef* i2c_pin);
void OLED_Display_Off(const I2C_PinDef* i2c_pin);
void OLED_Set_Pos(const I2C_PinDef* i2c_pin, uint8_t x, uint8_t y);
void OLED_Show_Char(const I2C_PinDef* i2c_pin, uint8_t x, uint8_t y, char ch);
void OLED_Show_String(const I2C_PinDef* i2c_pin, uint8_t x, uint8_t y, char* str);
void OLED_Show_Num(const I2C_PinDef* i2c_pin, uint8_t x, uint8_t y, uint32_t num);
void OLED_Show_Big_Char(const I2C_PinDef* i2c_pin, uint8_t x, uint8_t y, char ch);
void OLED_Show_Chinese(const I2C_PinDef* i2c_pin, uint8_t x, uint8_t y, uint8_t index);
void OLED_Refresh_Gram(const I2C_PinDef* i2c_pin);
void OLED_Fill(const I2C_PinDef* i2c_pin, uint8_t data);

// 显存
extern uint8_t OLED_GRAM[128][8];

#endif /* __OLED_H__ */
