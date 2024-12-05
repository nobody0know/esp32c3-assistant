#include "lvgl__lvgl/lvgl.h"
#include <stdio.h>
#include <string.h>
#include "sdkconfig.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "esp_check.h"
#include "lvgl_gui.h"
#include "esp_heap_caps.h"
#include "EPD_GUI.h"
#include "EPD.h"
#include <time.h>
#include <sys/time.h>
#include "esp32_wifi/common.h"
#include "get_weather.h"
#include "get_ntptime.h"
#include "lvgl_hw.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "lvgl_gui/generated/gui_guider.h"
#include "lvgl_gui/custom/custom.h"
#include "generated/events_init.h"
#include "guider_customer_fonts.h"

#include "gxhtc3.h"
#include "lsm6dso.h"

static const char *TAG = "LVGL_GUI";

LV_FONT_DECLARE(font_alipuhui);
LV_FONT_DECLARE(font_qweather);
LV_FONT_DECLARE(font_led);
LV_FONT_DECLARE(font_myawesome);

lv_obj_t *label_wifi;
lv_obj_t *label_sntp;
lv_obj_t *label_weather;

lv_obj_t *qweather_icon_label;
SemaphoreHandle_t lv_mutex; // 互斥锁句柄

lv_ui guider_ui; // guiguider ui ------------------------

extern EventGroupHandle_t my_event_group;
extern time_t now;
extern struct tm timeinfo;

extern int qwnow_temp;      // 实时天气温度
extern int qwnow_humi;      // 实时天气湿度
extern int qwnow_icon;      // 实时天气图标
extern char qwnow_text[32]; // 实时天气状态

extern int qwdaily_tempMax;      // 当天最高温度
extern int qwdaily_tempMin;      // 当天最低温度
extern char qwdaily_sunrise[10]; // 当天日出时间
extern char qwdaily_sunset[10];  // 当天日落时间

extern int qanow_level; // 实时空气质量等级

extern int th_update_flag;
extern int qwnow_update_flag;
extern int qair_update_flag;
extern int qwdaily_update_flag;

extern int reset_flag;

