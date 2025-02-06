#include "sdkconfig.h"
#include "esp_system.h"
#include "esp_check.h"
#include "esp_err.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "gpio_func.h"

static const char *TAG = "GPIO";

QueueHandle_t gpio_evt_queue = NULL;
bool usermsg_send_start;//debug to receive imu data

static void IRAM_ATTR gpio_isr_handler(void *arg)
{
    uint32_t gpio_num = (uint32_t)arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

esp_err_t gpio_init()
{
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_NEGEDGE, // disable interrupt
        .mode = GPIO_MODE_INPUT,       // set as output mode
        .pin_bit_mask = 1 << GPIO_NUM_2,        // bit mask of the pins
        .pull_down_en = 0,              // disable pull-down mode
        .pull_up_en = 1,                // enable pull-up mode
    };
    //configure GPIO with the given settings
    gpio_config(&io_conf);

    // 配置按键引脚
    io_conf.intr_type = GPIO_INTR_NEGEDGE;  // falling edge interrupt
    io_conf.mode = GPIO_MODE_INPUT;         // set as input mode
    io_conf.pin_bit_mask = 1 << GPIO_NUM_9; // bit mask of the pins GPIO9
    io_conf.pull_down_en = 0;               // disable pull-down mode
    io_conf.pull_up_en = 1;                 // enable pull-up mode
    gpio_config(&io_conf);                  // configure GPIO with the given settings

    //配置墨水屏spi引脚
    //CS
    io_conf.intr_type = GPIO_INTR_DISABLE;  
    io_conf.mode = GPIO_MODE_OUTPUT;         
    io_conf.pin_bit_mask = 1 << GPIO_NUM_4; // bit mask of the pins GPIO4
    io_conf.pull_down_en = 0;               
    io_conf.pull_up_en = 0;                 
    gpio_config(&io_conf);                  
    //DC   
    io_conf.pin_bit_mask = 1 << GPIO_NUM_6; // bit mask of the pins GPIO4             
    gpio_config(&io_conf);

    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = 1 << GPIO_NUM_7;
    //configure GPIO with the given settings
    gpio_config(&io_conf);//复位

    // create a queue to handle gpio event from isr
    gpio_evt_queue = xQueueCreate(1, sizeof(uint32_t));

    // install gpio isr service
    gpio_install_isr_service(0);
    // hook isr handler for specific gpio pin
    // gpio_isr_handler_add(GPIO_NUM_9, gpio_isr_handler, (void *)GPIO_NUM_9);
    gpio_isr_handler_add(GPIO_NUM_2, gpio_isr_handler, (void *)GPIO_NUM_2);

    /* Initialize I2C peripheral */
    const i2c_config_t es_i2c_cfg = {
        .sda_io_num = I2C_SDA_IO,
        .scl_io_num = I2C_SCL_IO,
        .mode = I2C_MODE_MASTER,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000,//iic速率是100k
    };
    ESP_RETURN_ON_ERROR(i2c_param_config(I2C_NUM, &es_i2c_cfg), TAG, "config i2c failed");
    ESP_RETURN_ON_ERROR(i2c_driver_install(I2C_NUM, I2C_MODE_MASTER, 0, 0, 0), TAG, "install i2c driver failed");
    return ESP_OK;
}