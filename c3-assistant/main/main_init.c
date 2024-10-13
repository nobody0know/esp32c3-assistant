#include "i2s_es8311.h"
#include "gpio_func.h"
#include "qmi8658c.h"
#include "esp_log.h"
#include "esp32_wifi.h"
static const char *TAG = "MAIN INIT";

void app_main()
{
    if (gpio_init() != ESP_OK)
    {
        ESP_LOGE(TAG, "gpio init failed");
        abort();
    }
    else
    {
        ESP_LOGI(TAG, "gpio init success");
    }

    // if (wifi_sta_init() != ESP_OK)
    // {
    //     ESP_LOGE(TAG, "gpio init failed");
    //     abort();
    // }
    // else
    // {
    //     ESP_LOGI(TAG, "gpio init success");
    // }

    qmi8658c_init();
    es8311_user_init();
}