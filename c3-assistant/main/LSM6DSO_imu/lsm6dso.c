#include "lsm6dso.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "math.h"
#include "common.h"

QueueHandle_t imu_evt_queue = NULL;
QueueHandle_t imu_data_queue = NULL;

static const char *TAG = "LSM6DSO";
static void imu_task(void *pvParameter);

esp_err_t lsm6dso_register_read(uint8_t reg_addr, uint8_t *data, size_t len)
{
    return i2c_master_write_read_device(I2C_MASTER_NUM, LSM6DSO_SENSOR_ADDR, &reg_addr, 1, data, len, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
}

esp_err_t lsm6dso_register_write_byte(uint8_t reg_addr, uint8_t data)
{
    uint8_t write_buf[2] = {reg_addr, data};

    return i2c_master_write_to_device(I2C_MASTER_NUM, LSM6DSO_SENSOR_ADDR, write_buf, sizeof(write_buf), I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
}

esp_err_t lsm6dso_read_register(uint8_t reg_addr, uint8_t *data, size_t len)
{
    // 写入寄存器地址
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (LSM6DSO_SENSOR_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg_addr, true);
    i2c_master_start(cmd); // 重复起始信号
    i2c_master_write_byte(cmd, (LSM6DSO_SENSOR_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd, data, len, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

void lsm6dso_init(void)
{
    uint8_t id = 0;

    while (id != 0x6c)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        // lsm6dso_read_register(LSM6DSO_REG_WHO_AM_I,&id,1);
        lsm6dso_register_read(LSM6DSO_REG_WHO_AM_I, &id, 1);
        ESP_LOGI(TAG, "get lsm6dso id is %d", id);
    }
    ESP_LOGI(TAG, "LSM6DSO OK!");

    vTaskDelay(10 / portTICK_PERIOD_MS);
    lsm6dso_register_write_byte(LSM6DSO_REG_INT1_CTRL, 0x01);                 // CTRL1 设置地址自动增加
    ESP_ERROR_CHECK(lsm6dso_register_write_byte(LSM6DSO_REG_CTRL1_XL, 0x60)); // CTRL7 允许加速度和陀螺仪
    lsm6dso_register_write_byte(LSM6DSO_REG_INT1_CTRL, 0x02);                 // CTRL2 设置ACC 4g 250Hz
    ESP_ERROR_CHECK(lsm6dso_register_write_byte(LSM6DSO_REG_CTRL2_G, 0x60));  // CTRL3 设置GRY 512dps 250Hz

    imu_evt_queue = xQueueCreate(1, sizeof(uint32_t));
    imu_data_queue = xQueueCreate(10, 12);

    xTaskCreate(imu_task, "imu_task", 2048, NULL, 3, NULL);
}

// 读取温度数据
esp_err_t lsm6dso_read_temperature(float *temperature)
{
    uint8_t temp_data[2] = {0};
    int16_t raw_temp = 0;

    // 读取 OUT_TEMP寄存器
    esp_err_t ret = lsm6dso_register_read(LSM6DSO_REG_TEMP_L, temp_data, 2);
    if (ret != ESP_OK)
        return ret;

    // 合成 16 位温度数据
    raw_temp = (int16_t)((temp_data[1] << 8) | temp_data[0]);

    printf("temp is L:%x H%x",temp_data[0],temp_data[1]);

    // 转换为摄氏度
    *temperature = raw_temp / 256.0f + 25.0f;
    return ESP_OK;
}

// 主函数
void imu_task(void *pvParameter)
{
    float temperature = 0.0;
    extern EventGroupHandle_t my_event_group;
    // xEventGroupWaitBits(my_event_group, WIFI_GET_RTWEATHER_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
    while (1)
    {
        // 读取环境温度
        ESP_LOGI(TAG, "Reading temperature...");
        esp_err_t ret = lsm6dso_read_temperature(&temperature);

        if (ret == ESP_OK)
        {
            ESP_LOGI(TAG, "Temperature: %.2f °C", temperature);
        }
        else
        {
            ESP_LOGE(TAG, "Failed to read temperature. Error: %s", esp_err_to_name(ret));
        }
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
    
}
