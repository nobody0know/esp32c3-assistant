#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_timer.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_err.h"
#include "esp_log.h"
#include "lvgl.h"
#include "lv_demos.h"

#include <string.h>
#include <sys/param.h>
#include <stdlib.h>
#include <ctype.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"

#include "esp_http_client.h"
#include "cJSON.h"
#include "esp_tls.h"
#include "esp_crt_bundle.h"
#include "zlib.h"
#include "common.h"

#include <math.h>

static const char *TAG = "WEATHER";

extern uint8_t reset_flag;
extern EventGroupHandle_t my_event_group;
extern lv_obj_t *label_weather;

int th_update_flag;
int qwnow_update_flag;
int qair_update_flag;
int qwdaily_update_flag;

int qwnow_temp;       // 实时天气温度
int qwnow_humi;       // 实时天气湿度
int qwnow_icon = 100; // 实时天气图标
char qwnow_text[32];  // 实时天气状态

int qwdaily_tempMax;      // 当天最高温度
int qwdaily_tempMin;      // 当天最低温度
char qwdaily_sunrise[10]; // 当天日出时间
char qwdaily_sunset[10];  // 当天日落时间

int qanow_level; // 实时空气质量等级

#define MAX_HTTP_OUTPUT_BUFFER 2048

#define QWEATHER_DAILY_URL "https://devapi.qweather.com/v7/weather/3d?&location=101280604&key=dc16e8bc7a0c4ecfb5dd150ed4743a6a"
#define QWEATHER_NOW_URL "https://devapi.qweather.com/v7/weather/now?&location=101280604&key=dc16e8bc7a0c4ecfb5dd150ed4743a6a"
#define QAIR_NOW_URL "https://devapi.qweather.com/v7/air/now?&location=101280604&key=dc16e8bc7a0c4ecfb5dd150ed4743a6a"

esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    static char *output_buffer; // Buffer to store response of http request from event handler
    static int output_len;      // Stores number of bytes read
    switch (evt->event_id)
    {
    case HTTP_EVENT_ERROR:
        ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
        break;
    case HTTP_EVENT_ON_CONNECTED:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
        break;
    case HTTP_EVENT_HEADER_SENT:
        ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
        break;
    case HTTP_EVENT_ON_HEADER:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
        break;
    case HTTP_EVENT_ON_DATA:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
        /*
         *  Check for chunked encoding is added as the URL for chunked encoding used in this example returns binary data.
         *  However, event handler can also be used in case chunked encoding is used.
         */
        if (!esp_http_client_is_chunked_response(evt->client))
        {
            // If user_data buffer is configured, copy the response into the buffer
            int copy_len = 0;
            if (evt->user_data)
            {
                copy_len = MIN(evt->data_len, (MAX_HTTP_OUTPUT_BUFFER - output_len));
                if (copy_len)
                {
                    memcpy(evt->user_data + output_len, evt->data, copy_len);
                }
            }
            else
            {
                const int buffer_len = esp_http_client_get_content_length(evt->client);
                if (output_buffer == NULL)
                {
                    output_buffer = (char *)malloc(buffer_len);
                    output_len = 0;
                    if (output_buffer == NULL)
                    {
                        ESP_LOGE(TAG, "Failed to allocate memory for output buffer");
                        return ESP_FAIL;
                    }
                }
                copy_len = MIN(evt->data_len, (buffer_len - output_len));
                if (copy_len)
                {
                    memcpy(output_buffer + output_len, evt->data, copy_len);
                }
            }
            output_len += copy_len;
        }

        break;
    case HTTP_EVENT_ON_FINISH:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
        if (output_buffer != NULL)
        {
            // Response is accumulated in output_buffer. Uncomment the below line to print the accumulated response
            // ESP_LOG_BUFFER_HEX(TAG, output_buffer, output_len);
            free(output_buffer);
            output_buffer = NULL;
        }
        output_len = 0;
        break;
    case HTTP_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
        int mbedtls_err = 0;
        esp_err_t err = esp_tls_get_and_clear_last_error((esp_tls_error_handle_t)evt->data, &mbedtls_err, NULL);
        if (err != 0)
        {
            ESP_LOGI(TAG, "Last esp error code: 0x%x", err);
            ESP_LOGI(TAG, "Last mbedtls failure: 0x%x", mbedtls_err);
        }
        if (output_buffer != NULL)
        {
            free(output_buffer);
            output_buffer = NULL;
        }
        output_len = 0;
        break;
    case HTTP_EVENT_REDIRECT:
        ESP_LOGD(TAG, "HTTP_EVENT_REDIRECT");
        esp_http_client_set_header(evt->client, "From", "user@example.com");
        esp_http_client_set_header(evt->client, "Accept", "text/html");
        esp_http_client_set_redirection(evt->client);
        break;
    }
    return ESP_OK;
}

