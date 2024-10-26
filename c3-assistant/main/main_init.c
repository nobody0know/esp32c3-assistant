#include "i2s_es8311.h"
#include "gpio_func.h"
#include "qmi8658c.h"
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

static const char *TAG = "MAIN INIT";

uint8_t reset_flag;
EventGroupHandle_t my_event_group;

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

    if (wifi_sta_init() != ESP_OK)
    {
        ESP_LOGE(TAG, "wifi init failed");
        abort();
    }
    else
    {
        ESP_LOGI(TAG, "wifi init success");
    }

    ntp_time_init();
    weather_init();

    EPD_Init();
    lvgl_gui_init();
    allgui_init();
	// /************************全刷************************/
	// EPD_Display(gImage_1);
	// EPD_Update();
	// EPD_DeepSleep();
	// delay_ms(1000);
//   /*********************快刷模式**********************/
	// EPD_FastInit();
	// EPD_Display(gImage_2);
	// EPD_FastUpdate();
	// EPD_DeepSleep();
	// delay_ms(1000);

    // qmi8658c_init();
    // es8311_user_init();
}