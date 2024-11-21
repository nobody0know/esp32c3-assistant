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
#include "esp_smartconfig.h"

#include "esp32_wifi.h"
#include "udp_com/udp_server.h"
#include "common.h"
#include "lvgl.h"

extern EventGroupHandle_t my_event_group;
extern lv_obj_t *label_wifi;
extern lv_obj_t *label_sntp;
extern uint8_t reset_flag;

char wifi_ssid[33] = {0};
char wifi_password[65] = {0};
uint8_t wifi_config_flag;
uint8_t wifi_disconnect_flag = 0;
static const char *TAG = "WIFI";
EventGroupHandle_t s_wifi_event_group;
static nvs_handle wifi_config_nvs_h;
static uint16_t wifi_retry_count = 0;


static void smartconfig_task(void *parm);

static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
        printf("connect to wifi\n");
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        ESP_LOGI(TAG, "wifi connect faile try to reconnect!");
        wifi_disconnect_flag = 1;
        wifi_retry_count++;
        xEventGroupClearBits(s_wifi_event_group, CONNECTED_BIT);

        // printf("reset flag is %d",reset_flag);//debug
        if (wifi_retry_count > WIFI_MAX_RETRY_TIMES && reset_flag == 0) // 刚开机触发首次联网失败且重连次数过多，在运行期重连失败不执行配网程序
        {
            lv_label_set_text(label_wifi, "等待配网......");
            ESP_LOGW(TAG, "wifi connect error turn to esptouch!");
            xTaskCreate(smartconfig_task, "smartconfig_task", 4096, NULL, 3, NULL);
        }
        else
        {
            esp_wifi_connect();
        }
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) // 只有获取到路由器分配的IP，才认为是连上了路由器
    {
        ESP_LOGI(TAG, "get ip address");
        lv_label_set_text(label_wifi, "√ WiFi连接成功");
        lv_label_set_text(label_sntp, "正在获取网络时间");
        wifi_disconnect_flag = 0;
        xEventGroupSetBits(my_event_group, WIFI_CONNECTED_BIT);
    }
    else if (event_base == SC_EVENT && event_id == SC_EVENT_SCAN_DONE)
    {
        ESP_LOGI(TAG, "Scan done");
    }
    else if (event_base == SC_EVENT && event_id == SC_EVENT_FOUND_CHANNEL)
    {
        ESP_LOGI(TAG, "Found channel");
    }
    else if (event_base == SC_EVENT && event_id == SC_EVENT_GOT_SSID_PSWD)
    {
        ESP_LOGI(TAG, "Got SSID and password");

        smartconfig_event_got_ssid_pswd_t *evt = (smartconfig_event_got_ssid_pswd_t *)event_data;
        wifi_config_t wifi_config;

        bzero(&wifi_config, sizeof(wifi_config_t));
        memcpy(wifi_config.sta.ssid, evt->ssid, sizeof(wifi_config.sta.ssid));
        memcpy(wifi_config.sta.password, evt->password, sizeof(wifi_config.sta.password));

        memcpy(wifi_ssid, evt->ssid, sizeof(evt->ssid));
        memcpy(wifi_password, evt->password, sizeof(evt->password));
        ESP_LOGI(TAG, "SSID:%s", wifi_ssid);
        ESP_LOGI(TAG, "PASSWORD:%s", wifi_password);

        if (wifi_config_flag == 0) // first need write wifi config
        {
            printf("wifi_cfg update now... \n");
            wifi_config_flag = 1;
            ESP_ERROR_CHECK(nvs_set_str(wifi_config_nvs_h, "wifi_ssid", wifi_ssid));
            ESP_ERROR_CHECK(nvs_set_str(wifi_config_nvs_h, "wifi_passwd", wifi_password));
            ESP_ERROR_CHECK(nvs_set_u8(wifi_config_nvs_h, "wifi_update", wifi_config_flag));
            printf("wifi_cfg update ok. \n");
            ESP_ERROR_CHECK(nvs_commit(wifi_config_nvs_h)); /* 提交 */
            nvs_close(wifi_config_nvs_h);                   /* 关闭 */
        }
        else // next power on just to need check wifi config
        {
            char nvs_wifi_ssid[33] = {0};
            char nvs_wifi_password[65] = {0};

            size_t size = sizeof(wifi_ssid);
            nvs_get_str(wifi_config_nvs_h, "wifi_ssid", nvs_wifi_ssid, &size);
            size = sizeof(wifi_password);
            nvs_get_str(wifi_config_nvs_h, "wifi_passwd", nvs_wifi_password, &size);
            ESP_LOGI(TAG, "NVS SSID:%s", nvs_wifi_ssid);
            ESP_LOGI(TAG, "NSV PASSWORD:%s", nvs_wifi_password);

            if (memcmp(wifi_ssid, nvs_wifi_ssid, sizeof(wifi_ssid)) && memcmp(wifi_password, nvs_wifi_password, sizeof(wifi_password)))
            {
                printf("wifi_cfg update now... \n");
                ESP_ERROR_CHECK(nvs_set_str(wifi_config_nvs_h, "wifi_ssid", wifi_ssid));
                ESP_ERROR_CHECK(nvs_set_str(wifi_config_nvs_h, "wifi_passwd", wifi_password));
                printf("wifi_cfg update ok. \n");
                ESP_ERROR_CHECK(nvs_commit(wifi_config_nvs_h)); /* 提交 */
                nvs_close(wifi_config_nvs_h);                   /* 关闭 */
            }
        }

        ESP_ERROR_CHECK(esp_wifi_disconnect());
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
        esp_wifi_connect();
    }
    else if (event_base == SC_EVENT && event_id == SC_EVENT_SEND_ACK_DONE)
    {
        ESP_LOGI(TAG, "ESP TOUCH OK");
        xEventGroupSetBits(s_wifi_event_group, ESPTOUCH_DONE_BIT);
    }
}

