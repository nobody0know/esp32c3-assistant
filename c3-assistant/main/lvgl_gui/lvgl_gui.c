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

static const char * TAG ="LVGL_GUI";

LV_FONT_DECLARE(font_alipuhui);
LV_FONT_DECLARE(font_qweather);
LV_FONT_DECLARE(font_led);
LV_FONT_DECLARE(font_myawesome);


lv_obj_t * label_wifi;
extern lv_obj_t * label_sntp;
extern lv_obj_t * label_weather;

lv_obj_t * qweather_icon_label;
lv_obj_t * qweather_temp_label;
lv_obj_t * qweather_text_label;
lv_obj_t * qair_level_obj;
lv_obj_t * qair_level_label;
lv_obj_t * led_time_label;
lv_obj_t * week_label;
lv_obj_t * sunset_label;
lv_obj_t *indoor_temp_label;
lv_obj_t *indoor_humi_label;
lv_obj_t *outdoor_temp_label;
lv_obj_t *outdoor_humi_label;
lv_obj_t * date_label;

extern EventGroupHandle_t my_event_group;
extern time_t now;
extern struct tm timeinfo;

extern int qwnow_temp; // 实时天气温度
extern int qwnow_humi; // 实时天气湿度
extern int qwnow_icon; // 实时天气图标
extern char qwnow_text[32]; // 实时天气状态

extern int qwdaily_tempMax;       // 当天最高温度
extern int qwdaily_tempMin;       // 当天最低温度
extern char qwdaily_sunrise[10];  // 当天日出时间
extern char qwdaily_sunset[10];   // 当天日落时间

