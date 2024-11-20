#include "lsm6dso.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "math.h"

QueueHandle_t imu_evt_queue = NULL;
QueueHandle_t imu_data_queue = NULL;

static const char *TAG = "LSM6DSO";
static void imu_task(void *pvParameter);

esp_err_t lsm6dso_register_read(uint8_t reg_addr, uint8_t *data, size_t len)
{
    return i2c_master_write_read_device(I2C_MASTER_NUM, LSM6DSO_SENSOR_ADDR,  &reg_addr, 1, data, len, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
}

esp_err_t lsm6dso_register_write_byte(uint8_t reg_addr, uint8_t data)
{
    uint8_t write_buf[2] = {reg_addr, data};

    return i2c_master_write_to_device(I2C_MASTER_NUM, LSM6DSO_SENSOR_ADDR, write_buf, sizeof(write_buf), I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
}

esp_err_t lsm6dso_read_register(uint8_t reg_addr, uint8_t *data, size_t len) {
    // 写入寄存器地址
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (LSM6DSO_SENSOR_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg_addr, true);
    i2c_master_start(cmd);  // 重复起始信号
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
        lsm6dso_register_read(LSM6DSO_REG_WHO_AM_I, &id ,1);
        ESP_LOGI(TAG,"get lsm6dso id is %d",id);
    }
    ESP_LOGI(TAG, "LSM6DSO OK!");

    vTaskDelay(10 / portTICK_PERIOD_MS);
    // lsm6dso_register_write_byte(LSM6DSO_REG_INT1_CTRL, 0x01); // CTRL1 设置地址自动增加
    // ESP_ERROR_CHECK(lsm6dso_register_write_byte(LSM6DSO_REG_CTRL1_XL, 0x60)); // CTRL7 允许加速度和陀螺仪
    // // lsm6dso_register_write_byte(LSM6DSO_REG_INT1_CTRL, 0x02); // CTRL2 设置ACC 4g 250Hz
    // ESP_ERROR_CHECK(lsm6dso_register_write_byte(LSM6DSO_REG_CTRL2_G, 0x60)); // CTRL3 设置GRY 512dps 250Hz 

    // imu_evt_queue = xQueueCreate(1, sizeof(uint32_t));
    // imu_data_queue = xQueueCreate(10, 12);

    // xTaskCreate(imu_task, "imu_task", 4096, NULL, 3, NULL);
}



// void lsm6dso_Read_AccAndGry(t_sQMI8658C *p)
// {
//     uint8_t status, data_ready=0;
//     int16_t buf[6];

//     lsm6dso_register_read(QMI8658C_STATUS0, &status, 1); // 读状态寄存器 
//     if (status & 0x03) // 判断加速度和陀螺仪数据是否可读
//     {
//         data_ready = 1;
//     }
//     if (data_ready == 1)
//     {
//         data_ready = 0;
//         lsm6dso_register_read(QMI8658C_AX_L, (uint8_t *)buf, 12); // 读加速度值
//         p->acc_x = buf[0];
//         p->acc_y = buf[1];
//         p->acc_z = buf[2];
//         p->gyr_x = buf[3];
//         p->gyr_y = buf[4];
//         p->gyr_z = buf[5];
//     }
// }

// void lsm6dso_fetch_angleFromAcc(t_sQMI8658C *p)
// {
//     float temp;

//     lsm6dso_Read_AccAndGry(p);

//     temp = (float)p->acc_x / sqrt( ((float)p->acc_y * (float)p->acc_y + (float)p->acc_z * (float)p->acc_z) );
//     p->AngleX = atan(temp)*57.3f; // 180/3.14=57.3
//     temp = (float)p->acc_y / sqrt( ((float)p->acc_x * (float)p->acc_x + (float)p->acc_z * (float)p->acc_z) );
//     p->AngleY = atan(temp)*57.3f; // 180/3.14=57.3
//     temp = (float)p->acc_z / sqrt( ((float)p->acc_x * (float)p->acc_x + (float)p->acc_y * (float)p->acc_y) );
//     p->AngleZ = atan(temp)*57.3f; // 180/3.14=57.3
// }

// static void imu_task(void *pvParameter)
// {
//     t_sQMI8658C QMI8658C;
//     while (1)
//     {
//         vTaskDelay(100 / portTICK_PERIOD_MS);
//         lsm6dso_fetch_angleFromAcc(&QMI8658C);
//         // ESP_LOGI(TAG, "angle_x = %.1f  angle_y = %.1f angle_y = %.1f",QMI8658C.AngleX, QMI8658C.AngleY, QMI8658C.AngleZ);
//         // ESP_LOGI(TAG, "gyr_x = %d  gyr_y = %d gyr_z = %d",QMI8658C.gyr_x, QMI8658C.gyr_y, QMI8658C.gyr_z);
//         struct 
//         {
//             uint32_t timestemp;
//             uint16_t x;
//             uint16_t y;
//             uint16_t z;
//         }gyr_data;
//         gyr_data.timestemp = esp_log_timestamp();
//         gyr_data.x = QMI8658C.gyr_x;
//         gyr_data.y = QMI8658C.gyr_y;
//         gyr_data.z = QMI8658C.gyr_z;
        
//         // xQueueSend(imu_data_queue,&gyr_data,10);//for data record
//         if(QMI8658C.gyr_x>8000||QMI8658C.gyr_y>8000||QMI8658C.gyr_z>8000)
//         {
//             ESP_LOGI(TAG, "gyr_x = %d  gyr_y = %d gyr_z = %d",QMI8658C.gyr_x, QMI8658C.gyr_y, QMI8658C.gyr_z);
//             uint32_t attitude_flag = 1;
//             xQueueSend(imu_evt_queue,&attitude_flag,100);
//             vTaskDelay(1000 / portTICK_PERIOD_MS);
//         }
//     }
// }