static void smartconfig_task(void *parm)
{
    EventBits_t uxBits;
    ESP_ERROR_CHECK(esp_smartconfig_set_type(SC_TYPE_ESPTOUCH));
    smartconfig_start_config_t cfg = SMARTCONFIG_START_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_smartconfig_start(&cfg));
    while (1)
    {
        uxBits = xEventGroupWaitBits(s_wifi_event_group, CONNECTED_BIT | ESPTOUCH_DONE_BIT, true, false, portMAX_DELAY);
        if (uxBits & CONNECTED_BIT)
        {
            ESP_LOGI(TAG, "WiFi Connected to ap");
        }
        if (uxBits & ESPTOUCH_DONE_BIT)
        {
            ESP_LOGI(TAG, "smartconfig over");
            esp_smartconfig_stop();
            xEventGroupSetBits(my_event_group, WIFI_CONNECTED_BIT);
            vTaskDelete(NULL);
        }
    }
}

void wifi_connect_task()
{
    s_wifi_event_group = xEventGroupCreate();
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(nvs_open("wifi_config", NVS_READWRITE, &wifi_config_nvs_h));
    esp_err_t err = nvs_get_u8(wifi_config_nvs_h, "wifi_update", &wifi_config_flag);
    switch (err)
    {
    case ESP_OK:
        printf("wifi_cfg find value is %d. \n", wifi_config_flag);
        break;
    case ESP_ERR_NVS_NOT_FOUND:
        printf("wifi need to config\n");
        lv_label_set_text(label_wifi, "等待配网......");
        break;
    default:
        printf("Error (%s) reading!\n", esp_err_to_name(err));
    }

    if (wifi_config_flag == 1)
    {
        size_t size = sizeof(wifi_ssid);
        nvs_get_str(wifi_config_nvs_h, "wifi_ssid", wifi_ssid, &size);
        size = sizeof(wifi_password);
        nvs_get_str(wifi_config_nvs_h, "wifi_passwd", wifi_password, &size);
        ESP_LOGI(TAG, "NVS SSID:%s", wifi_ssid);
        ESP_LOGI(TAG, "NSV PASSWORD:%s", wifi_password);
    }

    printf("ESP_WIFI_MODE_STA \n");

    ESP_ERROR_CHECK(esp_netif_init());                // 用于初始化tcpip协议栈
    ESP_ERROR_CHECK(esp_event_loop_create_default()); // 创建一个默认系统事件调度循环，之后可以注册回调函数来处理系统的一些事件
    esp_netif_create_default_wifi_sta();              // 使用默认配置创建STA对象

    // 初始化WIFI
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // 注册事件
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));

    // 设置账户和密码
    wifi_config_t wifi_config;
    bzero(&wifi_config, sizeof(wifi_config_t));
    memcpy(wifi_config.sta.ssid, wifi_ssid, sizeof(wifi_config.sta.ssid));
    memcpy(wifi_config.sta.password, wifi_password, sizeof(wifi_config.sta.password));

    // 启动WIFI
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));               // 设置工作模式为STA
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config)); // 设置wifi配置
    ESP_ERROR_CHECK(esp_wifi_start());                               // 启动WIFI

    if (wifi_config_flag == 0)
    {
        xTaskCreate(smartconfig_task, "smartconfig_task", 4096, NULL, 3, NULL);
    }

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    vTaskDelete(NULL);
}

// WIFI STA初始化
esp_err_t wifi_sta_init(void)
{
    xTaskCreate(wifi_connect_task, "wifi_connect_task", 4096, NULL, 3, NULL);
    return ESP_OK;
}
