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

// 读取温度数据
int16_t lsm6dso_read_temperature()
{
    uint8_t temp_data[2] = {0};
    int16_t raw_temp = 0;

    // 读取 OUT_TEMP寄存器
    esp_err_t ret = lsm6dso_register_read(LSM6DSO_REG_TEMP_L, temp_data, 2);
    if (ret != ESP_OK)
        return ret;

    // 合成 16 位温度数据
    raw_temp = (int16_t)((temp_data[1] << 8) | temp_data[0]);

    // 转换为摄氏度
    float temperature = raw_temp / 256.0f + 25.0f;
    int16_t temp_return = (int16_t) temperature;

    printf("temp is %f\n",temperature);

    return temp_return;
}

int16_t lsm6dso_read_gyroscope()
{
    uint8_t gyro_data_raw[2] = {0};
    int16_t gyro_x_data = 0;

    // 读取 陀螺仪寄存器
    ESP_ERROR_CHECK(lsm6dso_register_read(LSM6DSO_REG_OUTX_L_G, &gyro_data_raw[0], 1));
    ESP_ERROR_CHECK(lsm6dso_register_read(LSM6DSO_REG_OUTX_H_G, &gyro_data_raw[1], 1));

    gyro_x_data = (int16_t)((gyro_data_raw[1] << 8) | gyro_data_raw[0]);

    printf("gyro is %d\n",gyro_x_data);

    return 0;
}

esp_err_t lsm6dso_set_tilt_cal(void)
{
    ESP_ERROR_CHECK(lsm6dso_register_write_byte(LSM6DSO_REG_FUNC_CFG_ACCESS, 0x80));
    ESP_ERROR_CHECK(lsm6dso_register_write_byte(LSM6DSO_REG_EMB_FUNC_EN_A, 0x10));
    ESP_ERROR_CHECK(lsm6dso_register_write_byte(LSM6DSO_REG_EMB_FUNC_INT2, 0x10));
    ESP_ERROR_CHECK(lsm6dso_register_write_byte(LSM6DSO_REG_PAGE_RW, 0x80));
    ESP_ERROR_CHECK(lsm6dso_register_write_byte(LSM6DSO_REG_FUNC_CFG_ACCESS, 0x00));
    ESP_ERROR_CHECK(lsm6dso_register_write_byte(LSM6DSO_REG_MD2_CFG, 0x02));
    ESP_ERROR_CHECK(lsm6dso_register_write_byte(LSM6DSO_REG_CTRL1_XL, 0x20));

    return ESP_OK;
}

esp_err_t lsm6dso_check_tilt_state(void)
{
    uint8_t tilt_state = 0;
    ESP_ERROR_CHECK(lsm6dso_register_write_byte(LSM6DSO_REG_FUNC_CFG_ACCESS, 0x80));
    ESP_ERROR_CHECK(lsm6dso_register_read(LSM6DSO_REG_EMB_FUNC_STATUS, &tilt_state, 1));
    ESP_ERROR_CHECK(lsm6dso_register_write_byte(LSM6DSO_REG_FUNC_CFG_ACCESS, 0x00));
    if (tilt_state != 0)
        printf("tilt state is %d\n", tilt_state);
    return ESP_OK;
}

esp_err_t lsm6dso_set_significant_motion_det(void)
{
    ESP_ERROR_CHECK(lsm6dso_register_write_byte(LSM6DSO_REG_FUNC_CFG_ACCESS, 0x80));
    ESP_ERROR_CHECK(lsm6dso_register_write_byte(LSM6DSO_REG_EMB_FUNC_EN_A, 0x20));
    ESP_ERROR_CHECK(lsm6dso_register_write_byte(LSM6DSO_REG_EMB_FUNC_INT2, 0x20));
    ESP_ERROR_CHECK(lsm6dso_register_write_byte(LSM6DSO_REG_PAGE_RW, 0x80));
    ESP_ERROR_CHECK(lsm6dso_register_write_byte(LSM6DSO_REG_FUNC_CFG_ACCESS, 0x00));
    ESP_ERROR_CHECK(lsm6dso_register_write_byte(LSM6DSO_REG_MD2_CFG, 0x02));
    ESP_ERROR_CHECK(lsm6dso_register_write_byte(LSM6DSO_REG_CTRL1_XL, 0x20));

    return ESP_OK;
}

esp_err_t lsm6dso_check_motion_state(void)
{
    uint8_t motion_state = 0;
    ESP_ERROR_CHECK(lsm6dso_register_write_byte(LSM6DSO_REG_FUNC_CFG_ACCESS, 0x80));
    ESP_ERROR_CHECK(lsm6dso_register_read(LSM6DSO_REG_EMB_FUNC_STATUS, &motion_state, 1));
    ESP_ERROR_CHECK(lsm6dso_register_write_byte(LSM6DSO_REG_FUNC_CFG_ACCESS, 0x00));
    if (motion_state != 0)
        printf("motion state is %d\n", motion_state);
    return ESP_OK;
}

