#include "i2s_es8311.h"
#include "gpio_func.h"
#include "lsm6dso.h"
#include "esp_log.h"

#include "EPD.h"
#include "EPD_GUI.h"
#include "lvgl_hw.h"
#include "Pic.h"
#include "lvgl.h"
#include "lvgl_gui.h"
#include "esp32_wifi.h"
#include "esp32_wifi/get_ntptime.h"
#include "esp32_wifi/get_weather.h"
#include "pca9557/pca9557_driver.h"
#include "esp_pm.h"
#include "esp_sleep.h"
#include "common.h"
#include "sleep_wakeup.h"

static const char *TAG = "MAIN INIT";

uint8_t reset_flag = 0;
EventGroupHandle_t my_event_group;

void sleep_init()
{
    xEventGroupWaitBits(my_event_group, WIFI_MAIN_SCRE_OK_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
    esp_sleep_enable_gpio_wakeup();
    esp_pm_config_t pm_config = {
        .max_freq_mhz = 80,
        .min_freq_mhz = 10,
        .light_sleep_enable = true};
    ESP_ERROR_CHECK(esp_pm_configure(&pm_config));
}

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

    EPD_SPIInit();

    lvgl_gui_init();
    allgui_init();

    wifi_sta_init();

    ntp_time_init();
    weather_init();

    // pca9557_init();
    // lsm6dso_init();

    // es8311_user_init();

    sleep_init();
}