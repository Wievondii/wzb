/**
 * @file    i2c_soft.c
 * @brief   软件 I2C 实现
 * @note    用于驱动 OLED 显示屏
 */

#include "i2c_soft.h"
#include "delay.h"
#include "system.h"

// I2C 引脚配置
const I2C_PinDef I2C1_Pin = {
    .scl_port = OLED1_SCL_PORT,
    .scl_pin = OLED1_SCL_PIN,
    .sda_port = OLED1_SDA_PORT,
    .sda_pin = OLED1_SDA_PIN
};

const I2C_PinDef I2C2_Pin = {
    .scl_port = OLED2_SCL_PORT,
    .scl_pin = OLED2_SCL_PIN,
    .sda_port = OLED2_SDA_PORT,
    .sda_pin = OLED2_SDA_PIN
};

// 内部延时函数
static void I2C_Delay(void)
{
    Delay_us(2);
}

/**
 * @brief   初始化软件 I2C
 */
void I2C_Soft_Init(void)
{
    // GPIO 已在 gpio.c 中初始化
    // 确保初始状态为高电平
    GPIO_SetBits(I2C1_Pin.scl_port, I2C1_Pin.scl_pin);
    GPIO_SetBits(I2C1_Pin.sda_port, I2C1_Pin.sda_pin);
    GPIO_SetBits(I2C2_Pin.scl_port, I2C2_Pin.scl_pin);
    GPIO_SetBits(I2C2_Pin.sda_port, I2C2_Pin.sda_pin);
}

/**
 * @brief   产生 I2C 起始信号
 * @param   i2c_pin: I2C 引脚配置指针
 */
void I2C_Soft_Start(const I2C_PinDef* i2c_pin)
{
    GPIO_SetBits(i2c_pin->sda_port, i2c_pin->sda_pin);
    GPIO_SetBits(i2c_pin->scl_port, i2c_pin->scl_pin);
    I2C_Delay();
    GPIO_ResetBits(i2c_pin->sda_port, i2c_pin->sda_pin);
    I2C_Delay();
    GPIO_ResetBits(i2c_pin->scl_port, i2c_pin->scl_pin);
}

/**
 * @brief   产生 I2C 停止信号
 * @param   i2c_pin: I2C 引脚配置指针
 */
void I2C_Soft_Stop(const I2C_PinDef* i2c_pin)
{
    GPIO_ResetBits(i2c_pin->sda_port, i2c_pin->sda_pin);
    I2C_Delay();
    GPIO_SetBits(i2c_pin->scl_port, i2c_pin->scl_pin);
    I2C_Delay();
    GPIO_SetBits(i2c_pin->sda_port, i2c_pin->sda_pin);
    I2C_Delay();
}

/**
 * @brief   等待应答信号
 * @param   i2c_pin: I2C 引脚配置指针
 * @retval  0: 有应答，1: 无应答
 */
uint8_t I2C_Soft_WaitAck(const I2C_PinDef* i2c_pin)
{
    uint8_t ucErrTime = 0;
    
    GPIO_SetBits(i2c_pin->sda_port, i2c_pin->sda_pin);
    I2C_Delay();
    GPIO_SetBits(i2c_pin->scl_port, i2c_pin->scl_pin);
    I2C_Delay();
    
    while (GPIO_ReadInputDataBit(i2c_pin->sda_port, i2c_pin->sda_pin)) {
        ucErrTime++;
        if (ucErrTime > 250) {
            I2C_Soft_Stop(i2c_pin);
            return 1;
        }
    }
    
    GPIO_ResetBits(i2c_pin->scl_port, i2c_pin->scl_pin);
    return 0;
}

/**
 * @brief   发送应答信号
 * @param   i2c_pin: I2C 引脚配置指针
 * @param   ack: 0-应答，1-非应答
 */
