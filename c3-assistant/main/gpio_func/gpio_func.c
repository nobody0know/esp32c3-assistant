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
bool usermsg_send_start;

static void IRAM_ATTR gpio_isr_handler(void *arg)
{
    uint32_t gpio_num = (uint32_t)arg;
    usermsg_send_start = 1;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

// static void gpio_task(void* arg)
// {
//     uint32_t io_num;
//     for(;;) {

//     }
// }

esp_err_t gpio_init()
{
    // 特别要注意I2S引脚当中的GPIO11，这个引脚目前还是VDD_SPI引脚，默认是一个电源引脚，输出3.3V
    // 我们需要把它变成GPIO11才可以使用。这个变化是不可逆的，变成GPIO11以后，就不能再变成VDD_SPI引脚了。
    /// 设计产品的时候注意，如果这个引脚用作VDD_SPI给外部FLASH供电，千万不要把它搞成GPIO11，否则就得换芯片了。
    // 开发板上只把它用做GPIO11，I2S_DO引脚。把它变成GPIO11，需要调用一个函数即可。
    //  printf("ESP_EFUSE_VDD_SPI_AS_GPIO start\n-----------------------------\n");
    //  esp_efuse_write_field_bit(ESP_EFUSE_VDD_SPI_AS_GPIO);//只需要执行一次

    // 初始化PA芯片NS4150B控制引脚 低电平关闭音频输出 高电平允许音频输出
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE, // disable interrupt
        .mode = GPIO_MODE_OUTPUT,       // set as output mode
        .pin_bit_mask = 1 << GPIO_NUM_13,        // bit mask of the pins
        .pull_down_en = 0,              // disable pull-down mode
        .pull_up_en = 1,                // enable pull-up mode
    };
    // configure GPIO with the given settings
    gpio_config(&io_conf);
    gpio_set_level(GPIO_NUM_13, 1); // out put high level

    vTaskDelay(pdMS_TO_TICKS(500));

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
 
    io_conf.intr_type = GPIO_INTR_NEGEDGE;//这里为啥要用下降沿来进行中断触发，存疑
    //bit mask of the pins, use GPIO4/5 here
    io_conf.pin_bit_mask = 1 << GPIO_NUM_2;
    //set as input mode    
    io_conf.mode = GPIO_MODE_INPUT;
    //enable pull-up mode
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

    // create a queue to handle gpio event from isr
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    // start gpio task
    // xTaskCreate(gpio_task, "gpio_task_example", 2048, NULL, 10, NULL);

    // install gpio isr service
    gpio_install_isr_service(0);
    // hook isr handler for specific gpio pin
    gpio_isr_handler_add(GPIO_NUM_9, gpio_isr_handler, (void *)GPIO_NUM_9);

    /* Initialize I2C peripheral */
    const i2c_config_t es_i2c_cfg = {
        .sda_io_num = I2C_SDA_IO,
        .scl_io_num = I2C_SCL_IO,
        .mode = I2C_MODE_MASTER,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000,
    };
    ESP_RETURN_ON_ERROR(i2c_param_config(I2C_NUM, &es_i2c_cfg), TAG, "config i2c failed");
    ESP_RETURN_ON_ERROR(i2c_driver_install(I2C_NUM, I2C_MODE_MASTER, 0, 0, 0), TAG, "install i2c driver failed");
    return ESP_OK;
}