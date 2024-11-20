#pragma once
#include "esp_err.h"
#include <stdint.h>

#define LSM6DSO_SENSOR_ADDR 0x6A

// LSM6DSO 寄存器地址枚举

enum LSM6DSO_Registers {
    // 设备 ID 寄存器
    LSM6DSO_REG_WHO_AM_I = 0x0F,           // 设备 ID (WHO_AM_I)

    // 控制寄存器组
    LSM6DSO_REG_CTRL1_XL = 0x10,           // 加速度计控制寄存器1
    LSM6DSO_REG_CTRL2_G = 0x11,            // 陀螺仪控制寄存器2
    LSM6DSO_REG_CTRL3_C = 0x12,            // 控制寄存器3
    LSM6DSO_REG_CTRL4_C = 0x13,            // 控制寄存器4
    LSM6DSO_REG_CTRL5_C = 0x14,            // 控制寄存器5
    LSM6DSO_REG_CTRL6_C = 0x15,            // 控制寄存器6
    LSM6DSO_REG_CTRL7_G = 0x16,            // 陀螺仪控制寄存器7
    LSM6DSO_REG_CTRL8_XL = 0x17,           // 加速度计控制寄存器8
    LSM6DSO_REG_CTRL9_XL = 0x18,           // 加速度计控制寄存器9

    // 输出数据寄存器组
    LSM6DSO_REG_OUTX_L_G = 0x22,           // 陀螺仪输出数据低字节
    LSM6DSO_REG_OUTX_H_G = 0x23,           // 陀螺仪输出数据高字节
    LSM6DSO_REG_OUTX_L_XL = 0x28,          // 加速度计输出数据低字节
    LSM6DSO_REG_OUTX_H_XL = 0x29,          // 加速度计输出数据高字节

    // 状态寄存器
    LSM6DSO_REG_STATUS_REG = 0x1E,         // 状态寄存器

    // 传感器Hub寄存器组
    LSM6DSO_REG_SENSORHUB1_REG = 0x2F,     // 传感器Hub寄存器1
    LSM6DSO_REG_SENSORHUB2_REG = 0x30,     // 传感器Hub寄存器2
    LSM6DSO_REG_SENSORHUB3_REG = 0x31,     // 传感器Hub寄存器3
    LSM6DSO_REG_SENSORHUB4_REG = 0x32,     // 传感器Hub寄存器4
    LSM6DSO_REG_SENSORHUB5_REG = 0x33,     // 传感器Hub寄存器5
    LSM6DSO_REG_SENSORHUB6_REG = 0x34,     // 传感器Hub寄存器6
    LSM6DSO_REG_SENSORHUB7_REG = 0x35,     // 传感器Hub寄存器7
    LSM6DSO_REG_SENSORHUB8_REG = 0x36,     // 传感器Hub寄存器8
    LSM6DSO_REG_SENSORHUB9_REG = 0x37,     // 传感器Hub寄存器9

    // FIFO 控制寄存器组
    LSM6DSO_REG_FIFO_CTRL1 = 0x2E,         // FIFO控制寄存器1
    LSM6DSO_REG_FIFO_CTRL2 = 0x2D,         // FIFO控制寄存器2
    LSM6DSO_REG_FIFO_STATUS = 0x3A,        // FIFO状态寄存器

    // 中断寄存器
    LSM6DSO_REG_INT1_CTRL = 0x0D,          // 中断1控制寄存器
    LSM6DSO_REG_INT2_CTRL = 0x0E,          // 中断2控制寄存器
    LSM6DSO_REG_TAP_CFG = 0x58,            // 点击检测配置寄存器
    LSM6DSO_REG_TAP_SRC = 0x59,            // 点击源寄存器

    // 其他控制寄存器
    LSM6DSO_REG_CTRL10_C = 0x19,           // 控制寄存器10
    LSM6DSO_REG_CTRL11_C = 0x1A,           // 控制寄存器11
    LSM6DSO_REG_CTRL12_C = 0x1B,           // 控制寄存器12
    LSM6DSO_REG_CTRL13_C = 0x1C,           // 控制寄存器13
    LSM6DSO_REG_CTRL14_C = 0x1D,           // 控制寄存器14

    // 备用寄存器
    LSM6DSO_REG_RESERVED = 0xFF            // 备用寄存器（仅示例）
};


typedef struct
{
    int16_t acc_x;
    int16_t acc_y;
    int16_t acc_z;
    int16_t gyr_x;
    int16_t gyr_y;
    int16_t gyr_z;
    float AngleX;
    float AngleY;
    float AngleZ;
} t_sLSM6DSO;

extern esp_err_t i2c_master_init(void);
#define I2C_MASTER_SCL_IO GPIO_NUM_1 /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO GPIO_NUM_0 /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_NUM 0             /*!< I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip */
#define I2C_MASTER_TIMEOUT_MS 1000

void lsm6dso_init(void);
void lsm6dso_fetch_angleFromAcc(t_sLSM6DSO *p);
