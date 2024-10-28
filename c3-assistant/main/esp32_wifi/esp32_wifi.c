#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "esp_random.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_crc.h"
#include "nvs_flash.h"

#include "esp32_wifi.h"
#include "udp_com/udp_server.h"
#include "common.h"
#include "lvgl.h"

extern EventGroupHandle_t my_event_group;
extern lv_obj_t *label_wifi;
extern lv_obj_t *label_sntp;
extern uint8_t reset_flag;

#define DEFAULT_WIFI_SSID "602"
#define DEFAULT_WIFI_PASSWORD "38188477"
static const char *TAG = "WIFI";
/** 事件回调函数
 * @param arg   用户传递的参数
 * @param event_base    事件类别
 * @param event_id      事件ID
 * @param event_data    事件携带的数据
 * @return 无
 */
static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT)
    {
        switch (event_id)
        {
        case WIFI_EVENT_STA_START: // WIFI以STA模式启动后触发此事件
            esp_wifi_connect();    // 启动WIFI连接
            break;
        case WIFI_EVENT_STA_CONNECTED: // WIFI连上路由器后，触发此事件
            ESP_LOGI(TAG, "connected to AP");
            break;
        case WIFI_EVENT_STA_DISCONNECTED: // WIFI从路由器断开连接后触发此事件
            esp_wifi_connect();           // 继续重连
            ESP_LOGI(TAG, "connect to the AP fail,retry now");
            break;
        default:
            break;
        }
    }
    if (event_base == IP_EVENT) // IP相关事件
    {
        switch (event_id)
        {
        case IP_EVENT_STA_GOT_IP: // 只有获取到路由器分配的IP，才认为是连上了路由器
            ESP_LOGI(TAG, "get ip address");
            if (reset_flag == 1) // 如果是刚开机
            {
                lv_label_set_text(label_wifi, "√ WiFi连接成功");
                lv_label_set_text(label_sntp, "正在获取网络时间");
            }
            xEventGroupSetBits(my_event_group, WIFI_CONNECTED_BIT);
            break;
        }
    }
}
// WIFI STA初始化
esp_err_t wifi_sta_init(void)
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_netif_init());                // 用于初始化tcpip协议栈
    ESP_ERROR_CHECK(esp_event_loop_create_default()); // 创建一个默认系统事件调度循环，之后可以注册回调函数来处理系统的一些事件
    esp_netif_create_default_wifi_sta();              // 使用默认配置创建STA对象

    // 初始化WIFI
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // 注册事件
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));
    // 设置账户和密码,在menuconfig中设置
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = DEFAULT_WIFI_SSID,
            .password = DEFAULT_WIFI_PASSWORD,
        },
    };

    // 启动WIFI
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));               // 设置工作模式为STA
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config)); // 设置wifi配置
    ESP_ERROR_CHECK(esp_wifi_start());                               // 启动WIFI

    ESP_LOGI(TAG, "wifi_init_sta finished.");
    return ESP_OK;
}
