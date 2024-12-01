#include <stdio.h>
#include "driver/i2c.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"

#define I2C_MASTER_NUM 0             
#define PCA9557_ADDR                0x18             // PCA9557 的 I²C 地址

// PCA9557寄存器地址
#define PCA9557_REG_INPUT           0x00   // 输入寄存器
#define PCA9557_REG_OUTPUT          0x01   // 输出寄存器
#define PCA9557_REG_POLARITY        0x02   // 极性反转寄存器
#define PCA9557_REG_CONFIG          0x03   // 配置寄存器

void pca9557_set_p1_high(void);

// 向PCA9557写数据
esp_err_t pca9557_write(uint8_t reg_addr, uint8_t data)
{
    esp_err_t err;
    uint8_t data_buf[2] = {reg_addr, data};
    
    err = i2c_master_write_to_device(I2C_MASTER_NUM, PCA9557_ADDR, data_buf, sizeof(data_buf), 1000 / portTICK_PERIOD_MS);
    if (err != ESP_OK) {
        ESP_LOGE("I2C", "Failed to write data to PCA9557");
    }
    else{
        ESP_LOGI("PCA9557","PCA9557 INIT OK!");
    }
    return err;
}

// 初始化PCA9557配置，设置P1为输出
void pca9557_init(void)
{
    // 设置PCA9557的配置寄存器，使P1为输出
    pca9557_write(PCA9557_REG_CONFIG, 0xFD);  // 设置IO1为输出
    pca9557_set_p1_high();
}

// 设置P1输出高电平
void pca9557_set_p1_high(void)
{
    pca9557_write(PCA9557_REG_OUTPUT, 0x02);  // 输出0x02 设置P1为高电平
}