extern int qanow_level;       // 实时空气质量等级

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
    lv_obj_set_style_text_color(lv_scr_act(), lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_pos(label_wifi, 85 ,80);

    // 获取网络时间
    label_sntp = lv_label_create(lv_scr_act());
    lv_label_set_text(label_sntp, "");
    lv_obj_set_style_text_font(label_sntp, &font_alipuhui, 0);
    lv_obj_set_style_text_color(lv_scr_act(), lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_pos(label_sntp, 70 ,90);

    // 获取天气信息
    label_weather = lv_label_create(lv_scr_act());
    lv_label_set_text(label_weather, "");
    lv_obj_set_style_text_font(label_weather, &font_alipuhui, 0);
    lv_obj_set_style_text_color(lv_scr_act(), lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_pos(label_weather, 70 ,100);
}

// 显示天气图标
void lv_qweather_icon_show(void)
{
    switch (qwnow_icon)
    {
        case 100: lv_label_set_text(qweather_icon_label, "\xEF\x84\x81"); strcpy(qwnow_text, "晴"); break;
        case 101: lv_label_set_text(qweather_icon_label, "\xEF\x84\x82"); strcpy(qwnow_text, "多云"); break;
        case 102: lv_label_set_text(qweather_icon_label, "\xEF\x84\x83"); strcpy(qwnow_text, "少云"); break;
        case 103: lv_label_set_text(qweather_icon_label, "\xEF\x84\x84"); strcpy(qwnow_text, "晴间多云"); break;
        case 104: lv_label_set_text(qweather_icon_label, "\xEF\x84\x85"); strcpy(qwnow_text, "阴"); break;
        case 150: lv_label_set_text(qweather_icon_label, "\xEF\x84\x86"); strcpy(qwnow_text, "晴"); break;
        case 151: lv_label_set_text(qweather_icon_label, "\xEF\x84\x87"); strcpy(qwnow_text, "多云"); break;
        case 152: lv_label_set_text(qweather_icon_label, "\xEF\x84\x88"); strcpy(qwnow_text, "少云"); break;
        case 153: lv_label_set_text(qweather_icon_label, "\xEF\x84\x89"); strcpy(qwnow_text, "晴间多云"); break;
        case 300: lv_label_set_text(qweather_icon_label, "\xEF\x84\x8A"); strcpy(qwnow_text, "阵雨"); break;
        case 301: lv_label_set_text(qweather_icon_label, "\xEF\x84\x8B"); strcpy(qwnow_text, "强阵雨"); break;
        case 302: lv_label_set_text(qweather_icon_label, "\xEF\x84\x8C"); strcpy(qwnow_text, "雷阵雨"); break;
        case 303: lv_label_set_text(qweather_icon_label, "\xEF\x84\x8D"); strcpy(qwnow_text, "强雷阵雨"); break;
        case 304: lv_label_set_text(qweather_icon_label, "\xEF\x84\x8E"); strcpy(qwnow_text, "雷阵雨伴有冰雹"); break;
        case 305: lv_label_set_text(qweather_icon_label, "\xEF\x84\x8F"); strcpy(qwnow_text, "小雨"); break;
        case 306: lv_label_set_text(qweather_icon_label, "\xEF\x84\x90"); strcpy(qwnow_text, "中雨"); break;
        case 307: lv_label_set_text(qweather_icon_label, "\xEF\x84\x91"); strcpy(qwnow_text, "大雨"); break;
        case 308: lv_label_set_text(qweather_icon_label, "\xEF\x84\x92"); strcpy(qwnow_text, "极端降雨"); break;
        case 309: lv_label_set_text(qweather_icon_label, "\xEF\x84\x93"); strcpy(qwnow_text, "毛毛雨"); break;
        case 310: lv_label_set_text(qweather_icon_label, "\xEF\x84\x94"); strcpy(qwnow_text, "暴雨"); break;
        case 311: lv_label_set_text(qweather_icon_label, "\xEF\x84\x95"); strcpy(qwnow_text, "大暴雨"); break;
        case 312: lv_label_set_text(qweather_icon_label, "\xEF\x84\x96"); strcpy(qwnow_text, "特大暴雨"); break;
        case 313: lv_label_set_text(qweather_icon_label, "\xEF\x84\x97"); strcpy(qwnow_text, "冻雨"); break;
        case 314: lv_label_set_text(qweather_icon_label, "\xEF\x84\x98"); strcpy(qwnow_text, "小到中雨"); break;
        case 315: lv_label_set_text(qweather_icon_label, "\xEF\x84\x99"); strcpy(qwnow_text, "中到大雨"); break;
        case 316: lv_label_set_text(qweather_icon_label, "\xEF\x84\x9A"); strcpy(qwnow_text, "大到暴雨"); break;
        case 317: lv_label_set_text(qweather_icon_label, "\xEF\x84\x9B"); strcpy(qwnow_text, "暴雨到大暴雨"); break;
        case 318: lv_label_set_text(qweather_icon_label, "\xEF\x84\x9C"); strcpy(qwnow_text, "大暴雨到特大暴雨"); break;
        case 350: lv_label_set_text(qweather_icon_label, "\xEF\x84\x9D"); strcpy(qwnow_text, "阵雨"); break;
        case 351: lv_label_set_text(qweather_icon_label, "\xEF\x84\x9E"); strcpy(qwnow_text, "强阵雨"); break;
        case 399: lv_label_set_text(qweather_icon_label, "\xEF\x84\x9F"); strcpy(qwnow_text, "雨"); break;
        case 400: lv_label_set_text(qweather_icon_label, "\xEF\x84\xA0"); strcpy(qwnow_text, "小雪"); break;
        case 401: lv_label_set_text(qweather_icon_label, "\xEF\x84\xA1"); strcpy(qwnow_text, "中雪"); break;
        case 402: lv_label_set_text(qweather_icon_label, "\xEF\x84\xA2"); strcpy(qwnow_text, "大雪"); break;
        case 403: lv_label_set_text(qweather_icon_label, "\xEF\x84\xA3"); strcpy(qwnow_text, "暴雪"); break;
        case 404: lv_label_set_text(qweather_icon_label, "\xEF\x84\xA4"); strcpy(qwnow_text, "雨夹雪"); break;
        case 405: lv_label_set_text(qweather_icon_label, "\xEF\x84\xA5"); strcpy(qwnow_text, "雨雪天气"); break;
        case 406: lv_label_set_text(qweather_icon_label, "\xEF\x84\xA6"); strcpy(qwnow_text, "阵雨夹雪"); break;
        case 407: lv_label_set_text(qweather_icon_label, "\xEF\x84\xA7"); strcpy(qwnow_text, "阵雪"); break;
        case 408: lv_label_set_text(qweather_icon_label, "\xEF\x84\xA8"); strcpy(qwnow_text, "小到中雪"); break;
        case 409: lv_label_set_text(qweather_icon_label, "\xEF\x84\xA9"); strcpy(qwnow_text, "中到大雪"); break;
        case 410: lv_label_set_text(qweather_icon_label, "\xEF\x84\xAA"); strcpy(qwnow_text, "大到暴雪"); break;
        case 456: lv_label_set_text(qweather_icon_label, "\xEF\x84\xAB"); strcpy(qwnow_text, "阵雨夹雪"); break;
        case 457: lv_label_set_text(qweather_icon_label, "\xEF\x84\xAC"); strcpy(qwnow_text, "阵雪"); break;
        case 499: lv_label_set_text(qweather_icon_label, "\xEF\x84\xAD"); strcpy(qwnow_text, "雪"); break;
        case 500: lv_label_set_text(qweather_icon_label, "\xEF\x84\xAE"); strcpy(qwnow_text, "薄雾"); break;
        case 501: lv_label_set_text(qweather_icon_label, "\xEF\x84\xAF"); strcpy(qwnow_text, "雾"); break;
        case 502: lv_label_set_text(qweather_icon_label, "\xEF\x84\xB0"); strcpy(qwnow_text, "霾"); break;
        case 503: lv_label_set_text(qweather_icon_label, "\xEF\x84\xB1"); strcpy(qwnow_text, "扬沙"); break;
        case 504: lv_label_set_text(qweather_icon_label, "\xEF\x84\xB2"); strcpy(qwnow_text, "浮尘"); break;
        case 507: lv_label_set_text(qweather_icon_label, "\xEF\x84\xB3"); strcpy(qwnow_text, "沙尘暴"); break;
        case 508: lv_label_set_text(qweather_icon_label, "\xEF\x84\xB4"); strcpy(qwnow_text, "强沙尘暴"); break;
        case 509: lv_label_set_text(qweather_icon_label, "\xEF\x84\xB5"); strcpy(qwnow_text, "浓雾"); break;
        case 510: lv_label_set_text(qweather_icon_label, "\xEF\x84\xB6"); strcpy(qwnow_text, "强浓雾"); break;
        case 511: lv_label_set_text(qweather_icon_label, "\xEF\x84\xB7"); strcpy(qwnow_text, "中度霾"); break;
        case 512: lv_label_set_text(qweather_icon_label, "\xEF\x84\xB8"); strcpy(qwnow_text, "重度霾"); break;
        case 513: lv_label_set_text(qweather_icon_label, "\xEF\x84\xB9"); strcpy(qwnow_text, "严重霾"); break;
        case 514: lv_label_set_text(qweather_icon_label, "\xEF\x84\xBA"); strcpy(qwnow_text, "大雾"); break;
        case 515: lv_label_set_text(qweather_icon_label, "\xEF\x84\xBB"); strcpy(qwnow_text, "特强浓雾"); break;
        case 900: lv_label_set_text(qweather_icon_label, "\xEF\x85\x84"); strcpy(qwnow_text, "热"); break;
        case 901: lv_label_set_text(qweather_icon_label, "\xEF\x85\x85"); strcpy(qwnow_text, "冷"); break;
    
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
        case 0: lv_label_set_text(week_label, "星期日"); break;
        case 1: lv_label_set_text(week_label, "星期一"); break;
        case 2: lv_label_set_text(week_label, "星期二"); break;
        case 3: lv_label_set_text(week_label, "星期三"); break;
        case 4: lv_label_set_text(week_label, "星期四"); break; 
        case 5: lv_label_set_text(week_label, "星期五"); break;
        case 6: lv_label_set_text(week_label, "星期六"); break;
        default: lv_label_set_text(week_label, "星期日"); break;
    }
}

// 显示空气质量
void lv_qair_level_show(void)
{
    switch (qanow_level)
    {
        case 1: 
            lv_label_set_text(qair_level_label, "优"); 
            lv_obj_set_style_bg_color(qair_level_obj, lv_palette_main(LV_PALETTE_GREEN), 0); 
            lv_obj_set_style_text_color(qair_level_label, lv_color_hex(0xFFFFFF), 0);
            break;
        case 2: 
            lv_label_set_text(qair_level_label, "良"); 
            lv_obj_set_style_bg_color(qair_level_obj, lv_palette_main(LV_PALETTE_YELLOW), 0); 
            lv_obj_set_style_text_color(qair_level_label, lv_color_hex(0x000000), 0);
            break;
        case 3: 
            lv_label_set_text(qair_level_label, "轻");
            lv_obj_set_style_bg_color(qair_level_obj, lv_palette_main(LV_PALETTE_ORANGE), 0); 
            lv_obj_set_style_text_color(qair_level_label, lv_color_hex(0xFFFFFF), 0); 
            break;
        case 4: 
            lv_label_set_text(qair_level_label, "中"); 
            lv_obj_set_style_bg_color(qair_level_obj, lv_palette_main(LV_PALETTE_RED), 0); 
            lv_obj_set_style_text_color(qair_level_label, lv_color_hex(0xFFFFFF), 0);
            break; 
        case 5: 
            lv_label_set_text(qair_level_label, "重"); 
            lv_obj_set_style_bg_color(qair_level_obj, lv_palette_main(LV_PALETTE_PURPLE), 0); 
            lv_obj_set_style_text_color(qair_level_label, lv_color_hex(0xFFFFFF), 0);
            break;
        case 6: 
            lv_label_set_text(qair_level_label, "严"); 
            lv_obj_set_style_bg_color(qair_level_obj, lv_palette_main(LV_PALETTE_BROWN), 0); 
            lv_obj_set_style_text_color(qair_level_label, lv_color_hex(0xFFFFFF), 0);
            break;
        default: 
            lv_label_set_text(qair_level_label, "未"); 
            lv_obj_set_style_bg_color(qair_level_obj, lv_palette_main(LV_PALETTE_GREEN), 0); 
            lv_obj_set_style_text_color(qair_level_label, lv_color_hex(0xFFFFFF), 0);
            break;
    }
}

// 主界面
void lv_main_page(void)
{
    static lv_style_t style;
    lv_style_init(&style);

    /*Create an object with the new style*/
    lv_obj_t * obj = lv_obj_create(lv_scr_act());
    lv_obj_add_style(obj, &style, 0);

    // 显示地理位置
    lv_obj_t * addr_label = lv_label_create(obj);
    lv_obj_set_style_text_font(addr_label, &font_alipuhui, 0);
    lv_label_set_text(addr_label, "深圳市|坪山区");
    lv_obj_align_to(addr_label, obj, LV_ALIGN_TOP_LEFT, 0, 0);

    // 显示年月日
    date_label = lv_label_create(obj);
    lv_obj_set_style_text_font(date_label, &font_alipuhui, 0);
    lv_label_set_text_fmt(date_label, "%d年%02d月%02d日", timeinfo.tm_year+1900, timeinfo.tm_mon+1, timeinfo.tm_mday);
    lv_obj_align_to(date_label, obj, LV_ALIGN_TOP_RIGHT, 0, 0);

    // 显示分割线
    lv_obj_t * above_bar = lv_bar_create(obj);
    lv_obj_set_size(above_bar, 300, 3);
    lv_obj_set_pos(above_bar, 0 , 30);
    lv_bar_set_value(above_bar, 100, LV_ANIM_OFF);

    // 显示天气图标
    qweather_icon_label = lv_label_create(obj);
    lv_obj_set_style_text_font(qweather_icon_label, &font_qweather, 0);
    lv_obj_set_pos(qweather_icon_label, 0 , 40);
    lv_qweather_icon_show();

    // 显示空气质量
    static lv_style_t qair_level_style;
    lv_style_init(&qair_level_style);
    lv_style_set_radius(&qair_level_style, 10);  // 设置圆角半径
    lv_style_set_border_width(&qair_level_style, 0);
    lv_style_set_pad_all(&qair_level_style, 0);
    lv_style_set_width(&qair_level_style, 50);  // 设置宽
    lv_style_set_height(&qair_level_style, 26); // 设置高

    qair_level_obj = lv_obj_create(obj);
    lv_obj_add_style(qair_level_obj, &qair_level_style, 0);
    lv_obj_align_to(qair_level_obj, qweather_icon_label, LV_ALIGN_OUT_RIGHT_TOP, 5, 0);

    qair_level_label = lv_label_create(qair_level_obj);
    lv_obj_set_style_text_font(qair_level_label, &font_alipuhui, 0);
    lv_obj_align(qair_level_label, LV_ALIGN_CENTER, 0, 0);
    lv_qair_level_show();

    // 显示当天室外温度范围
    qweather_temp_label = lv_label_create(obj);
    lv_obj_set_style_text_font(qweather_temp_label, &font_alipuhui, 0);
    lv_label_set_text_fmt(qweather_temp_label, "%d~%d℃", qwdaily_tempMin, qwdaily_tempMax);
    lv_obj_align_to(qweather_temp_label, qweather_icon_label, LV_ALIGN_OUT_RIGHT_MID, 5, 5);

    // 显示当天天气图标代表的天气状况
    qweather_text_label = lv_label_create(obj);
    lv_obj_set_style_text_font(qweather_text_label, &font_alipuhui, 0);
    lv_label_set_long_mode(qweather_text_label, LV_LABEL_LONG_SCROLL_CIRCULAR);     /*Circular scroll*/
    lv_obj_set_width(qweather_text_label, 80);
    lv_label_set_text_fmt(qweather_text_label, "%s", qwnow_text);
    lv_obj_align_to(qweather_text_label, qweather_icon_label, LV_ALIGN_OUT_RIGHT_BOTTOM, 5, 0);
    
    // 显示时间  小时:分钟:秒钟
    led_time_label = lv_label_create(obj);
    lv_obj_set_style_text_font(led_time_label, &font_led, 0);
    lv_label_set_text_fmt(led_time_label, "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    lv_obj_set_pos(led_time_label, 142, 42);

    // 显示星期几
    week_label = lv_label_create(obj);
    lv_obj_set_style_text_font(week_label, &font_alipuhui, 0);
    lv_obj_align_to(week_label, led_time_label, LV_ALIGN_OUT_BOTTOM_RIGHT, -10, 6);
    lv_week_show();


    // 显示室外温湿度
    static lv_style_t outdoor_style;
    lv_style_init(&outdoor_style);
    lv_style_set_radius(&outdoor_style, 10);  // 设置圆角半径
    lv_style_set_border_width(&outdoor_style, 0);
    lv_style_set_pad_all(&outdoor_style, 5);
    lv_style_set_width(&outdoor_style, 100);  // 设置宽
    lv_style_set_height(&outdoor_style, 80); // 设置高

    lv_obj_t * outdoor_obj = lv_obj_create(obj);
    lv_obj_add_style(outdoor_obj, &outdoor_style, 0);
    lv_obj_align(outdoor_obj, LV_ALIGN_BOTTOM_LEFT, 0, 0);

    lv_obj_t *outdoor_th_label = lv_label_create(outdoor_obj);
    lv_obj_set_style_text_font(outdoor_th_label, &font_alipuhui, 0);
    lv_label_set_text(outdoor_th_label, "室外");
    lv_obj_align(outdoor_th_label, LV_ALIGN_TOP_MID, 0, 0);

    lv_obj_t *temp_symbol_label1 = lv_label_create(outdoor_obj);
    lv_obj_set_style_text_font(temp_symbol_label1, &font_myawesome, 0);
    lv_label_set_text(temp_symbol_label1, "\xEF\x8B\x88");  // 显示温度图标
    lv_obj_align(temp_symbol_label1, LV_ALIGN_LEFT_MID, 10, 0);

    outdoor_temp_label = lv_label_create(outdoor_obj);
    lv_obj_set_style_text_font(outdoor_temp_label, &font_alipuhui, 0);
    lv_label_set_text_fmt(outdoor_temp_label, "%d℃", qwnow_temp);
    lv_obj_align_to(outdoor_temp_label, temp_symbol_label1, LV_ALIGN_OUT_RIGHT_MID, 10, 0);

    lv_obj_t *humi_symbol_label1 = lv_label_create(outdoor_obj);
    lv_obj_set_style_text_font(humi_symbol_label1, &font_myawesome, 0);
    lv_label_set_text(humi_symbol_label1, "\xEF\x81\x83");  // 显示湿度图标
    lv_obj_align(humi_symbol_label1, LV_ALIGN_BOTTOM_LEFT, 10, 0);

    outdoor_humi_label = lv_label_create(outdoor_obj);
    lv_obj_set_style_text_font(outdoor_humi_label, &font_alipuhui, 0);
    lv_label_set_text_fmt(outdoor_humi_label, "%d%%", qwnow_humi);
    lv_obj_align_to(outdoor_humi_label, humi_symbol_label1, LV_ALIGN_OUT_RIGHT_MID, 10, 0);

}

// 主界面各值更新函数
void value_update_cb(lv_timer_t * timer)
{
    extern int th_update_flag;
    extern int qwnow_update_flag;
    extern int qair_update_flag;
    extern int qwdaily_update_flag;
    // 更新日期 星期 时分秒
    time(&now);
    localtime_r(&now, &timeinfo);
    lv_label_set_text_fmt(led_time_label, "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    lv_label_set_text_fmt(date_label, "%d年%02d月%02d日", timeinfo.tm_year+1900, timeinfo.tm_mon+1, timeinfo.tm_mday);
    lv_week_show();

    // 日出日落时间交替显示 每隔5秒切换
    if (timeinfo.tm_sec%10 == 0) 
        lv_label_set_text_fmt(sunset_label, "日落 %s", qwdaily_sunset);
    else if(timeinfo.tm_sec%10 == 5)
        lv_label_set_text_fmt(sunset_label, "日出 %s", qwdaily_sunrise);

    // 更新实时天气
    if(qwnow_update_flag == 1)
    {
        qwnow_update_flag = 0;
        lv_qweather_icon_show(); // 更新天气图标
        lv_label_set_text_fmt(qweather_text_label, "%s", qwnow_text); // 更新天气情况文字描述
        lv_label_set_text_fmt(outdoor_temp_label, "%d℃", qwnow_temp); // 更新室外温度
        lv_label_set_text_fmt(outdoor_humi_label, "%d%%", qwnow_humi); // 更新室外湿度
    }
    // 更新空气质量
    if(qair_update_flag ==1)
    {
        qair_update_flag = 0;
        lv_qair_level_show();
    }
    // 更新每日天气
    if(qwdaily_update_flag == 1)
    {
        qwdaily_update_flag = 0;
        lv_label_set_text_fmt(qweather_temp_label, "%d~%d℃", qwdaily_tempMin, qwdaily_tempMax); // 温度范围
    }
}

// 主界面 任务函数
static void main_page_task(void *pvParameters)
{
    int tm_cnt1 = 0;
    int tm_cnt2 = 0;

    xEventGroupWaitBits(my_event_group, WIFI_GET_WEATHER_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
    vTaskDelay(pdMS_TO_TICKS(100));
    lv_obj_clean(lv_scr_act());
    vTaskDelay(pdMS_TO_TICKS(100));
    lv_main_page();

    th_update_flag = 0;
    qwnow_update_flag = 0;
    qair_update_flag = 0;
    qwdaily_update_flag = 0;

    lv_timer_create(value_update_cb, 1000, NULL);  // 创建一个lv_timer 每秒更新一次数据

    reset_flag = 0; // 标记开机完成

    while (1)
    {
        tm_cnt1++;
        if (tm_cnt1 > 1800) // 30分钟更新一次实时天气和实时空气质量
        {
            tm_cnt1 = 0; // 计数清零
            get_now_weather();  // 获取实时天气信息
            get_air_quality();  // 获取实时空气质量
            tm_cnt2++;
            if (tm_cnt2 > 1) // 60分钟更新一次每日天气
            {
                tm_cnt2 = 0;
                get_daily_weather(); // 获取每日天气信息
            }
            printf("weather update time:%02d:%02d:%02d\n", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
        }
        
        vTaskDelay(pdMS_TO_TICKS(1000));    
    }
    
    vTaskDelete(NULL);
}

void allgui_init()
{
    xEventGroupWaitBits(my_event_group, WIFI_GET_WEATHER_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
    static lv_style_t style;
    lv_style_init(&style);
        /*Create an object with the new style*/
    lv_obj_t * obj = lv_obj_create(lv_scr_act());
    lv_obj_add_style(obj, &style, 0);

    reset_flag = 1; // 标记刚开机

    // 显示天气图标
    qweather_icon_label = lv_label_create(obj);
    lv_obj_set_style_text_font(qweather_icon_label, &font_qweather, 0);
    lv_obj_set_pos(qweather_icon_label, 0 , 30);
    lv_qweather_icon_show();

    // xTaskCreate(main_page_task, "main_page_task", 4096, NULL, 5, NULL);         // 非一次性任务 主界面任务

}

