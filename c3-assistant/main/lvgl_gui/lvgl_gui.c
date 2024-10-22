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

#define EPD_W	152 
#define EPD_H	296

static const char * TAG ="LVGL";

static void lv_tick_inc_cb(void *data)
{
    uint32_t tick_inc_period_ms = *((uint32_t *) data);
    lv_tick_inc(tick_inc_period_ms);
}

static esp_err_t lv_port_tick_init(void)
{
    static uint32_t tick_inc_period_ms = 5;
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = lv_tick_inc_cb,
        .name = "lvgl_timer",
        .arg = &tick_inc_period_ms,
        .dispatch_method = ESP_TIMER_TASK,
        .skip_unhandled_events = true,
    };
 
    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, tick_inc_period_ms * 1000));
    return ESP_OK;
}

static void disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
{
    int32_t x;
	int32_t y;

	for(y = area->y1; y <= area->y2; y++)
    {
        for(x = area->x1; x <= area->x2; x++)
        {
            if(color_p->full>0)
            {
                Paint_SetPixel(x,y,1);
            }
            else
            {
                Paint_SetPixel(x,y,0);
            }
            color_p++;
        }

    }
	EPD_FastInit();
	EPD_Display(Paint.Image);
	EPD_FastUpdate();
    EPD_DeepSleep();
	delay_ms(1000);

    /*IMPORTANT!!!
     *Inform the graphics library that you are ready with the flushing*/
    lv_disp_flush_ready(disp_drv);
}

static void lv_port_disp_init(void)
{
    static lv_disp_draw_buf_t draw_buf_dsc;
    static lv_disp_drv_t disp_drv;          /*A variable to hold the drivers. Must be static or global.*/
    size_t disp_buf_height = 40;
 
    /* 必须从内部RAM分配显存，这样刷新速度快 */
    lv_color_t *p_disp_buf1 = heap_caps_malloc(EPD_W * disp_buf_height * sizeof(lv_color_t), MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA);
    lv_color_t *p_disp_buf2 = heap_caps_malloc(EPD_W * disp_buf_height * sizeof(lv_color_t), MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA);
    ESP_LOGI(TAG, "Try allocate two %u * %u display buffer, size:%u Byte", EPD_W, disp_buf_height, EPD_W * disp_buf_height * sizeof(lv_color_t) * 2);
    if (NULL == p_disp_buf1 || NULL == p_disp_buf2) {
        ESP_LOGE(TAG, "No memory for LVGL display buffer");
    }
    /* 初始化显示缓存 */
    lv_disp_draw_buf_init(&draw_buf_dsc, p_disp_buf1, p_disp_buf2, EPD_W * disp_buf_height);
    /* 初始化显示驱动 */
    lv_disp_drv_init(&disp_drv);
    /*设置水平和垂直宽度*/
    disp_drv.hor_res = EPD_W;
    disp_drv.ver_res = EPD_H;
    /* 设置刷新数据函数 */
    disp_drv.flush_cb = disp_flush;
    /*设置显示缓存*/
    disp_drv.draw_buf = &draw_buf_dsc;
    /*注册显示驱动*/
    lv_disp_drv_register(&disp_drv);
}

void lv_demo_button(void)
{
    lv_style_t style;

    lv_style_init(&style);
    lv_style_set_outline_width(&style, 2);

    lv_obj_set_style_bg_color(lv_scr_act(),   lv_color_white(), 0);

    lv_obj_t * btn = lv_btn_create(lv_scr_act());     /*Add a button the current screen*/
    lv_obj_set_pos(btn, 20, 20);                            /*Set its position*/
    lv_obj_set_size(btn, 120, 50);                          /*Set its size*/
    lv_obj_add_style(btn,&style,LV_STATE_DEFAULT);

    lv_obj_t * label = lv_label_create(btn);          /*Add a label to the button*/
    lv_label_set_text(label, "Button");                     /*Set the labels text*/
    lv_obj_set_style_text_color(label,lv_color_black(),0);
    lv_obj_center(label);
}



void lvgl_gui_init()
{
    ESP_ERROR_CHECK(lv_port_tick_init());
    lv_port_disp_init();
    lv_demo_button();
}