// 开机界面
void lv_gui_start(void)
{
    // 连接wifi
    label_wifi = lv_label_create(lv_scr_act());
    lv_label_set_text(label_wifi, "正在连接WiFi");
    lv_obj_set_style_text_font(label_wifi, &font_alipuhui, 0);
    lv_obj_set_pos(label_wifi, 85, 30);

    // 获取网络时间
    label_sntp = lv_label_create(lv_scr_act());
    lv_label_set_text(label_sntp, "");
    lv_obj_set_style_text_font(label_sntp, &font_alipuhui, 0);
    lv_obj_set_pos(label_sntp, 70, 70);

    // 获取天气信息
    label_weather = lv_label_create(lv_scr_act());
    lv_label_set_text(label_weather, "");
    lv_obj_set_style_text_font(label_weather, &font_alipuhui, 0);
    lv_obj_set_pos(label_weather, 70, 110);
}
uint16_t icon[80] = {100, 101, 102, 103, 104, 150, 151, 152, 153, 300, 301, 302, 303, 304, 305, 306, 307, 308, 309, 310, 311, 312, 313, 314, 315, 316, 317, 318, 350, 351, 399, 400, 401, 402, 403, 404, 405, 406, 407, 408, 409, 410, 456, 457, 499, 500, 501, 502, 503, 504, 507, 508, 509, 510, 511, 512, 513, 514, 515, 900, 901};
// 显示天气图标
void lv_qweather_icon_show(void)
{
    switch (qwnow_icon)
    {
    case 100:
        lv_label_set_text(qweather_icon_label, "\xEF\x84\x81");
        strcpy(qwnow_text, "晴");
        break;
    case 101:
        lv_label_set_text(qweather_icon_label, "\xEF\x84\x82");
        strcpy(qwnow_text, "多云");
        break;
    case 102:
        lv_label_set_text(qweather_icon_label, "\xEF\x84\x83");
        strcpy(qwnow_text, "少云");
        break;
    case 103:
        lv_label_set_text(qweather_icon_label, "\xEF\x84\x84");
        strcpy(qwnow_text, "晴间多云");
        break;
    case 104:
        lv_label_set_text(qweather_icon_label, "\xEF\x84\x85");
        strcpy(qwnow_text, "阴");
        break;
    case 150:
        lv_label_set_text(qweather_icon_label, "\xEF\x84\x86");
        strcpy(qwnow_text, "晴");
        break;
    case 151:
        lv_label_set_text(qweather_icon_label, "\xEF\x84\x87");
        strcpy(qwnow_text, "多云");
        break;
    case 152:
        lv_label_set_text(qweather_icon_label, "\xEF\x84\x88");
        strcpy(qwnow_text, "少云");
        break;
    case 153:
        lv_label_set_text(qweather_icon_label, "\xEF\x84\x89");
        strcpy(qwnow_text, "晴间多云");
        break;
    case 300:
        lv_label_set_text(qweather_icon_label, "\xEF\x84\x8A");
        strcpy(qwnow_text, "阵雨");
        break;
    case 301:
        lv_label_set_text(qweather_icon_label, "\xEF\x84\x8B");
        strcpy(qwnow_text, "强阵雨");
        break;
    case 302:
        lv_label_set_text(qweather_icon_label, "\xEF\x84\x8C");
        strcpy(qwnow_text, "雷阵雨");
        break;
    case 303:
        lv_label_set_text(qweather_icon_label, "\xEF\x84\x8D");
        strcpy(qwnow_text, "强雷阵雨");
        break;
    case 304:
        lv_label_set_text(qweather_icon_label, "\xEF\x84\x8E");
        strcpy(qwnow_text, "雷雨冰雹");
        break;
    case 305:
        lv_label_set_text(qweather_icon_label, "\xEF\x84\x8F");
        strcpy(qwnow_text, "小雨");
        break;
    case 306:
        lv_label_set_text(qweather_icon_label, "\xEF\x84\x90");
        strcpy(qwnow_text, "中雨");
        break;
    case 307:
        lv_label_set_text(qweather_icon_label, "\xEF\x84\x91");
        strcpy(qwnow_text, "大雨");
        break;
    case 308:
        lv_label_set_text(qweather_icon_label, "\xEF\x84\x92");
        strcpy(qwnow_text, "极端降雨");
        break;
    case 309:
        lv_label_set_text(qweather_icon_label, "\xEF\x84\x93");
        strcpy(qwnow_text, "毛毛雨");
        break;
    case 310:
        lv_label_set_text(qweather_icon_label, "\xEF\x84\x94");
        strcpy(qwnow_text, "暴雨");
        break;
    case 311:
        lv_label_set_text(qweather_icon_label, "\xEF\x84\x95");
        strcpy(qwnow_text, "大暴雨");
        break;
    case 312:
        lv_label_set_text(qweather_icon_label, "\xEF\x84\x96");
        strcpy(qwnow_text, "特大暴雨");
        break;
    case 313:
        lv_label_set_text(qweather_icon_label, "\xEF\x84\x97");
        strcpy(qwnow_text, "冻雨");
        break;
    case 314:
        lv_label_set_text(qweather_icon_label, "\xEF\x84\x98");
        strcpy(qwnow_text, "小到中雨");
        break;
    case 315:
        lv_label_set_text(qweather_icon_label, "\xEF\x84\x99");
        strcpy(qwnow_text, "中到大雨");
        break;
    case 316:
        lv_label_set_text(qweather_icon_label, "\xEF\x84\x9A");
        strcpy(qwnow_text, "大到暴雨");
        break;
    case 317:
        lv_label_set_text(qweather_icon_label, "\xEF\x84\x9B");
        strcpy(qwnow_text, "暴雨到大暴雨");
        break;
    case 318:
        lv_label_set_text(qweather_icon_label, "\xEF\x84\x9C");
        strcpy(qwnow_text, "大到特大暴雨");
        break;
    case 350:
        lv_label_set_text(qweather_icon_label, "\xEF\x84\x9D");
        strcpy(qwnow_text, "阵雨");
        break;
    case 351:
        lv_label_set_text(qweather_icon_label, "\xEF\x84\x9E");
        strcpy(qwnow_text, "强阵雨");
        break;
    case 399:
        lv_label_set_text(qweather_icon_label, "\xEF\x84\x9F");
        strcpy(qwnow_text, "雨");
        break;
    case 400:
        lv_label_set_text(qweather_icon_label, "\xEF\x84\xA0");
        strcpy(qwnow_text, "小雪");
        break;
    case 401:
        lv_label_set_text(qweather_icon_label, "\xEF\x84\xA1");
        strcpy(qwnow_text, "中雪");
        break;
    case 402:
        lv_label_set_text(qweather_icon_label, "\xEF\x84\xA2");
        strcpy(qwnow_text, "大雪");
        break;
    case 403:
        lv_label_set_text(qweather_icon_label, "\xEF\x84\xA3");
        strcpy(qwnow_text, "暴雪");
        break;
    case 404:
        lv_label_set_text(qweather_icon_label, "\xEF\x84\xA4");
        strcpy(qwnow_text, "雨夹雪");
        break;
    case 405:
        lv_label_set_text(qweather_icon_label, "\xEF\x84\xA5");
        strcpy(qwnow_text, "雨雪天气");
        break;
    case 406:
        lv_label_set_text(qweather_icon_label, "\xEF\x84\xA6");
        strcpy(qwnow_text, "阵雨夹雪");
        break;
    case 407:
        lv_label_set_text(qweather_icon_label, "\xEF\x84\xA7");
        strcpy(qwnow_text, "阵雪");
        break;
    case 408:
        lv_label_set_text(qweather_icon_label, "\xEF\x84\xA8");
        strcpy(qwnow_text, "小到中雪");
        break;
    case 409:
        lv_label_set_text(qweather_icon_label, "\xEF\x84\xA9");
        strcpy(qwnow_text, "中到大雪");
        break;
    case 410:
        lv_label_set_text(qweather_icon_label, "\xEF\x84\xAA");
        strcpy(qwnow_text, "大到暴雪");
        break;
    case 456:
        lv_label_set_text(qweather_icon_label, "\xEF\x84\xAB");
        strcpy(qwnow_text, "阵雨夹雪");
        break;
    case 457:
        lv_label_set_text(qweather_icon_label, "\xEF\x84\xAC");
        strcpy(qwnow_text, "阵雪");
        break;
    case 499:
        lv_label_set_text(qweather_icon_label, "\xEF\x84\xAD");
        strcpy(qwnow_text, "雪");
        break;
    case 500:
        lv_label_set_text(qweather_icon_label, "\xEF\x84\xAE");
        strcpy(qwnow_text, "薄雾");
        break;
    case 501:
        lv_label_set_text(qweather_icon_label, "\xEF\x84\xAF");
        strcpy(qwnow_text, "雾");
        break;
    case 502:
        lv_label_set_text(qweather_icon_label, "\xEF\x84\xB0");
        strcpy(qwnow_text, "霾");
        break;
    case 503:
        lv_label_set_text(qweather_icon_label, "\xEF\x84\xB1");
        strcpy(qwnow_text, "扬沙");
        break;
    case 504:
        lv_label_set_text(qweather_icon_label, "\xEF\x84\xB2");
        strcpy(qwnow_text, "浮尘");
        break;
    case 507:
        lv_label_set_text(qweather_icon_label, "\xEF\x84\xB3");
        strcpy(qwnow_text, "沙尘暴");
        break;
    case 508:
        lv_label_set_text(qweather_icon_label, "\xEF\x84\xB4");
        strcpy(qwnow_text, "强沙尘暴");
        break;
    case 509:
        lv_label_set_text(qweather_icon_label, "\xEF\x84\xB5");
        strcpy(qwnow_text, "浓雾");
        break;
    case 510:
        lv_label_set_text(qweather_icon_label, "\xEF\x84\xB6");
        strcpy(qwnow_text, "强浓雾");
        break;
    case 511:
        lv_label_set_text(qweather_icon_label, "\xEF\x84\xB7");
        strcpy(qwnow_text, "中度霾");
        break;
    case 512:
        lv_label_set_text(qweather_icon_label, "\xEF\x84\xB8");
        strcpy(qwnow_text, "重度霾");
        break;
    case 513:
        lv_label_set_text(qweather_icon_label, "\xEF\x84\xB9");
        strcpy(qwnow_text, "严重霾");
        break;
    case 514:
        lv_label_set_text(qweather_icon_label, "\xEF\x84\xBA");
        strcpy(qwnow_text, "大雾");
        break;
    case 515:
        lv_label_set_text(qweather_icon_label, "\xEF\x84\xBB");
        strcpy(qwnow_text, "特强浓雾");
        break;
    case 900:
        lv_label_set_text(qweather_icon_label, "\xEF\x85\x84");
        strcpy(qwnow_text, "热");
        break;
    case 901:
        lv_label_set_text(qweather_icon_label, "\xEF\x85\x85");
        strcpy(qwnow_text, "冷");
        break;

    default:
        printf("ICON_CODE:%d\n", qwnow_icon);
        lv_label_set_text(qweather_icon_label, "\xEF\x85\x86");
        strcpy(qwnow_text, "未知天气");
        break;
    }
}

