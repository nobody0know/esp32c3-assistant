#pragma once
#include "esp_err.h"
#include <stdint.h>
#define LSM6DSOW_SENSOR_ADDR 0x6A
// LSM6DSO 寄存器地址枚举
enum LSM6DSO_Registers
{
    // 设备 ID 寄存器
    LSM6DSOW_REG_WHO_AM_I = 0x0F,
    // 功能配置访问寄存器
    LSM6DSOW_REG_FUNC_CFG_ACCESS = 0x01,
    // 引脚控制寄存器
    LSM6DSOW_REG_PIN_CTRL = 0x02,
    // FIFO控制寄存器1
    LSM6DSOW_REG_FIFO_CTRL1 = 0x07,
    // FIFO控制寄存器2
    LSM6DSOW_REG_FIFO_CTRL2 = 0x08,
    // FIFO控制寄存器3
    LSM6DSOW_REG_FIFO_CTRL3 = 0x09,
    // FIFO控制寄存器4
    LSM6DSOW_REG_FIFO_CTRL4 = 0x0A,
    // 计数器批处理数据速率寄存器1
    LSM6DSOW_REG_COUNTER_BDR_REG1 = 0x0B,
    // 计数器批处理数据速率寄存器2
    LSM6DSOW_REG_COUNTER_BDR_REG2 = 0x0C,
    // INT1引脚控制寄存器
    LSM6DSOW_REG_INT1_CTRL = 0x0D,
    // INT2引脚控制寄存器
    LSM6DSOW_REG_INT2_CTRL = 0x0E,
    // 加速度计控制寄存器1
    LSM6DSOW_REG_CTRL1_XL = 0x10,
    // 陀螺仪控制寄存器2
    LSM6DSOW_REG_CTRL2_G = 0x11,
    // 控制寄存器3
    LSM6DSOW_REG_CTRL3_C = 0x12,
    // 控制寄存器4
    LSM6DSOW_REG_CTRL4_C = 0x13,
    // 控制寄存器5
    LSM6DSOW_REG_CTRL5_C = 0x14,
    // 控制寄存器6
    LSM6DSOW_REG_CTRL6_C = 0x15,
    // 控制寄存器7
    LSM6DSOW_REG_CTRL7_G = 0x16,
    // 控制寄存器8
    LSM6DSOW_REG_CTRL8_XL = 0x17,
    // 控制寄存器9
    LSM6DSOW_REG_CTRL9_XL = 0x18,
    // 控制寄存器10
    LSM6DSOW_REG_CTRL10_C = 0x19,
    // 所有中断源寄存器
    LSM6DSOW_REG_ALL_INT_SRC = 0x1A,
    // 唤醒中断源寄存器
    LSM6DSOW_REG_WAKE_UP_SRC = 0x1B,
    // 敲击源寄存器
    LSM6DSOW_REG_TAP_SRC = 0x1C,
    // 6D方向源寄存器
    LSM6DSOW_REG_D6D_SRC = 0x1D,
    // 状态寄存器
    LSM6DSOW_REG_STATUS_REG = 0x1E,
    // 温度数据输出寄存器（低字节）
    LSM6DSOW_REG_OUT_TEMP_L = 0x20,
    // 温度数据输出寄存器（高字节）
    LSM6DSOW_REG_OUT_TEMP_H = 0x21,
    // 陀螺仪X轴角速度输出寄存器（低字节）
    LSM6DSOW_REG_OUTX_L_G = 0x22,
    // 陀螺仪X轴角速度输出寄存器（高字节）
    LSM6DSOW_REG_OUTX_H_G = 0x23,
    // 陀螺仪Y轴角速度输出寄存器（低字节）
    LSM6DSOW_REG_OUTY_L_G = 0x24,
    // 陀螺仪Y轴角速度输出寄存器（高字节）
    LSM6DSOW_REG_OUTY_H_G = 0x25,
    // 陀螺仪Z轴角速度输出寄存器（低字节）
    LSM6DSOW_REG_OUTZ_L_G = 0x26,
    // 陀螺仪Z轴角速度输出寄存器（高字节）
    LSM6DSOW_REG_OUTZ_H_G = 0x27,
    // 加速度计X轴输出寄存器（低字节）
    LSM6DSOW_REG_OUTX_L_A = 0x28,
    // 加速度计X轴输出寄存器（高字节）
    LSM6DSOW_REG_OUTX_H_A = 0x29,
    // 加速度计Y轴输出寄存器（低字节）
    LSM6DSOW_REG_OUTY_L_A = 0x2A,
    // 加速度计Y轴输出寄存器（高字节）
    LSM6DSOW_REG_OUTY_H_A = 0x2B,
    // 加速度计Z轴输出寄存器（低字节）
    LSM6DSOW_REG_OUTZ_L_A = 0x2C,
    // 加速度计Z轴输出寄存器（高字节）
    LSM6DSOW_REG_OUTZ_H_A = 0x2D,
    // 嵌入式功能状态主页面寄存器
    LSM6DSOW_REG_EMB_FUNC_STATUS_MAINPAGE = 0x35,
    // 有限状态机状态A主页面寄存器
    LSM6DSOW_REG_FSM_STATUS_A_MAINPAGE = 0x36,
    // 有限状态机状态B主页面寄存器
    LSM6DSOW_REG_FSM_STATUS_B_MAINPAGE = 0x37,
    // 状态主页面寄存器
    LSM6DSOW_REG_STATUS_MASTER_MAINPAGE = 0x39,
    // FIFO状态寄存器1
    LSM6DSOW_REG_FIFO_STATUS1 = 0x3A,
    // FIFO状态寄存器2
    LSM6DSOW_REG_FIFO_STATUS2 = 0x3B,
    // 时间戳输出寄存器0
    LSM6DSOW_REG_TIMESTAMP0 = 0x40,
    // 时间戳输出寄存器1
    LSM6DSOW_REG_TIMESTAMP1 = 0x41,
    // 时间戳输出寄存器2
    LSM6DSOW_REG_TIMESTAMP2 = 0x42,
    // 时间戳输出寄存器3
    LSM6DSOW_REG_TIMESTAMP3 = 0x43,
    // 敲击配置寄存器0
    LSM6DSOW_REG_TAP_CFG0 = 0x56,
    // 敲击配置寄存器1
    LSM6DSOW_REG_TAP_CFG1 = 0x57,
    // 敲击配置寄存器2
    LSM6DSOW_REG_TAP_CFG2 = 0x58,
    // 6D位置和敲击功能阈值寄存器
    LSM6DSOW_REG_TAP_THS_6D = 0x59,
    // 敲击识别功能设置寄存器
    LSM6DSOW_REG_INT_DUR2 = 0x5A,
    // 唤醒配置寄存器
    LSM6DSOW_REG_WAKE_UP_THS = 0x5B,
    // 唤醒和睡眠模式功能持续时间设置寄存器
    LSM6DSOW_REG_WAKE_UP_DUR = 0x5C,
    // 自由落体功能持续时间设置寄存器
    LSM6DSOW_REG_FREE_FALL = 0x5D,
    // INT1功能路由寄存器
    LSM6DSOW_REG_MD1_CFG = 0x5E,
    // INT2功能路由寄存器
    LSM6DSOW_REG_MD2_CFG = 0x5F,
    // I3C总线可用时间选择寄存器
    LSM6DSOW_REG_I3C_BUS_AVB = 0x62,
    // 内部频率微调寄存器
    LSM6DSOW_REG_INTERNAL_FREQ_FINE = 0x63,
    // 加速度计X轴用户偏移校正寄存器
    LSM6DSOW_REG_X_OFS_USR = 0x73,
    // 加速度计Y轴用户偏移校正寄存器
    LSM6DSOW_REG_Y_OFS_USR = 0x74,
    // 加速度计Z轴用户偏移校正寄存器
    LSM6DSOW_REG_Z_OFS_USR = 0x75,
    // FIFO标签寄存器
    LSM6DSOW_REG_FIFO_DATA_OUT_TAG = 0x78,
    // FIFO数据输出X轴（低字节）
    LSM6DSOW_REG_FIFO_DATA_OUT_X_L = 0x79,
    // FIFO数据输出X轴（高字节）
    LSM6DSOW_REG_FIFO_DATA_OUT_X_H = 0x7A,
    // FIFO数据输出Y轴（低字节）
    LSM6DSOW_REG_FIFO_DATA_OUT_Y_L = 0x7B,
    // FIFO数据输出Y轴（高字节）
    LSM6DSOW_REG_FIFO_DATA_OUT_Y_H = 0x7C,
    // FIFO数据输出Z轴（低字节）
    LSM6DSOW_REG_FIFO_DATA_OUT_Z_L = 0x7D,
    // FIFO数据输出Z轴（高字节）
    LSM6DSOW_REG_FIFO_DATA_OUT_Z_H = 0x7E,
    // 嵌入式功能页选择寄存器
    LSM6DSOW_REG_PAGE_SEL = 0x02,
    // 嵌入式功能使能寄存器A
    LSM6DSOW_REG_EMB_FUNC_EN_A = 0x04,
    // 嵌入式功能使能寄存器B
    LSM6DSOW_REG_EMB_FUNC_EN_B = 0x05,
    // 页面地址寄存器
    LSM6DSOW_REG_PAGE_ADDRESS = 0x08,
    // 页面值寄存器
    LSM6DSOW_REG_PAGE_VALUE = 0x09,
    // INT1引脚嵌入式功能控制寄存器
    LSM6DSOW_REG_EMB_FUNC_INT1 = 0x0A,
    // FSM INT1 A寄存器
    LSM6DSOW_REG_FSM_INT1_A = 0x0B,
    // FSM INT1 B寄存器
    LSM6DSOW_REG_FSM_INT1_B = 0x0C,
    // INT2引脚嵌入式功能控制寄存器
    LSM6DSOW_REG_EMB_FUNC_INT2 = 0x0E,
    // FSM INT2 A寄存器
    LSM6DSOW_REG_FSM_INT2_A = 0x0F,
    // FSM INT2 B寄存器
    LSM6DSOW_REG_FSM_INT2_B = 0x10,
    // 嵌入式功能状态寄存器
    LSM6DSOW_REG_EMB_FUNC_STATUS = 0x12,
    // 有限状态机状态A寄存器
    LSM6DSOW_REG_FSM_STATUS_A = 0x13,
    // 有限状态机状态B寄存器
    LSM6DSOW_REG_FSM_STATUS_B = 0x14,
    // 页面读写寄存器
    LSM6DSOW_REG_PAGE_RW = 0x17,
    // 嵌入式功能FIFO配置寄存器
    LSM6DSOW_REG_EMB_FUNC_FIFO_CFG = 0x44,
    // FSM使能寄存器A
    LSM6DSOW_REG_FSM_ENABLE_A = 0x46,
    // FSM使能寄存器B
    LSM6DSOW_REG_FSM_ENABLE_B = 0x47,
    // FSM长计数器低字节寄存器
    LSM6DSOW_REG_FSM_LONG_COUNTER_L = 0x48,
    // FSM长计数器高字节寄存器
    LSM6DSOW_REG_FSM_LONG_COUNTER_H = 0x49,
    // FSM长计数器清除寄存器
    LSM6DSOW_REG_FSM_LONG_COUNTER_CLEAR = 0x4A,
    // FSM输出1寄存器
    LSM6DSOW_REG_FSM_OUTS1 = 0x4C,
    // FSM输出2寄存器
    LSM6DSOW_REG_FSM_OUTS2 = 0x4D,
    // FSM输出3寄存器
    LSM6DSOW_REG_FSM_OUTS3 = 0x4E,
    // FSM输出4寄存器
    LSM6DSOW_REG_FSM_OUTS4 = 0x4F,
    // FSM输出5寄存器
    LSM6DSOW_REG_FSM_OUTS5 = 0x50,
    // FSM输出6寄存器
    LSM6DSOW_REG_FSM_OUTS6 = 0x51,
    // FSM输出7寄存器
    LSM6DSOW_REG_FSM_OUTS7 = 0x52,
    // FSM输出8寄存器
    LSM6DSOW_REG_FSM_OUTS8 = 0x53,
    // FSM输出9寄存器
    LSM6DSOW_REG_FSM_OUTS9 = 0x54,
    // FSM输出10寄存器
    LSM6DSOW_REG_FSM_OUTS10 = 0x55,
    // FSM输出11寄存器
    LSM6DSOW_REG_FSM_OUTS11 = 0x56,
    // FSM输出12寄存器
    LSM6DSOW_REG_FSM_OUTS12 = 0x57,
    // FSM输出13寄存器
    LSM6DSOW_REG_FSM_OUTS13 = 0x58,
    // FSM输出14寄存器
    LSM6DSOW_REG_FSM_OUTS14 = 0x59,
    // FSM输出15寄存器
    LSM6DSOW_REG_FSM_OUTS15 = 0x5A,
    // FSM输出16寄存器
    LSM6DSOW_REG_FSM_OUTS16 = 0x5B,
    // 有限状态机输出数据速率配置寄存器
    LSM6DSOW_REG_EMB_FUNC_ODR_CFG_B = 0x5F,
    // 步数计数器低字节寄存器
    LSM6DSOW_REG_STEP_COUNTER_L = 0x62,
    // 步数计数器高字节寄存器
    LSM6DSOW_REG_STEP_COUNTER_H = 0x63,
    // 嵌入式功能源寄存器
    LSM6DSOW_REG_EMB_FUNC_SRC = 0x64,
    // 嵌入式功能初始化寄存器A
    LSM6DSOW_REG_EMB_FUNC_INIT_A = 0x66,
    // 嵌入式功能初始化寄存器B
    LSM6DSOW_REG_EMB_FUNC_INIT_B = 0x67,
    // 外部磁力计灵敏度值寄存器（低字节）
    LSM6DSOW_REG_MAG_SENSITIVITY_L = 0xBA,
    // 外部磁力计灵敏度值寄存器（高字节）
    LSM6DSOW_REG_MAG_SENSITIVITY_H = 0xBB,
    // X轴硬铁补偿偏移寄存器（低字节）
    LSM6DSOW_REG_MAG_OFFX_L = 0xC0,
    // X轴硬铁补偿偏移寄存器（高字节）
    LSM6DSOW_REG_MAG_OFFX_H = 0xC1,
    // Y轴硬铁补偿偏移寄存器（低字节）
    LSM6DSOW_REG_MAG_OFFY_L = 0xC2,
    // Y轴硬铁补偿偏移寄存器（高字节）
    LSM6DSOW_REG_MAG_OFFY_H = 0xC3,
    // Z轴硬铁补偿偏移寄存器（低字节）
    LSM6DSOW_REG_MAG_OFFZ_L = 0xC4,
    // Z轴硬铁补偿偏移寄存器（高字节）
    LSM6DSOW_REG_MAG_OFFZ_H = 0xC5,
    // 软铁矩阵校正寄存器XX（低字节）
    LSM6DSOW_REG_MAG_SI_XX_L = 0xC6,
    // 软铁矩阵校正寄存器XX（高字节）
    LSM6DSOW_REG_MAG_SI_XX_H = 0xC7,
    // 软铁矩阵校正寄存器XY（低字节）
    LSM6DSOW_REG_MAG_SI_XY_L = 0xC8,
    // 软铁矩阵校正寄存器XY（高字节）
    LSM6DSOW_REG_MAG_SI_XY_H = 0xC9,
    // 软铁矩阵校正寄存器XZ（低字节）
    LSM6DSOW_REG_MAG_SI_XZ_L = 0xCA,
    // 软铁矩阵校正寄存器XZ（高字节）
    LSM6DSOW_REG_MAG_SI_XZ_H = 0xCB,
    // 软铁矩阵校正寄存器YY（低字节）
    LSM6DSOW_REG_MAG_SI_YY_L = 0xCC,
    // 软铁矩阵校正寄存器YY（高字节）
    LSM6DSOW_REG_MAG_SI_YY_H = 0xCD,
    // 软铁矩阵校正寄存器YZ（低字节）
    LSM6DSOW_REG_MAG_SI_YZ_L = 0xCE,
    // 软铁矩阵校正寄存器YZ（高字节）
    LSM6DSOW_REG_MAG_SI_YZ_H = 0xCF,
    // 软铁矩阵校正寄存器ZZ（低字节）
    LSM6DSOW_REG_MAG_SI_ZZ_L = 0xD0,
    // 软铁矩阵校正寄存器ZZ（高字节）
    LSM6DSOW_REG_MAG_SI_ZZ_H = 0xD1,
    // 外部磁力计坐标旋转寄存器A
    LSM6DSOW_REG_MAG_CFG_A = 0xD4,
    // 外部磁力计坐标旋转寄存器B
    LSM6DSOW_REG_MAG_CFG_B = 0xD5,
    // FSM长计数器超时寄存器（低字节）
    LSM6DSOW_REG_FSM_LC_TIMEOUT_L = 0x7A,
    // FSM长计数器超时寄存器（高字节）
    LSM6DSOW_REG_FSM_LC_TIMEOUT_H = 0x7B,
    // FSM程序数量寄存器
    LSM6DSOW_REG_FSM_PROGRAMS = 0x7C,
    // FSM起始地址寄存器（低字节）
    LSM6DSOW_REG_FSM_START_ADD_L = 0x7E,
    // FSM起始地址寄存器（高字节）
    LSM6DSOW_REG_FSM_START_ADD_H = 0x7F,
    // 计步器配置寄存器
    LSM6DSOW_REG_PEDO_CMD_REG = 0x83,
    // 计步器防抖配置寄存器
    LSM6DSOW_REG_PEDO_DEB_STEPS_CONF = 0x84,
    // 计步器时间周期寄存器（低字节）
    LSM6DSOW_REG_PEDO_SC_DELTAT_L = 0xD0,
    // 计步器时间周期寄存器（高字节）
    LSM6DSOW_REG_PEDO_SC_DELTAT_H = 0xD1,
    // 传感器集线器输出寄存器1
    LSM6DSOW_REG_SENSOR_HUB_1 = 0x02,
    // 传感器集线器输出寄存器2
    LSM6DSOW_REG_SENSOR_HUB_2 = 0x03,
    // 传感器集线器输出寄存器3
    LSM6DSOW_REG_SENSOR_HUB_3 = 0x04,
    // 传感器集线器输出寄存器4
    LSM6DSOW_REG_SENSOR_HUB_4 = 0x05,
    // 传感器集线器输出寄存器5
    LSM6DSOW_REG_SENSOR_HUB_5 = 0x06,
    // 传感器集线器输出寄存器6
    LSM6DSOW_REG_SENSOR_HUB_6 = 0x07,
    // 传感器集线器输出寄存器7
    LSM6DSOW_REG_SENSOR_HUB_7 = 0x08,
    // 传感器集线器输出寄存器8
    LSM6DSOW_REG_SENSOR_HUB_8 = 0x09,
    // 传感器集线器输出寄存器9
    LSM6DSOW_REG_SENSOR_HUB_9 = 0x0A,
    // 传感器集线器输出寄存器10
    LSM6DSOW_REG_SENSOR_HUB_10 = 0x0B,
    // 传感器集线器输出寄存器11
    LSM6DSOW_REG_SENSOR_HUB_11 = 0x0C,
    // 传感器集线器输出寄存器12
    LSM6DSOW_REG_SENSOR_HUB_12 = 0x0D,
    // 传感器集线器输出寄存器13
    LSM6DSOW_REG_SENSOR_HUB_13 = 0x0E,
    // 传感器集线器输出寄存器14
    LSM6DSOW_REG_SENSOR_HUB_14 = 0x0F,
    // 传感器集线器输出寄存器15
    LSM6DSOW_REG_SENSOR_HUB_15 = 0x10,
    // 传感器集线器输出寄存器16
    LSM6DSOW_REG_SENSOR_HUB_16 = 0x11,
    // 传感器集线器输出寄存器17
    LSM6DSOW_REG_SENSOR_HUB_17 = 0x12,
    // 传感器集线器输出寄存器18
    LSM6DSOW_REG_SENSOR_HUB_18 = 0x13,
    // 主配置寄存器
    LSM6DSOW_REG_MASTER_CONFIG = 0x14,
    // 从设备0地址寄存器
    LSM6DSOW_REG_SLVO_ADD = 0x15,
    // 从设备0子地址寄存器
    LSM6DSOW_REG_SLVO_SUBADD = 0x16,
    // 从设备0配置寄存器
    LSM6DSOW_REG_SLAVE0_CONFIG = 0x17,
    // 从设备1地址寄存器
    LSM6DSOW_REG_SLV1_ADD = 0x18,
    // 从设备1子地址寄存器
    LSM6DSOW_REG_SLV1_SUBADD = 0x19,
    // 从设备1配置寄存器
    LSM6DSOW_REG_SLAVE1_CONFIG = 0x1A,
    // 从设备2地址寄存器
    LSM6DSOW_REG_SLV2_ADD = 0x1B,
    // 从设备2子地址寄存器
    LSM6DSOW_REG_SLV2_SUBADD = 0x1C,
    // 从设备2配置寄存器
    LSM6DSOW_REG_SLAVE2_CONFIG = 0x1D,
    // 从设备3地址寄存器
    LSM6DSOW_REG_SLV3_ADD = 0x1E,
    // 从设备3子地址寄存器
    LSM6DSOW_REG_SLV3_SUBADD = 0x1F,
    // 从设备3配置寄存器
    LSM6DSOW_REG_SLAVE3_CONFIG = 0x20,
    // 写入从设备0数据寄存器
    LSM6DSOW_REG_DATAWRITE_SRC_MODE_SUB_SLV0 = 0x21,
    // 状态主寄存器
    LSM6DSOW_REG_STATUS_MASTER = 0x22
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
void lsm6dso_read_gyroscope(int16_t *gyro_x_data, int16_t *gyro_y_data, int16_t *gyro_z_data);
int16_t lsm6dso_read_temperature();