// GZIP解压函数
int gzDecompress(char *src, int srcLen, char *dst, int *dstLen)
{
    z_stream strm;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;

    strm.avail_in = srcLen;
    strm.avail_out = *dstLen;
    strm.next_in = (Bytef *)src;
    strm.next_out = (Bytef *)dst;

    int err = -1;
    err = inflateInit2(&strm, 31); // 初始化
    if (err == Z_OK)
    {
        printf("inflateInit2 err=Z_OK\n");
        err = inflate(&strm, Z_FINISH); // 解压gzip数据
        if (err == Z_STREAM_END)        // 解压成功
        {
            printf("inflate err=Z_OK\n");
            *dstLen = strm.total_out;
        }
        else // 解压失败
        {
            printf("inflate err=!Z_OK\n");
        }
        inflateEnd(&strm);
    }
    else
    {
        printf("inflateInit2 err! err=%d\n", err);
    }

    return err;
}

// 获取每日天气预报
void get_daily_weather(void)
{
    char local_response_buffer[MAX_HTTP_OUTPUT_BUFFER] = {0};
    int client_code = 0;
    int64_t gzip_len = 0;

    esp_http_client_config_t config = {
        .url = QWEATHER_DAILY_URL,
        .event_handler = _http_event_handler,
        .crt_bundle_attach = esp_crt_bundle_attach,
        .user_data = local_response_buffer, // Pass address of local buffer to get response
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK)
    {
        client_code = esp_http_client_get_status_code(client);
        gzip_len = esp_http_client_get_content_length(client);
        ESP_LOGI(TAG, "HTTPS Status = %d, content_length = %" PRIu64, client_code, gzip_len);
    }
    else
    {
        ESP_LOGE(TAG, "Error perform http request %s", esp_err_to_name(err));
    }

    if (client_code == 200)
    {
        int buffSize = MAX_HTTP_OUTPUT_BUFFER;
        char *buffData = (char *)malloc(MAX_HTTP_OUTPUT_BUFFER);
        memset(buffData, 0, MAX_HTTP_OUTPUT_BUFFER);

        int ret = gzDecompress(local_response_buffer, gzip_len, buffData, &buffSize);

        if (Z_STREAM_END == ret)
        { /* 解压成功 */
            printf("daily_weather decompress success\n");
            printf("buffSize = %d\n", buffSize);
            // lv_label_set_text(label_weather, "√ 天气信息获取成功");

            cJSON *root = cJSON_Parse(buffData);
            cJSON *daily = cJSON_GetObjectItem(root, "daily");

            cJSON *daily1 = cJSON_GetArrayItem(daily, 0);

            char *temp_max = cJSON_GetObjectItem(daily1, "tempMax")->valuestring;
            char *temp_min = cJSON_GetObjectItem(daily1, "tempMin")->valuestring;
            char *sunset = cJSON_GetObjectItem(daily1, "sunset")->valuestring;
            char *sunrise = cJSON_GetObjectItem(daily1, "sunrise")->valuestring;

            qwdaily_tempMax = atoi(temp_max);
            qwdaily_tempMin = atoi(temp_min);
            strcpy(qwdaily_sunrise, sunrise);
            strcpy(qwdaily_sunset, sunset);

            ESP_LOGI(TAG, "最高气温：%d", qwdaily_tempMax);
            ESP_LOGI(TAG, "最低气温：%d", qwdaily_tempMin);
            ESP_LOGI(TAG, "日出时间：%s", qwdaily_sunrise);
            ESP_LOGI(TAG, "日落时间：%s", qwdaily_sunset);

            cJSON_Delete(root);

            qwdaily_update_flag = 1;
        }
        else
        {
            printf("decompress failed:%d\n", ret);
        }
        free(buffData);
    }
    esp_http_client_cleanup(client);
}

// 获取每日天气预报的任务函数
void get_dwather_task(void *pvParameters)
{
    xEventGroupWaitBits(my_event_group, WIFI_GET_SNTP_BIT, pdFALSE, pdFALSE, portMAX_DELAY);

    // get_daily_weather();

    if (qwdaily_update_flag == 1)
    {
        qwdaily_update_flag = 0;
        xEventGroupSetBits(my_event_group, WIFI_GET_DAILYWEATHER_BIT);
    }
    else
    {
        printf("获取3日天气信息失败\n");
        qwdaily_update_flag = 0;
        xEventGroupSetBits(my_event_group, WIFI_GET_DAILYWEATHER_BIT);
    }

    vTaskDelete(NULL);
}