// 显示星期几
void lv_week_show(void)
{
    switch (timeinfo.tm_wday)
    {
    case 0:
        lv_label_set_text(guider_ui.screen_label_5, "星期日");
        break;
    case 1:
        lv_label_set_text(guider_ui.screen_label_5, "星期一");
        break;
    case 2:
        lv_label_set_text(guider_ui.screen_label_5, "星期二");
        break;
    case 3:
        lv_label_set_text(guider_ui.screen_label_5, "星期三");
        break;
    case 4:
        lv_label_set_text(guider_ui.screen_label_5, "星期四");
        break;
    case 5:
        lv_label_set_text(guider_ui.screen_label_5, "星期五");
        break;
    case 6:
        lv_label_set_text(guider_ui.screen_label_5, "星期六");
        break;
    default:
        lv_label_set_text(guider_ui.screen_label_5, "星期日");
        break;
    }
}

// 主界面
void lv_main_page(void)
{
    /*Create a GUI-Guider app */
    setup_ui(&guider_ui);
    events_init(&guider_ui);
    custom_init(&guider_ui);

    time(&now);
    localtime_r(&now, &timeinfo);
    // lv_label_set_text_fmt(guider_ui.screen_label_6, "%d/%d/%d",timeinfo.tm_year+1900,timeinfo.tm_mon+1,timeinfo.tm_mday);
    lv_label_set_text_fmt(guider_ui.screen_label_6, "%d/%d %d:%02d", timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min);

    // 显示天气图标
    qweather_icon_label = lv_label_create(guider_ui.screen);
    lv_obj_set_style_text_font(qweather_icon_label, &font_qweather, 0);
    lv_obj_set_pos(qweather_icon_label, 20, 30);
    lv_qweather_icon_show();
    lv_label_set_text_fmt(guider_ui.screen_qweather_text_label, "%s", qwnow_text);
    lv_obj_set_style_text_font(guider_ui.screen_qweather_text_label, &lv_customer_font_vivo_Sans_20, LV_PART_MAIN | LV_STATE_DEFAULT);
    ESP_LOGI(TAG, "weather is %s", qwnow_text);

    lv_week_show();
    lv_label_set_text_fmt(guider_ui.screen_qweather_temp_label, "室外：%d℃\n室内：%02d℃", qwnow_temp, lsm6dso_read_temperature());
}