esp_err_t lsm6dso_set_6D_det(void)
{
    ESP_ERROR_CHECK(lsm6dso_register_write_byte(LSM6DSO_REG_CTRL1_XL, 0x60));
    ESP_ERROR_CHECK(lsm6dso_register_write_byte(LSM6DSO_REG_TAP_CFG0, 0x41));
    ESP_ERROR_CHECK(lsm6dso_register_write_byte(LSM6DSO_REG_TAP_CFG2, 0x80));
    ESP_ERROR_CHECK(lsm6dso_register_write_byte(LSM6DSO_REG_TAP_THS_6D, 0x40));
    ESP_ERROR_CHECK(lsm6dso_register_write_byte(LSM6DSO_REG_CTRL8_XL, 0x01));
    ESP_ERROR_CHECK(lsm6dso_register_write_byte(LSM6DSO_REG_MD2_CFG, 0x04));

    return ESP_OK;
}

esp_err_t lsm6dso_check_6D_state(void)
{
    uint8_t get_6D_state = 0;
    ESP_ERROR_CHECK(lsm6dso_register_read(LSM6DSO_REG_D6D_SRC, &get_6D_state, 1));
    if (get_6D_state != 0x20)
        printf("6d state is 0x%x\n", get_6D_state);
    return ESP_OK;
}

esp_err_t lsm6dso_set_active_det(void)
{
    ESP_ERROR_CHECK(lsm6dso_register_write_byte(LSM6DSO_REG_CTRL1_XL, 0x51));
    ESP_ERROR_CHECK(lsm6dso_register_write_byte(LSM6DSO_REG_CTRL2_G, 0x40));
    ESP_ERROR_CHECK(lsm6dso_register_write_byte(LSM6DSO_REG_WAKE_UP_DUR, 0x72));
    ESP_ERROR_CHECK(lsm6dso_register_write_byte(LSM6DSO_REG_WAKE_UP_THS, 0x05));
    ESP_ERROR_CHECK(lsm6dso_register_write_byte(LSM6DSO_REG_TAP_CFG0, 0x30));
    ESP_ERROR_CHECK(lsm6dso_register_write_byte(LSM6DSO_REG_TAP_CFG2, 0xE0));
    ESP_ERROR_CHECK(lsm6dso_register_write_byte(LSM6DSO_REG_MD2_CFG, 0x80));

    return ESP_OK;
}

esp_err_t lsm6dso_check_active_state(void)
{
    uint8_t get_active_state = 0;
    ESP_ERROR_CHECK(lsm6dso_register_read(LSM6DSO_REG_WAKE_UP_SRC, &get_active_state, 1));
    if (get_active_state != 0x0)
        printf("active state is 0x%x\n", get_active_state);
    return ESP_OK;
}

void lsm6dso_init(void)
{
    uint8_t id = 0;

    while (id != 0x6c)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        lsm6dso_register_read(LSM6DSO_REG_WHO_AM_I, &id, 1);
        ESP_LOGI(TAG, "get lsm6dso id is %d", id);
    }
    ESP_LOGI(TAG, "LSM6DSO OK!");

    vTaskDelay(10 / portTICK_PERIOD_MS);

    lsm6dso_set_active_det();

    // xTaskCreate(imu_task, "imu_task", 2048, NULL, 3, NULL);
}

// 任务函数,调试用
void imu_task(void *pvParameter)
{
    float temperature = 0.0;
    extern EventGroupHandle_t my_event_group;
    // lsm6dso_set_active_det();
    extern int8_t door_state;
    int8_t last_door_state = door_state;
    while (1)
    {
        extern QueueHandle_t gpio_evt_queue;

        // if(door_state != last_door_state)
            // printf("door state is %d\n", door_state);
        // last_door_state = door_state;
        uint32_t io_num;
        static uint16_t isr_times = 0;
        if ((xQueueReceive(gpio_evt_queue, &io_num, 10)==pdTRUE))
        {
            isr_times++;
            printf("get gpio isr total:%d\n",isr_times);
            lsm6dso_read_gyroscope();
        }
        // lsm6dso_read_gyroscope();
        // 读取环境温度
        // ESP_LOGI(TAG, "Reading temperature...");
        // esp_err_t ret = lsm6dso_read_temperature(&temperature);

        // if (ret == ESP_OK)
        // {
        //     ESP_LOGI(TAG, "Temperature: %.2f °C", temperature);
        // }
        // else
        // {
        //     ESP_LOGE(TAG, "Failed to read temperature. Error: %s", esp_err_to_name(ret));
        // }
        // lsm6dso_check_active_state();
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}