// 获取实时天气信息
void get_now_weather(void)
{
    char local_response_buffer[MAX_HTTP_OUTPUT_BUFFER] = {0};
    int client_code = 0;
    int64_t gzip_len = 0;

    esp_http_client_config_t config = {
        .url = QWEATHER_NOW_URL,
        .event_handler = _http_event_handler,
        .crt_bundle_attach = esp_crt_bundle_attach,
        .user_data = local_response_buffer, // Pass address of local buffer to get response
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK)
    {
        client_code = esp_http_client_get_status_code(client);
        gzip_len = esp_http_client_get_content_length(client);
        ESP_LOGI(TAG, "HTTPS Status = %d, content_length = %" PRIu64, client_code, gzip_len);
    }
    else
    {
        ESP_LOGE(TAG, "Error perform http request %s", esp_err_to_name(err));
    }

    if (client_code == 200)
    {
        int buffSize = MAX_HTTP_OUTPUT_BUFFER;
        char *buffData = (char *)malloc(MAX_HTTP_OUTPUT_BUFFER);
        memset(buffData, 0, MAX_HTTP_OUTPUT_BUFFER);

        int ret = gzDecompress(local_response_buffer, gzip_len, buffData, &buffSize);

        if (Z_STREAM_END == ret)
        { /* 解压成功 */
            printf("now weather decompress success\n");
            printf("buffSize = %d\n", buffSize);

            cJSON *root = cJSON_Parse(buffData);
            cJSON *now = cJSON_GetObjectItem(root, "now");

            char *temp = cJSON_GetObjectItem(now, "temp")->valuestring;
            char *icon = cJSON_GetObjectItem(now, "icon")->valuestring;
            char *humidity = cJSON_GetObjectItem(now, "humidity")->valuestring;

            qwnow_temp = atoi(temp);
            qwnow_humi = atoi(humidity);
            qwnow_icon = atoi(icon);

            ESP_LOGI(TAG, "地区：深圳市南山区");
            ESP_LOGI(TAG, "温度：%d", qwnow_temp);
            ESP_LOGI(TAG, "湿度：%d", qwnow_humi);
            ESP_LOGI(TAG, "图标：%d", qwnow_icon);

            cJSON_Delete(root);

            qwnow_update_flag = 1;
        }
        else
        {
            printf("decompress failed:%d\n", ret);
        }
        free(buffData);
    }
    esp_http_client_cleanup(client);
}

// 获取实时天气信息的任务函数
void get_rtweather_task(void *pvParameters)
{

    xEventGroupWaitBits(my_event_group, WIFI_GET_SNTP_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
    // vTaskDelay(pdMS_TO_TICKS(100));
    get_now_weather();
    if (qwnow_update_flag == 1) // 获取实时天气信息成功
    {
        if (reset_flag == 0) // 如果是刚开机
        {
            lv_label_set_text(label_weather, "√ 天气信息获取成功");
        }
        xEventGroupSetBits(my_event_group, WIFI_GET_RTWEATHER_BIT);
    }
    vTaskDelete(NULL);
}

// 获取实时空气质量
void get_air_quality(void)
{
    char local_response_buffer[MAX_HTTP_OUTPUT_BUFFER] = {0};
    int client_code = 0;
    int64_t gzip_len = 0;

    esp_http_client_config_t config = {
        .url = QAIR_NOW_URL,
        .event_handler = _http_event_handler,
        .crt_bundle_attach = esp_crt_bundle_attach,
        .user_data = local_response_buffer, // Pass address of local buffer to get response
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK)
    {
        client_code = esp_http_client_get_status_code(client);
        gzip_len = esp_http_client_get_content_length(client);
        ESP_LOGI(TAG, "HTTPS Status = %d, content_length = %" PRIu64, client_code, gzip_len);
    }
    else
    {
        ESP_LOGE(TAG, "Error perform http request %s", esp_err_to_name(err));
    }

    if (client_code == 200)
    {
        int buffSize = MAX_HTTP_OUTPUT_BUFFER;
        char *buffData = (char *)malloc(MAX_HTTP_OUTPUT_BUFFER);
        memset(buffData, 0, MAX_HTTP_OUTPUT_BUFFER);

        int ret = gzDecompress(local_response_buffer, gzip_len, buffData, &buffSize);

        if (Z_STREAM_END == ret)
        { /* 解压成功 */
            printf("decompress success\n");
            printf("buffSize = %d\n", buffSize);

            cJSON *root = cJSON_Parse(buffData);
            cJSON *now = cJSON_GetObjectItem(root, "now");

            char *level = cJSON_GetObjectItem(now, "level")->valuestring;

            qanow_level = atoi(level);

            ESP_LOGI(TAG, "空气质量：%d", qanow_level);

            cJSON_Delete(root);

            qair_update_flag = 1;
        }
        else
        {
            printf("decompress failed:%d\n", ret);
        }
        free(buffData);
    }
    esp_http_client_cleanup(client);
}

// 获取实时空气质量的任务函数
void get_airq_task(void *pvParameters)
{
    // 等待获取完实时空气质量 再获取空气质量
    xEventGroupWaitBits(my_event_group, WIFI_GET_RTWEATHER_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
    // vTaskDelay(pdMS_TO_TICKS(100));
    get_air_quality();

    if (qair_update_flag == 1)
    {
        xEventGroupSetBits(my_event_group, WIFI_GET_WEATHER_BIT);
    }

    vTaskDelete(NULL);
}

void weather_init()
{
    // xTaskCreate(get_dwather_task, "get_dwather_task", 8192, NULL, 5, NULL);     // 一次性任务   获取每日天气信息
    xTaskCreate(get_rtweather_task, "get_rtweather_task", 8192, NULL, 5, NULL); // 一次性任务   获取实时天气信息
    // xTaskCreate(get_airq_task, "get_airq_task", 8192, NULL, 5, NULL);           // 一次性任务   获取实时空气质量
}