void I2C_Soft_SendAck(const I2C_PinDef* i2c_pin, uint8_t ack)
{
    if (ack) {
        GPIO_SetBits(i2c_pin->sda_port, i2c_pin->sda_pin);
    } else {
        GPIO_ResetBits(i2c_pin->sda_port, i2c_pin->sda_pin);
    }
    GPIO_SetBits(i2c_pin->scl_port, i2c_pin->scl_pin);
    I2C_Delay();
    GPIO_ResetBits(i2c_pin->scl_port, i2c_pin->scl_pin);
}

/**
 * @brief   发送一个字节
 * @param   i2c_pin: I2C 引脚配置指针
 * @param   byte: 要发送的字节
 */
void I2C_Soft_SendByte(const I2C_PinDef* i2c_pin, uint8_t byte)
{
    uint8_t i;
    
    GPIO_ResetBits(i2c_pin->scl_port, i2c_pin->scl_pin);
    for (i = 0; i < 8; i++) {
        if (byte & 0x80) {
            GPIO_SetBits(i2c_pin->sda_port, i2c_pin->sda_pin);
        } else {
            GPIO_ResetBits(i2c_pin->sda_port, i2c_pin->sda_pin);
        }
        byte <<= 1;
        GPIO_SetBits(i2c_pin->scl_port, i2c_pin->scl_pin);
        I2C_Delay();
        GPIO_ResetBits(i2c_pin->scl_port, i2c_pin->scl_pin);
        I2C_Delay();
    }
}

/**
 * @brief   读取一个字节
 * @param   i2c_pin: I2C 引脚配置指针
 * @param   ack: 读取后是否发送应答 (1-发送应答，0-非应答)
 * @retval  读取的字节
 */
uint8_t I2C_Soft_ReadByte(const I2C_PinDef* i2c_pin, uint8_t ack)
{
    uint8_t i, byte = 0;
    
    GPIO_SetBits(i2c_pin->sda_port, i2c_pin->sda_pin);
    for (i = 0; i < 8; i++) {
        GPIO_SetBits(i2c_pin->scl_port, i2c_pin->scl_pin);
        I2C_Delay();
        byte <<= 1;
        if (GPIO_ReadInputDataBit(i2c_pin->sda_port, i2c_pin->sda_pin)) {
            byte |= 0x01;
        }
        GPIO_ResetBits(i2c_pin->scl_port, i2c_pin->scl_pin);
        I2C_Delay();
    }
    
    return byte;
}

/**
 * @brief   向指定地址写入一个字节
 * @param   i2c_pin: I2C 引脚配置指针
 * @param   addr: 寄存器地址
 * @param   data: 要写入的数据
 */
void I2C_Soft_WriteByte(const I2C_PinDef* i2c_pin, uint8_t addr, uint8_t data)
{
    I2C_Soft_Start(i2c_pin);
    I2C_Soft_SendByte(i2c_pin, 0x78);  // OLED 写命令地址 (0x3C << 1)
    I2C_Soft_WaitAck(i2c_pin);
    I2C_Soft_SendByte(i2c_pin, addr);  // 寄存器地址
    I2C_Soft_WaitAck(i2c_pin);
    I2C_Soft_SendByte(i2c_pin, data);  // 数据
    I2C_Soft_WaitAck(i2c_pin);
    I2C_Soft_Stop(i2c_pin);
}

/**
 * @brief   向指定地址写入多个字节
 * @param   i2c_pin: I2C 引脚配置指针
 * @param   addr: 寄存器地址
 * @param   data: 数据缓冲区指针
 * @param   len: 数据长度
 */
void I2C_Soft_WriteBytes(const I2C_PinDef* i2c_pin, uint8_t addr, uint8_t* data, uint16_t len)
{
    uint16_t i;
    
    I2C_Soft_Start(i2c_pin);
    I2C_Soft_SendByte(i2c_pin, 0x78);
    I2C_Soft_WaitAck(i2c_pin);
    I2C_Soft_SendByte(i2c_pin, addr);
    I2C_Soft_WaitAck(i2c_pin);
    
    for (i = 0; i < len; i++) {
        I2C_Soft_SendByte(i2c_pin, data[i]);
        I2C_Soft_WaitAck(i2c_pin);
    }
    
    I2C_Soft_Stop(i2c_pin);
}
