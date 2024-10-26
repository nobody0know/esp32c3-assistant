#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_attr.h"
#include "esp_sleep.h"
#include "esp_netif_sntp.h"
#include "lwip/ip_addr.h"
#include "esp_sntp.h"
#include "common.h"

static const char *TAG = "NTP";

extern EventGroupHandle_t my_event_group;

time_t now;
struct tm timeinfo;

// 获得日期时间 任务函数
void get_time_task(void *pvParameters)
{
    xEventGroupWaitBits(my_event_group, WIFI_CONNECTED_BIT, pdFALSE, pdFALSE, portMAX_DELAY);

    esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG("pool.ntp.org");
    esp_netif_sntp_init(&config);
    // wait for time to be set
    int retry = 0;
    const int retry_count = 10;
    while (esp_netif_sntp_sync_wait(2000 / portTICK_PERIOD_MS) == ESP_ERR_TIMEOUT && ++retry < retry_count) {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
    }

    if(retry>5)
    {
        ESP_LOGE(TAG,"Get NTP time failed!!!!");
    }

    esp_netif_sntp_deinit();

    char strftime_buf[64];
    // 设置时区
    setenv("TZ", "CST-8", 1); 
    tzset();
    // 获取系统时间
    time(&now);
    localtime_r(&now, &timeinfo);

    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGI(TAG, "The current date/time in Shanghai is: %s", strftime_buf);

    xEventGroupSetBits(my_event_group, WIFI_GET_SNTP_BIT);
    
    vTaskDelete(NULL);
}

void ntp_time_init()
{
    xTaskCreate(get_time_task, "get_time_task", 4096, NULL, 5, NULL);           // 一次性任务   获取网络时间
}