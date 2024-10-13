#include "esp_err.h"
/* I2C port and GPIOs */
#define I2C_NUM         (0)
#if CONFIG_IDF_TARGET_ESP32 || CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3
#define I2C_SCL_IO      (GPIO_NUM_16)
#define I2C_SDA_IO      (GPIO_NUM_17)
#elif CONFIG_IDF_TARGET_ESP32H2
#define I2C_SCL_IO      (GPIO_NUM_8)
#define I2C_SDA_IO      (GPIO_NUM_9)
#elif CONFIG_IDF_TARGET_ESP32P4
#define I2C_SCL_IO      (GPIO_NUM_8)
#define I2C_SDA_IO      (GPIO_NUM_7)
#else
#define I2C_SCL_IO      (GPIO_NUM_1)
#define I2C_SDA_IO      (GPIO_NUM_0)
#endif

extern esp_err_t gpio_init();