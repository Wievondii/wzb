/**
 * @file    i2c_soft.h
 * @brief   软件 I2C 头文件
 */

#ifndef __I2C_SOFT_H__
#define __I2C_SOFT_H__

#include "stm32f10x.h"

// I2C 引脚定义
typedef struct {
    GPIO_TypeDef* scl_port;
    uint16_t scl_pin;
    GPIO_TypeDef* sda_port;
    uint16_t sda_pin;
} I2C_PinDef;

// 外部声明两个 I2C 引脚配置
extern const I2C_PinDef I2C1_Pin;  // OLED1
extern const I2C_PinDef I2C2_Pin;  // OLED2

void I2C_Soft_Init(void);

// I2C 基本操作函数
void I2C_Soft_Start(const I2C_PinDef* i2c_pin);
void I2C_Soft_Stop(const I2C_PinDef* i2c_pin);
uint8_t I2C_Soft_WaitAck(const I2C_PinDef* i2c_pin);
void I2C_Soft_SendAck(const I2C_PinDef* i2c_pin, uint8_t ack);
void I2C_Soft_SendByte(const I2C_PinDef* i2c_pin, uint8_t byte);
uint8_t I2C_Soft_ReadByte(const I2C_PinDef* i2c_pin, uint8_t ack);

// 便捷函数
void I2C_Soft_WriteByte(const I2C_PinDef* i2c_pin, uint8_t addr, uint8_t data);
void I2C_Soft_WriteBytes(const I2C_PinDef* i2c_pin, uint8_t addr, uint8_t* data, uint16_t len);

#endif /* __I2C_SOFT_H__ */