// 主界面各值更新函数
void value_update_cb()
{
    extern int qwnow_update_flag;
    extern uint8_t wifi_disconnect_flag;
    // 更新日期 星期 时分秒
    time(&now);
    localtime_r(&now, &timeinfo);
    // lv_label_set_text_fmt(guider_ui.screen_label_6, "%d/%d/%d",timeinfo.tm_year+1900,timeinfo.tm_mon+1,timeinfo.tm_mday);
    lv_label_set_text_fmt(guider_ui.screen_label_6, "%d/%d %d:%02d", timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min);
    lv_week_show();

    ESP_LOGI(TAG,"update srceen value");
    ESP_LOGI(TAG,"update srceen time:%02d:%02d:%02d\n", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);


    // 更新实时天气
    if (qwnow_update_flag == 1)
    {
        //-------test code start
        // static int i=0;
        // qwnow_icon = icon[i++];
        // if(i == sizeof(icon)-1) i=0;
        //-------test code end

        ESP_LOGI(TAG, "icon change to %d", qwnow_icon);
        qwnow_update_flag = 0;
        lv_qweather_icon_show(); // 更新天气图标
        lv_label_set_text_fmt(guider_ui.screen_qweather_text_label, "%s", qwnow_text);
        lv_label_set_text_fmt(guider_ui.screen_qweather_temp_label, "室外：%d℃\n室内：%02d℃", qwnow_temp, lsm6dso_read_temperature());
    }

    if (wifi_disconnect_flag == 1)
    {
        lv_label_set_text(guider_ui.screen_label_7, "" LV_SYMBOL_WIFI "!");
    }
    else
    {
        lv_label_set_text(guider_ui.screen_label_7, "" LV_SYMBOL_WIFI "");
    }
}

// 主界面 任务函数
static void main_page_task(void *pvParameters)
{
    uint16_t tm_cnt1 = 0;
    xEventGroupWaitBits(my_event_group, WIFI_GET_RTWEATHER_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
    vTaskDelay(pdMS_TO_TICKS(3000));
    lv_epaper_clean();

    ESP_LOGI(TAG, "clean and draw main page!!!");

    lv_main_page();
    vTaskDelay(pdMS_TO_TICKS(5000));

    th_update_flag = 0;
    qwnow_update_flag = 0;
    qair_update_flag = 0;
    qwdaily_update_flag = 0;

    reset_flag = 1; // 标记开机完成
    xEventGroupSetBits(my_event_group,WIFI_MAIN_SCRE_OK_BIT);

    while (1)
    {
        value_update_cb();
        tm_cnt1++;
        if (tm_cnt1 > 30)
        {
            get_now_weather(); // 获取实时天气信息
            printf("weather update time:%02d:%02d:%02d\n", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
            tm_cnt1 = 0;
        }
        vTaskDelay(pdMS_TO_TICKS(60 * 1000));//run pre 1min
    }

    vTaskDelete(NULL);
}

void allgui_init()
{

    lv_gui_start();

    xTaskCreate(main_page_task, "main_page_task", 8192, NULL, 5, NULL); // 非一次性任务 主界面任务
}
